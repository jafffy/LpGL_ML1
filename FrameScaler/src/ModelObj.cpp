#include "ModelObj.h"

#include <string>
#include <vector>

#include <ml_logging.h>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/euler_angles.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader.h"

#include "ShaderUtils.h"

class ModelObjImpl
{
public:
	GLuint vertex_array_id;
	GLuint vertex_buffer;
	GLuint program_id;

	GLuint matrix_id;

	std::string vertex_shader_path;
	std::string frag_shader_path;

	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 rotation = glm::vec3(0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);
};

ModelObj::ModelObj()
{
	impl = new ModelObjImpl();
}

ModelObj::~ModelObj()
{
	if (impl) {
		delete impl;
		impl = nullptr;
	}
}

void ModelObj::Load(std::string path, std::string base_path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), base_path.c_str());

	ML_LOG(Info, "shape material: %d %d\n", shapes.size(), materials.size());

	ML_LOG(Info, "# of vertices: %d\n", attrib.vertices.size());
}

void ModelObj::SetShaders(std::string vertex_shader_path, std::string frag_shader_path)
{
	impl->vertex_shader_path = vertex_shader_path;
	impl->frag_shader_path = frag_shader_path;
}

bool ModelObj::Create()
{
	impl->program_id = LoadShaders(
		impl->vertex_shader_path.c_str(),
		impl->frag_shader_path.c_str());

	glGenVertexArrays(1, &impl->vertex_array_id);
	glBindVertexArray(impl->vertex_array_id);

	static const GLfloat vertex_buffer_data[] =
	{
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	glGenBuffers(1, &impl->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, impl->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(0);

	glUseProgram(impl->program_id);

	impl->matrix_id = glGetUniformLocation(impl->program_id, "MVP");

	glUseProgram(0);

	return true;
}

void ModelObj::Destroy()
{
	glDeleteBuffers(1, &impl->vertex_buffer);
	glDeleteVertexArrays(1, &impl->vertex_array_id);

	glDeleteProgram(impl->program_id);
}

void ModelObj::Render(glm::mat4 VP)
{
	glUseProgram(impl->program_id);

	glm::mat4 model = glm::translate(impl->position)
		* glm::orientate4(impl->rotation)
		* glm::scale(impl->scale);
	glm::mat4 MVP = VP * model;

	glUniformMatrix4fv(impl->matrix_id, 1, GL_FALSE, glm::value_ptr(MVP));

	glBindVertexArray(impl->vertex_array_id);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);

	glUseProgram(0);
}

void ModelObj::SetPosition(glm::vec3 position)
{
	impl->position = position;
}

void ModelObj::SetRotation(glm::vec3 rotation)
{
	impl->rotation = rotation;
}

void ModelObj::SetScale(glm::vec3 scale)
{
	impl->scale = scale;
}
