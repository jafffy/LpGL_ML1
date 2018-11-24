#include "FrameScalerSampleApp.h"
#include "BoundingBox.h"
#include "ModelObj.h"
#include "Camera.h"
#include "Experiment.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <functional>

#ifndef _WIN32
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <ml_logging.h>
#endif

#define LOW
#ifdef LOW
static struct {
	float level1 = 0.7f;
	float level2 = 0.5f;
} threshold;
#endif
#ifdef HIGH
static struct {
	float level1 = 0.4f;
	float level2 = 0.2f;
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


class FrameScalerSampleAppImpl
{
public:
	std::vector<ModelObj*> models;
	int targetFrameRate = 60;
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

void FrameScalerSampleApp::Update(float dt)
{

}

void FrameScalerSampleApp::OnRender(int cameraIndex)
{
	for (auto* model : impl->models)
		model->Update();

#ifdef QDS

	if (cameraIndex == 0) {
		std::vector<const BoundingBox2D*> bbs;

		for (auto* model : impl->models) {
			std::vector<glm::vec3> boundingVertices = model->GetBoundingBox();
			auto boundingBox2D = new BoundingBox2D();

			for (const auto& v : boundingVertices) {
				glm::vec4 result = Camera::Instance().P_for_LpGL * (Camera::Instance().V * glm::vec4(v, 1.0f));
				boundingBox2D->AddPoint(result.x, result.y);
			}

			if (boundingBox2D->Min.y > 1 || boundingBox2D->Max.y < -1
				|| boundingBox2D->Min.x > 1 || boundingBox2D->Max.x < -1)
			{
				model->SetVisible(false);
			}
			else
			{
				model->SetVisible(true);
				bbs.push_back(boundingBox2D);

				if (boundingBox2D->Min.y > 0.50 || boundingBox2D->Max.y < -0.50
					|| boundingBox2D->Min.x > 0.50 || boundingBox2D->Max.x < -0.50) {
					model->SetReductionLevel(2);
				} else if (boundingBox2D->Min.y > 0.25 || boundingBox2D->Max.y < -0.25
					|| boundingBox2D->Min.x > 0.25 || boundingBox2D->Max.x < -0.25) {
					model->SetReductionLevel(1);
				} else {
					model->SetReductionLevel(0);
				}
			}
		}

		auto* quadTree = QuadTree::Create(bbs, QDS_DEPTH);

		if (lastQuadTree) {
			float dynamicScore = GetDynamicScoreBasedOnQuadtree(lastQuadTree->rootNode, quadTree->rootNode);

			if (dynamicScore > 0)
				ML_LOG(Info, "dynamic score: %f\n", dynamicScore);

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
#endif

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto* model : impl->models)
		model->Render();
}

int FrameScalerSampleApp::GetTargetFrameRate()
{
#ifdef QDS
	return impl->targetFrameRate;
#else
	return TARGET_FRAME_RATE;
#endif
}

void FrameScalerSampleApp::SetTargetFrameRate(int targetFrameRate)
{
	impl->targetFrameRate = targetFrameRate;
}

bool FrameScalerSampleApp::InitContents()
{
	int n = NUM_OBJECTS;

	for (int i = 0; i < n; ++i) {
		double t = i / (double)n;
		double r = 5.0f;
		float s = sinf(t * M_PI / 4);
		float c = cosf(t * M_PI / 4);

		auto model = new ModelObj();

		model->Load(TARGET_MODEL_FILEPATH, TARGET_MODEL_BASEPATH);
		model->SetShaders(VS_FILE_PATH, FS_FILE_PATH);
		model->SetPosition(glm::vec3(r*c, 0, r*s));
		model->SetScale(glm::vec3(5.0f));

		if (!model->Create())
			return false;

		impl->models.push_back(model);
	}

	return true;
}

void FrameScalerSampleApp::DestroyContents()
{
	for (auto* model : impl->models) {
		model->Destroy();

		if (model) {
			delete model;
			model = nullptr;
		}
	}

	impl->models.clear();
}
