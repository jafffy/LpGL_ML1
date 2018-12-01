#include "LpGLEngine.h"
#include "BoundingBox.h"
#include "ModelObj.h"
#include "Camera.h"
#include "Experiment.h"

#include <vector>
#include <functional>

#include <ml_logging.h>

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


class LpGLEngineImpl
{
public:
	int qds_depth = QDS_DEPTH;
};

LpGLEngine::LpGLEngine()
{
	impl = new LpGLEngineImpl();
}

LpGLEngine::~LpGLEngine()
{
	if (impl) {
		delete impl;
		impl = nullptr;
	}
}

int LpGLEngine::Update(int currentState, std::vector<ModelObj*>& models, int currentFPS, float dt)
{
	int recommended_fps = currentFPS;

	if (currentState != eels_without_lpgl) {
		std::vector<const BoundingBox2D*> bbs;

		for (auto* model : models) {
			std::vector<glm::vec3> boundingVertices = model->GetBoundingBox();
			auto boundingBox2D = new BoundingBox2D();

			for (const auto& v : boundingVertices) {
				glm::vec4 result = Camera::Instance().P_for_LpGL * (Camera::Instance().V_for_LpGL * glm::vec4(v, 1.0f));

				/*
				if (result.z < 0.0f)
					continue;
					*/

				boundingBox2D->AddPoint(result.x, result.y); // *0.25f, result.y * 0.25f);
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
					float kReductionLevel2 = LOD_LV2 / CULLING_FOV;// (LOD_LV2 + impl->simplificationAngularFactor) / CULLING_FOV;
					float kReductionLevel1 = LOD_LV1 / CULLING_FOV;

					auto& Min = boundingBox2D->Min;
					auto& Max = boundingBox2D->Max;

					// ML_LOG(Info, "Level2: %f %f %f %f\n", Min.x, Min.y, Max.x, Max.y);

					if (boundingBox2D->Min.y > kReductionLevel2 || boundingBox2D->Max.y < -kReductionLevel2
						|| boundingBox2D->Min.x > kReductionLevel2 || boundingBox2D->Max.x < -kReductionLevel2) {
						model->SetReductionLevel(2);
					}
					else if (boundingBox2D->Min.y > kReductionLevel1 || boundingBox2D->Max.y < -kReductionLevel1
						|| boundingBox2D->Min.x > kReductionLevel1 || boundingBox2D->Max.x < -kReductionLevel1) {
						model->SetReductionLevel(1);
					}
					else {
						model->SetReductionLevel(0);
					}
				}
			}

			if ((currentState == eels_with_qds || currentState == eels_with_full_lpgl)) {
				bbs.push_back(boundingBox2D);
			}
		}

		if ((currentState == eels_with_qds || currentState == eels_with_full_lpgl)) {
			auto* quadTree = QuadTree::Create(bbs, impl->qds_depth);

			if (lastQuadTree) {
				float dynamicScore = GetDynamicScoreBasedOnQuadtree(lastQuadTree->rootNode, quadTree->rootNode);

#ifdef LOG_DYNAMIC_SCORE
				if (dynamicScore > 0)
					ML_LOG(Verbose, "dynamic score: %f\n", dynamicScore);
#endif // LOG_DYNAMIC_SCORE

				if (currentFPS > 60 - 1) {
					if (dynamicScore < threshold.level1) {
						recommended_fps = 30;
					}
				}
				else if (currentFPS > 30 - 1) {
					if (dynamicScore > threshold.level1) {
						recommended_fps = 60;
					}
					else if (dynamicScore < threshold.level2) {
						recommended_fps = 15;
					}
				}
				else if (currentFPS > 15 - 1) {
					if (dynamicScore > threshold.level2) {
						recommended_fps = 30;
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

	return recommended_fps;
}

void LpGLEngine::SetQDSDepth(int depth)
{
	impl->qds_depth = depth;
}

LpGLEngine& LpGLEngine::instance()
{
	static LpGLEngine s_instance;
	return s_instance;
}

