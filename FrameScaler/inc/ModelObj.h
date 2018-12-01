#ifndef MODELOBJ_H_
#define MODELOBJ_H_

#include <string>
#include <vector>

#include "glm/glm.hpp"

class ModelObjImpl;

class BoundingSphere
{
public:
	float radius;
	glm::vec3 position;
};

class ModelObj
{
public:
	ModelObj();
	~ModelObj();

	void Load(std::string path, std::string reduced1_path, std::string reduced2_path, std::string base_path);
	void SetShaders(std::string vertex_shader_path, std::string frag_shader_path);

	bool Create();
	void Destroy();

	void Update(float dt);
	void Render();

	void SetPosition(glm::vec3 position);
	const glm::vec3& GetPosition() const;

	void SetRotation(glm::vec3 rotation);
	void SetScale(glm::vec3 scale);

	void SetCulled(bool isCulled);
	void SetVisible(bool isVisible);
	bool IsVisible() const;
	void SetReductionLevel(int n);

	void SetIsPhysicalObject(bool isPhysicalObject);
	void SetInitialVelocity(const glm::vec3& v0);

	std::vector<glm::vec3> GetBoundingBox() const;
	BoundingSphere GetBoundingSphere() const;

	glm::vec3 GetColor() const;
	void SetColor(const glm::vec3& color);

	void Reset(float position_weight, float dynamics);

	void SetAbnormal();
	bool IsAbnormal();

private:
	ModelObjImpl* impl = nullptr;
};

#endif // MODELOBJ_H_