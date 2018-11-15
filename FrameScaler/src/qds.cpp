#include "Experiment.h"
#include "qds.h"
#include "BoundingBox.h"

#include <vector>
#include <cmath>

#include <ml_logging.h>

struct BoundingBox;
class QuadTree;

struct QuadTreeNode
{
	~QuadTreeNode()
	{
		for (int i = 0; i < 4; ++i) {
			if (children[i]) {
				delete children[i];
				children[i] = nullptr;
			}
		}
	}
	BoundingBox coverage = BoundingBox(glm::vec2(-1, -1), glm::vec2(1, 1));

	bool is_leaf = false;
	bool is_full = false; // All children are full

	QuadTreeNode* parent = nullptr;

	// ---------
	// | 0 | 1 |
	// ---------
	// | 3 | 4 |
	// ---------
	QuadTreeNode* children[4] = { nullptr, nullptr, nullptr, nullptr };
};

class QuadTree
{
public:
	QuadTreeNode* rootNode = new QuadTreeNode();

	~QuadTree()
	{
		if (rootNode) {
			delete rootNode;
			rootNode = nullptr;
		}
	}

	static float get_dynamic_score(QuadTree* prev_frame, QuadTree* cur_frame);
};

static QuadTree* previous_quadtree = nullptr;
static float dynamic_score = 0.0f;
static int recommended_FPS = 60;

static BoundingBox boundingBoxes[512];
static int n_boundingBoxes = 0;


void construct_quad_subtree(QuadTreeNode* parent, const BoundingBox& bb, int depth) {
	auto& coverage_min = parent->coverage.min_point;
	auto& coverage_max = parent->coverage.max_point;

	auto mid = (coverage_min + coverage_max) * 0.5f;

	auto ll = BoundingBox(coverage_min, mid);
	auto ur = BoundingBox(mid, coverage_max);
	auto ul = BoundingBox(glm::vec2(ll.min_point.x, mid.y), glm::vec2(mid.x, ur.max_point.y));
	auto lr = BoundingBox(glm::vec2(mid.x, ll.min_point.y), glm::vec2(ur.max_point.x, mid.y));

	BoundingBox subdivisions[] = { ul, ul, ll, lr };

	for (int i = 0; i < 4; ++i) {
		auto& subdivision = subdivisions[i];

		if (bb.is_contained_in(subdivision)
			|| subdivision.is_contained_in(bb)
			|| bb.is_intersect_with(subdivision)) { // Occupied
			if (!parent->children[i]) {
				parent->children[i] = new QuadTreeNode();
				parent->children[i]->coverage = subdivision;
			}

			assert(parent->children[i]);
			if (depth - 1 == 0) {
				parent->children[i]->is_leaf = true;
			}
			else {
				construct_quad_subtree(parent->children[i], subdivision, depth - 1);
			}
		}
	}
}

static QuadTree* construct_quadtree()
{
	QuadTree* quadTree = new QuadTree();

	for (int i = 0; i < n_boundingBoxes; ++i) {
		auto& bb = boundingBoxes[i];

		construct_quad_subtree(quadTree->rootNode, bb, QDS_DEPTH);
	}

	return quadTree;
}

static float get_dynamic_score_recursive(QuadTreeNode* prev_node, QuadTreeNode* cur_node, int depth)
{
	int n_leaves = 0;

	assert(prev_node);
	assert(cur_node);

	for (int i = 0; i < 4; ++i) {
		bool prev_exist = prev_node->children[i] && prev_node->children[i]->is_leaf;
		bool cur_exist = cur_node->children[i] && cur_node->children[i]->is_leaf;

		if (prev_exist || cur_exist)
			n_leaves++;
	}

	if (n_leaves > 0) {
		return n_leaves / powf(4, depth + 1);
	}

	float sum = 0.0f;

	for (int i = 0; i < 4; ++i) {
		if (prev_node->children[i] && cur_node->children[i])
			sum += get_dynamic_score_recursive(prev_node->children[i], cur_node->children[i], depth + 1);
		else if (prev_node->children[i] || cur_node->children[i])
			sum += 1.0f / powf(4, depth + 1);
	}

	return sum;
}

float QuadTree::get_dynamic_score(QuadTree* prev_frame, QuadTree* cur_frame)
{
	return get_dynamic_score_recursive(prev_frame->rootNode, cur_frame->rootNode, 0);
}

void qds_insert_bounding_box(const glm::vec2& min_point, const glm::vec2& max_point)
{
	boundingBoxes[n_boundingBoxes++] = BoundingBox(min_point, max_point);
}

void qds_update()
{
	auto* current_quadtree = construct_quadtree();
	n_boundingBoxes = 0; // Clear bounding box vector

	if (previous_quadtree == nullptr) {
		previous_quadtree = current_quadtree;
		return;
	}

	dynamic_score = QuadTree::get_dynamic_score(previous_quadtree, current_quadtree);

	if (dynamic_score > 0)
		ML_LOG(Info, "dynamic_score: %f\n", dynamic_score);

	if (recommended_FPS == 60) {
		if (dynamic_score < 0.5f)
			recommended_FPS = 30;
	}
	else if (recommended_FPS == 30) {
		if (dynamic_score > 0.5f)
			recommended_FPS = 60;
		else
			recommended_FPS = 15;

	}
	else if (recommended_FPS == 15) {
		if (dynamic_score > 0.25f)
			recommended_FPS = 30;
	}

	if (previous_quadtree) {
		delete previous_quadtree;
		previous_quadtree = nullptr;
	}

	previous_quadtree = current_quadtree;
}

int get_recommended_framerate()
{
	return recommended_FPS;
}
