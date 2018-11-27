#ifndef MODELOBJ_H_
#define MODELOBJ_H_

#include <string>
#include <vector>

#include "glm/glm.hpp"

class ModelObjImpl;

class ModelObj
{
public:
	ModelObj();
	~ModelObj();

	void Load(std::string path, std::string base_path);
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

	std::vector<glm::vec3> GetBoundingBox() const;

private:
	ModelObjImpl* impl = nullptr;
};

#endif // MODELOBJ_H_