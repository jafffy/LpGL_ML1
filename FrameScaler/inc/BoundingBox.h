#ifndef BOUNDINGBOX_H_
#define BOUNDINGBOX_H_

#include "glm/glm.hpp"

struct BoundingBox
{
	glm::vec2 min_point, max_point;

	BoundingBox() {}

	BoundingBox(const glm::vec2& min_point, const glm::vec2& max_point)
		: min_point{ min_point }, max_point{ max_point }
	{
	}

	inline bool is_intersect_with(const BoundingBox& bb) const {
		return !(min_point.x > bb.max_point.x || max_point.x < bb.min_point.x
			|| min_point.y > bb.max_point.y || max_point.y < bb.min_point.y);
	}

	inline bool is_contained_in(const BoundingBox& bb) const {
		return min_point.x > bb.min_point.x && min_point.y > bb.min_point.y
			&& max_point.x < bb.max_point.x && max_point.y < bb.max_point.y;
	}
};


#endif // BOUNDINGBOX_H_