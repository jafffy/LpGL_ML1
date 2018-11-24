#ifndef BOUNDINGBOX_H_
#define BOUNDINGBOX_H_

#include "glm/glm.hpp"


struct BoundingBox2D
{
	glm::vec2 Min;
	glm::vec2 Max;

	BoundingBox2D()
		: Min(FLT_MAX, FLT_MAX),
		Max(-FLT_MAX, -FLT_MAX)
	{}

	BoundingBox2D(const glm::vec2& Min, const glm::vec2& Max)
		: Min(Min), Max(Max) {}

	float Width() const { return Max.x - Min.x; }
	float Height() const { return Max.y - Min.y; }

	void AddPoint(float x, float y)
	{
		Min.x = x < Min.x ? x : Min.x;
		Min.y = y < Min.y ? y : Min.y;
		Max.x = x > Max.x ? x : Max.x;
		Max.y = y > Max.y ? y : Max.y;
	}

	bool IncludePoint(const glm::vec2& v) const
	{
		return Min.x < v.x && Min.y < v.y && Max.x > v.x && Max.y > v.y;
	}

	bool Intersect(const BoundingBox2D& bb) const
	{
		return IncludePoint(bb.Min) || IncludePoint(bb.Max);
	}
};

struct BoundingBox3D
{
	glm::vec3 Min;
	glm::vec3 Max;

	glm::vec3 vertices[8];

	BoundingBox3D()
		: Min(FLT_MAX, FLT_MAX, FLT_MAX),
		Max(-FLT_MAX, -FLT_MAX, -FLT_MAX)
	{}

	void AddPoint(const glm::vec3 &p)
	{
		float* m = &Min.x;
		float* M = &Max.x;
		const float* pPoint = &p.x;

		for (int i = 0; i < 3; ++i) {
			if (m[i] > pPoint[i]) {
				m[i] = pPoint[i];
			}
			if (M[i] < pPoint[i]) {
				M[i] = pPoint[i];
			}
		}
	}

	void BuildGeometry()
	{
		vertices[0] = Min;
		vertices[1] = Max;
		vertices[2] = glm::vec3(Min.x, Max.y, Max.z);
		vertices[3] = glm::vec3(Min.x, Max.y, Min.z);
		vertices[4] = glm::vec3(Max.x, Max.y, Min.z);
		vertices[5] = glm::vec3(Min.x, Min.y, Max.z);
		vertices[6] = glm::vec3(Max.x, Min.y, Max.z);
		vertices[7] = glm::vec3(Max.x, Min.y, Min.z);
	}
};

#endif // BOUNDINGBOX_H_