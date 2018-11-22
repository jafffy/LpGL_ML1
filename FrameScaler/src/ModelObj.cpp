#include "Experiment.h"
#include "ModelObj.h"
#include "qds.h"
#include "BoundingBox.h"
#include "Camera.h"

#include <string>
#include <vector>
#include <cfloat>

#ifndef _WIN32
#include <ml_logging.h>
#else
#include <glad/glad.h>
#endif

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

	std::string vertex_shader_path;
	std::string frag_shader_path;

	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 rotation = glm::vec3(0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);

	std::vector<glm::vec3> vertices;

	glm::mat4 MVP;
	GLuint matrix_id;

	glm::mat4 V;
	glm::mat4 M;
	GLuint V_id;
	GLuint M_id;
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

	for (auto shape : shapes) {
		for (auto idx : shape.mesh.indices) {
			float x = attrib.vertices[3 * idx.vertex_index + 0];
			float y = attrib.vertices[3 * idx.vertex_index + 1];
			float z = attrib.vertices[3 * idx.vertex_index + 2];

			float nx = attrib.normals[3 * idx.normal_index + 0];
			float ny = attrib.normals[3 * idx.normal_index + 1];
			float nz = attrib.normals[3 * idx.normal_index + 2];

			impl->vertices.push_back(glm::vec3(x, y, z));
			impl->vertices.push_back(glm::vec3(nx, ny, nz));
		}
	}
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

	glGenBuffers(1, &impl->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, impl->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * impl->vertices.size(), impl->vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)sizeof(glm::vec3));

	glBindVertexArray(0);

	glUseProgram(impl->program_id);

	impl->matrix_id = glGetUniformLocation(impl->program_id, "MVP");
	impl->M_id = glGetUniformLocation(impl->program_id, "M");
	impl->V_id = glGetUniformLocation(impl->program_id, "V");

	glUseProgram(0);

	return true;
}

void ModelObj::Destroy()
{
	glDeleteBuffers(1, &impl->vertex_buffer);
	glDeleteVertexArrays(1, &impl->vertex_array_id);

	glDeleteProgram(impl->program_id);
}

void ModelObj::Update()
{
	glm::mat4 model = glm::translate(impl->position)
		* glm::orientate4(impl->rotation)
		* glm::scale(impl->scale);
	impl->MVP = Camera::Instance().P * Camera::Instance().V * model;
	impl->M = model;
	impl->V = Camera::Instance().V;
}

void ModelObj::Render()
{
	glUseProgram(impl->program_id);

	glUniformMatrix4fv(impl->matrix_id, 1, GL_FALSE, glm::value_ptr(impl->MVP));
	glUniformMatrix4fv(impl->M_id, 1, GL_FALSE, glm::value_ptr(impl->M));
	glUniformMatrix4fv(impl->V_id, 1, GL_FALSE, glm::value_ptr(impl->V));

	glBindVertexArray(impl->vertex_array_id);

	glDrawArrays(GL_TRIANGLES, 0, impl->vertices.size());

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
