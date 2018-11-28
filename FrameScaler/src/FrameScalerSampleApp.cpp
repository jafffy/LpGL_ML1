#include "FrameScalerSampleApp.h"
#include "BoundingBox.h"
#include "ModelObj.h"
#include "Camera.h"
#include "Experiment.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <functional>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/intersect.hpp"

#ifndef _WIN32
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <ml_logging.h>
#endif

#include "Quad.h"

#define HIGH
#ifdef LOW
static struct {
	float level1 = 0.7f;
	float level2 = 0.5f;
} threshold;
#endif
#ifdef HIGH
static struct {
	float level1 = 0.2f;
	float level2 = 0.1f;
} threshold;
#endif
#ifdef ORIGIN
static struct {
	float level1 = 0.0f;
	float level2 = 0.0f;
} threshold;
#endif

struct QuadTree
{
	struct Node
	{
		Node* parent = nullptr;
		Node* children[4] = { nullptr, nullptr, nullptr, nullptr };

		bool isFull = false;
		int depth;
	};

	Node* rootNode = nullptr;

	static QuadTree* Create(const std::vector<const BoundingBox2D*>& geometries, const int kMaxDepth);

	~QuadTree()
	{
		DeleteSubtree(rootNode);

		if (rootNode) {
			delete rootNode;
			rootNode = nullptr;
		}
	}

	void DeleteSubtree(Node* node)
	{
		if (!node)
			return;

		for (auto* child : node->children) {
			DeleteSubtree(child);

			if (child) {
				delete child;
				child = nullptr;
			}
		}
	}
};

QuadTree* QuadTree::Create(const std::vector<const BoundingBox2D*>& geometries, const int kMaxDepth)
{
	QuadTree* res = new QuadTree();

	std::function<void(const BoundingBox2D&, const std::vector<const BoundingBox2D*>&, int, QuadTree::Node*)> addDepth
		= [&](const BoundingBox2D& area, const std::vector<const BoundingBox2D*>& geometries, int depth, QuadTree::Node* parent) {
		auto& Min = area.Min;
		auto& Max = area.Max;
		float halfWidth = area.Width() * 0.5f;
		float halfHeight = area.Height() * 0.5f;
		std::vector<BoundingBox2D> subareas;
		subareas.push_back(BoundingBox2D(glm::vec2(Min.x, Min.y), glm::vec2(Min.x + halfWidth, Min.y + halfHeight)));
		subareas.push_back(BoundingBox2D(glm::vec2(Min.x + halfWidth, Min.y), glm::vec2(Max.x, Min.y + halfHeight)));
		subareas.push_back(BoundingBox2D(glm::vec2(Min.x, Min.y + halfHeight), glm::vec2(Min.x + halfWidth, Max.y)));
		subareas.push_back(BoundingBox2D(glm::vec2(Min.x + halfWidth, Min.y + halfHeight), glm::vec2(Max.x, Max.y)));

		for (int i = 0; i < subareas.size(); ++i) {
			auto& subarea = subareas[i];

			for (auto* geometry : geometries) {
				if (subarea.Intersect(*geometry)) {
					QuadTree::Node* pNode = new QuadTree::Node();
					parent->children[i] = pNode;
					pNode->parent = parent;
					pNode->isFull = false;
					pNode->depth = depth;

					if (depth + 1 < kMaxDepth) {
						addDepth(subarea, geometries, depth + 1, pNode);
					}

					break;
				}
			}
		}

		parent->isFull = parent->children[0] && parent->children[1] && parent->children[2] && parent->children[3];
	};

	BoundingBox2D viewArea(glm::vec2(-1, -1), glm::vec2(1, 1));
	QuadTree::Node* pRootNode = new QuadTree::Node();
	pRootNode->parent = nullptr;
	pRootNode->isFull = true;
	pRootNode->depth = 0;

	res->rootNode = pRootNode;

	addDepth(viewArea, geometries, 1, res->rootNode);

	return res;
}

static QuadTree* lastQuadTree = nullptr;

float GetDynamicScoreBasedOnQuadtree(const QuadTree::Node* prev, const QuadTree::Node* current)
{
	float sum = 0.0f;

	for (int i = 0; i < 4; ++i) {
		bool prevExist = prev->children[i];
		bool currentExist = current->children[i];

		if (prevExist && currentExist) {
			sum += GetDynamicScoreBasedOnQuadtree(prev->children[i], current->children[i]);
		}
		else if (prevExist != currentExist) {
			assert(prev->depth == current->depth);

			if (prevExist) {
				sum += 1.0f / (4 * prev->children[i]->depth);
			}
			else if (currentExist) {
				sum += 1.0f / (4 * current->children[i]->depth);
			}
		}
	}

	return sum;
}

enum eExpermentLpGLState
{
	eels_without_lpgl,
	eels_with_culling,
	eels_with_qds,
	eels_with_meshsimp,
	eels_with_full_lpgl,
	eels_count
};

enum eExperimentScenarioState
{
	eess_start,
	eess_basic,
	eess_high_dynamics,
	eess_low_dynamics,
	eess_left_most,
	eess_right_most
};

class FrameScalerSampleAppImpl
{
public:
	std::vector<ModelObj*> models;
	int targetFrameRate = 60;

	int currentLpGLState = 0;
	eExpermentLpGLState lpglStateSequence[eels_count];
	eExperimentScenarioState scenarioState = eess_start;

	float timer = 0.0f;

	void evaluateStateMachine();

	Quad quad;
};

FrameScalerSampleApp::FrameScalerSampleApp()
{
	impl = new FrameScalerSampleAppImpl();
}

FrameScalerSampleApp::~FrameScalerSampleApp()
{
	if (impl) {
		delete impl;
		impl = nullptr;
	}
}

int FrameScalerSampleApp::Start()
{
  return 0;
}

void FrameScalerSampleApp::Cleanup()
{

}

void FrameScalerSampleAppImpl::evaluateStateMachine()
{
	const float session_period = 10.0f;

	if (scenarioState == eess_start) {
		if (timer > 5) {
			scenarioState = eess_basic;
			timer = 0.0f;
		}
	}
	else if (scenarioState == eess_basic) {
		if (timer > session_period) {
			scenarioState = eess_high_dynamics;
			timer = 0.0f;
		}
	}
	else if (scenarioState == eess_high_dynamics) {
		if (timer > session_period) {
			scenarioState = eess_low_dynamics;
			timer = 0.0f;
		}
	}
	else if (scenarioState == eess_low_dynamics) {
		if (timer > session_period) {
			scenarioState = eess_left_most;
			timer = 0.0f;
		}
	}
	else if (scenarioState == eess_left_most) {
		if (timer > session_period) {
			scenarioState = eess_right_most;
			timer = 0.0f;
		}
	}
	else if (scenarioState == eess_right_most) {
		if (timer > session_period) {
			currentLpGLState++;
			scenarioState = eess_start;
			timer = 0.0f;

			for (auto* model : models) {
				model->SetReductionLevel(0);
				model->SetCulled(false);
				model->SetVisible(true);
			}
		}
	}
}

void FrameScalerSampleApp::Update(float dt)
{
	float timer = impl->timer;

	impl->evaluateStateMachine();

	impl->timer += dt;

	for (auto* model : impl->models) {
		if (model->IsVisible()) {
			const auto& p = model->GetPosition();

			if (p.y < -1) {
				bool isLeft = random() % 2 == 0;

				float initialX = 0.0f;
				glm::vec3 initialVelocity;

				float error_X = random() / (double)RAND_MAX * 0.05;
				float error_Y = random() / (double)RAND_MAX * 0.25f;
				float error_Z = random() / (double)RAND_MAX * 2.0f;

				if (isLeft) {
					initialX = -INITIAL_X;
					initialVelocity = glm::vec3(0.25f + error_X, 0.25f + error_Y, 0);
				}
				else {
					initialX = INITIAL_X;
					initialVelocity = glm::vec3(-0.25f + error_X, 0.25f + error_Y, 0);
				}

				model->SetPosition(glm::vec3(initialX, 0, -5.f + error_Z));
				model->SetInitialVelocity(initialVelocity);
			}

			model->Update(dt);
		}
	}
}

void FrameScalerSampleApp::OnRender(int cameraIndex, float dt)
{
	auto currentState = impl->lpglStateSequence[impl->currentLpGLState];

	Update(dt);

	if ((currentState != eels_without_lpgl) && cameraIndex == 0) {
		std::vector<const BoundingBox2D*> bbs;

		for (auto* model : impl->models) {
			std::vector<glm::vec3> boundingVertices = model->GetBoundingBox();
			auto boundingBox2D = new BoundingBox2D();

			for (const auto& v : boundingVertices) {
				glm::vec4 result = Camera::Instance().P_for_LpGL * (Camera::Instance().V_for_LpGL * glm::vec4(v, 1.0f));
				boundingBox2D->AddPoint(result.x, result.y);
			}

			if ((currentState == eels_with_culling || currentState == eels_with_full_lpgl)
				&& (boundingBox2D->Min.y > 1 || boundingBox2D->Max.y < -1
				|| boundingBox2D->Min.x > 1 || boundingBox2D->Max.x < -1))
			{
				model->SetCulled(true);
			}
			else
			{
				model->SetCulled(false);

				if ((currentState == eels_with_meshsimp || currentState == eels_with_full_lpgl)) {
					if (boundingBox2D->Min.y > 0.50 || boundingBox2D->Max.y < -0.50
						|| boundingBox2D->Min.x > 0.50 || boundingBox2D->Max.x < -0.50) {
						model->SetReductionLevel(2);
					}
					else if (boundingBox2D->Min.y > 0.25 || boundingBox2D->Max.y < -0.25
						|| boundingBox2D->Min.x > 0.25 || boundingBox2D->Max.x < -0.25) {
						model->SetReductionLevel(1);
					}
					else {
						model->SetReductionLevel(0);
					}
				}
			}

			if ((currentState == eels_with_qds || currentState == eels_with_full_lpgl)) {
				bbs.push_back(boundingBox2D);
				if ((currentState == eels_with_qds || currentState == eels_with_full_lpgl)) {
				}
			}
		}

		if ((currentState == eels_with_qds || currentState == eels_with_full_lpgl)) {
			auto* quadTree = QuadTree::Create(bbs, QDS_DEPTH);

			if (lastQuadTree) {
				float dynamicScore = GetDynamicScoreBasedOnQuadtree(lastQuadTree->rootNode, quadTree->rootNode);

#ifdef LOG_DYNAMIC_SCORE
				if (dynamicScore > 0)
					ML_LOG(Info, "dynamic score: %f\n", dynamicScore);
#endif // LOG_DYNAMIC_SCORE

				if (GetTargetFrameRate() > 60 - 1) {
					if (dynamicScore < threshold.level1) {
						SetTargetFrameRate(30);
					}
				}
				else if (GetTargetFrameRate() > 30 - 1) {
					if (dynamicScore > threshold.level1) {
						SetTargetFrameRate(60);
					}
					else if (dynamicScore < threshold.level2) {
						SetTargetFrameRate(15);
					}
				}
				else if (GetTargetFrameRate() > 15 - 1) {
					if (dynamicScore > threshold.level2) {
						SetTargetFrameRate(30);
					}
				}

				if (lastQuadTree) {
					delete lastQuadTree;
					lastQuadTree = nullptr;
				}
			}

			for (int i = 0; i < bbs.size(); ++i) {
				if (bbs[i]) {
					delete bbs[i];
					bbs[i] = nullptr;
				}
			}

			lastQuadTree = quadTree;
		}
	}

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto* model : impl->models)
		model->Render();

	impl->quad.Draw();
}

int FrameScalerSampleApp::GetTargetFrameRate()
{
	auto currentState = impl->lpglStateSequence[impl->currentLpGLState];

	if (currentState == eels_with_full_lpgl
		|| currentState == eels_with_qds)
		return impl->targetFrameRate;
	else
		return 60;
}

void FrameScalerSampleApp::SetTargetFrameRate(int targetFrameRate)
{
	impl->targetFrameRate = targetFrameRate;
}

bool FrameScalerSampleApp::InitContents()
{
	int n = NUM_OBJECTS;

	srandom(201120848);

	int currentLpGLstateSeq = 0;

	while (currentLpGLstateSeq < eels_count) {
		auto state = (eExpermentLpGLState)(random() % eels_count);

		for (int j = 0; j < currentLpGLstateSeq; ++j) {
			if (impl->lpglStateSequence[j] == state) {
				continue;
			}
		}

		impl->lpglStateSequence[currentLpGLstateSeq++] = state;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	for (int i = 0; i < n; ++i) {
		auto model = new ModelObj();

		bool isLeft = random() % 2 == 0;

		float initialX = 0.0f;
		glm::vec3 initialVelocity;

		float error_X = random() / (double)RAND_MAX * 0.05;
		float error_Y = random() / (double)RAND_MAX * 0.05f;
		float error_Z = random() / (double)RAND_MAX * 2.0f;

		if (isLeft) {
			initialX = -INITIAL_X;
			initialVelocity = glm::vec3(0.25f + error_X, 0.25f + error_Y, 0);
		}
		else {
			initialX = INITIAL_X;
			initialVelocity = glm::vec3(-0.25f + error_X, 0.25f + error_Y, 0);
		}

		model->Load(TARGET_MODEL_FILEPATH, TARGET_MODEL_FILEPATH_REDUCED_1, TARGET_MODEL_FILEPATH_REDUCED_2, TARGET_MODEL_BASEPATH);
		model->SetShaders(VS_FILE_PATH, FS_FILE_PATH);
		model->SetScale(glm::vec3(0.25f));
		model->SetPosition(glm::vec3(initialX, 0, -5.f + error_Z));
		model->SetRotation(glm::vec3(0, 0, 0));
		model->SetVisible(true);
		model->SetIsPhysicalObject(true);
		model->SetInitialVelocity(initialVelocity);

		if (!model->Create())
			return false;

		impl->models.push_back(model);
	}

	impl->quad.InitContents();

	return true;
}

void FrameScalerSampleApp::DestroyContents()
{
	impl->quad.DestroyContents();

	for (auto* model : impl->models) {
		model->Destroy();

		if (model) {
			delete model;
			model = nullptr;
		}
	}

	impl->models.clear();
}

void FrameScalerSampleApp::OnPressed()
{
	ModelObj* closestModel = nullptr;
	float maxDistance = -FLT_MAX;

	for (auto* model : impl->models) {
		std::vector<glm::vec3> boundingVertices = model->GetBoundingBox();
		auto boundingBox2D = new BoundingBox2D();

		for (const auto& v : boundingVertices) {
			glm::vec4 result = Camera::Instance().P_for_LpGL * (Camera::Instance().V_for_LpGL * glm::vec4(v, 1.0f));
			boundingBox2D->AddPoint(result.x, result.y);

			if (!(boundingBox2D->Min.y > 0.20 || boundingBox2D->Max.y < -0.20
				|| boundingBox2D->Min.x > 0.20 || boundingBox2D->Max.x < -0.20)) {

				if (maxDistance < result.z) {
					closestModel = model;
					maxDistance = result.z;
				}
			}
		}
	}

	if (!closestModel)
		return;

	bool isLeft = random() % 2 == 0;

	float initialX = 0.0f;
	glm::vec3 initialVelocity;

	float error_X = random() / (double)RAND_MAX * 0.05;
	float error_Y = random() / (double)RAND_MAX * 0.25f;
	float error_Z = random() / (double)RAND_MAX * 2.0f;

	if (isLeft) {
		initialX = -INITIAL_X;
		initialVelocity = glm::vec3(0.25f + error_X, 0.25f + error_Y, 0);
	}
	else {
		initialX = INITIAL_X;
		initialVelocity = glm::vec3(-0.25f + error_X, 0.25f + error_Y, 0);
	}

	closestModel->SetPosition(glm::vec3(initialX, 0, -5.f + error_Z));
	closestModel->SetInitialVelocity(initialVelocity);
}
