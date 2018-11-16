#ifndef MODELOBJ_H_
#define MODELOBJ_H_

#include <string>
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

	void Update();
	void Render();

	void SetPosition(glm::vec3 position);
	void SetRotation(glm::vec3 rotation);
	void SetScale(glm::vec3 scale);

private:
	ModelObjImpl* impl = nullptr;
};

#endif // MODELOBJ_H_