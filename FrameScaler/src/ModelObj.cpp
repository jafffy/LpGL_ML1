#include "Experiment.h"
#include "ModelObj.h"
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

	GLuint vertex_array_id_reduced_1;
	GLuint vertex_buffer_reduced_1;

	GLuint vertex_array_id_reduced_2;
	GLuint vertex_buffer_reduced_2;

	std::string vertex_shader_path;
	std::string frag_shader_path;

	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 rotation = glm::vec3(0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> vertices_reduced_1;
	std::vector<glm::vec3> vertices_reduced_2;

	bool isReduced1 = false;
	bool isReduced2 = false; // TODO: Replace boolean to enum

	glm::mat4 MVP;
	GLuint matrix_id;

	glm::mat4 V;
	glm::mat4 M;
	GLuint V_id;
	GLuint M_id;

	BoundingBox3D boundingBox;

	bool isCulled = true;
	bool isVisible = true;
	bool isPhysicalObject = false;

	glm::vec3 velocity = glm::vec3(0.25f, 0.25f, 0);
	glm::vec3 acceleration = glm::vec3(0, 0, 0);
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

			impl->boundingBox.AddPoint(glm::vec3(x, y, z));
		}
	}

	impl->boundingBox.BuildGeometry();

	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string warn;
		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, TARGET_MODEL_FILEPATH_REDUCED_1, base_path.c_str());

		for (auto shape : shapes) {
			for (auto idx : shape.mesh.indices) {
				float x = attrib.vertices[3 * idx.vertex_index + 0];
				float y = attrib.vertices[3 * idx.vertex_index + 1];
				float z = attrib.vertices[3 * idx.vertex_index + 2];

				float nx = attrib.normals[3 * idx.normal_index + 0];
				float ny = attrib.normals[3 * idx.normal_index + 1];
				float nz = attrib.normals[3 * idx.normal_index + 2];

				impl->vertices_reduced_1.push_back(glm::vec3(x, y, z));
				impl->vertices_reduced_1.push_back(glm::vec3(nx, ny, nz));
			}
		}
	}
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string warn;
		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, TARGET_MODEL_FILEPATH_REDUCED_2, base_path.c_str());

		for (auto shape : shapes) {
			for (auto idx : shape.mesh.indices) {
				float x = attrib.vertices[3 * idx.vertex_index + 0];
				float y = attrib.vertices[3 * idx.vertex_index + 1];
				float z = attrib.vertices[3 * idx.vertex_index + 2];

				float nx = attrib.normals[3 * idx.normal_index + 0];
				float ny = attrib.normals[3 * idx.normal_index + 1];
				float nz = attrib.normals[3 * idx.normal_index + 2];

				impl->vertices_reduced_2.push_back(glm::vec3(x, y, z));
				impl->vertices_reduced_2.push_back(glm::vec3(nx, ny, nz));
			}
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

	glGenVertexArrays(1, &impl->vertex_array_id_reduced_1);
	glBindVertexArray(impl->vertex_array_id_reduced_1);

	glGenBuffers(1, &impl->vertex_buffer_reduced_1);
	glBindBuffer(GL_ARRAY_BUFFER, impl->vertex_buffer_reduced_1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * impl->vertices_reduced_1.size(), impl->vertices_reduced_1.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)sizeof(glm::vec3));

	glGenVertexArrays(1, &impl->vertex_array_id_reduced_2);
	glBindVertexArray(impl->vertex_array_id_reduced_2);

	glGenBuffers(1, &impl->vertex_buffer_reduced_2);
	glBindBuffer(GL_ARRAY_BUFFER, impl->vertex_buffer_reduced_2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * impl->vertices_reduced_2.size(), impl->vertices_reduced_2.data(), GL_STATIC_DRAW);

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
	glDeleteBuffers(1, &impl->vertex_buffer_reduced_2);
	glDeleteVertexArrays(1, &impl->vertex_array_id_reduced_2);
	glDeleteBuffers(1, &impl->vertex_buffer_reduced_1);
	glDeleteVertexArrays(1, &impl->vertex_array_id_reduced_1);
	glDeleteBuffers(1, &impl->vertex_buffer);
	glDeleteVertexArrays(1, &impl->vertex_array_id);

	glDeleteProgram(impl->program_id);
}

void ModelObj::Update(float dt)
{
	if (impl->isPhysicalObject) {
		static const glm::vec3 gravity(0, -0.16f, 0);

		glm::vec3 acceleration = impl->acceleration + gravity;

		impl->velocity = impl->velocity + acceleration * dt;
		impl->position = impl->position + impl->velocity * dt;
	}

	glm::mat4 model = glm::translate(impl->position)
		* glm::orientate4(impl->rotation)
		* glm::scale(impl->scale);
	impl->MVP = Camera::Instance().P * Camera::Instance().V * model;
	impl->M = model;
	impl->V = Camera::Instance().V;
}

void ModelObj::Render()
{
	if (!impl->isVisible || !impl->isCulled)
		return;

	GLuint target_vaid = impl->vertex_array_id;
	unsigned int target_num_vertices = impl->vertices.size();

	if (impl->isReduced2) {
		target_vaid = impl->vertex_array_id_reduced_2;
		target_num_vertices = impl->vertices_reduced_2.size();
	}
	else if (impl->isReduced1) {
		target_vaid = impl->vertex_array_id_reduced_1;
		target_num_vertices = impl->vertices_reduced_1.size();
	}

	glUseProgram(impl->program_id);

	glUniformMatrix4fv(impl->matrix_id, 1, GL_FALSE, glm::value_ptr(impl->MVP));
	glUniformMatrix4fv(impl->M_id, 1, GL_FALSE, glm::value_ptr(impl->M));
	glUniformMatrix4fv(impl->V_id, 1, GL_FALSE, glm::value_ptr(impl->V));

	glBindVertexArray(target_vaid);

	glDrawArrays(GL_TRIANGLES, 0, target_num_vertices);

	glBindVertexArray(0);

	glUseProgram(0);
}

void ModelObj::SetPosition(glm::vec3 position)
{
	impl->position = position;
}

const glm::vec3& ModelObj::GetPosition() const
{
	return impl->position;
}

void ModelObj::SetRotation(glm::vec3 rotation)
{
	impl->rotation = rotation;
}

void ModelObj::SetScale(glm::vec3 scale)
{
	impl->scale = scale;
}

void ModelObj::SetCulled(bool isCulled)
{
	impl->isCulled = isCulled;
}

void ModelObj::SetVisible(bool isVisible)
{
	impl->isVisible = isVisible;
}

bool ModelObj::IsVisible() const
{
	return impl->isVisible;
}

void ModelObj::SetReductionLevel(int n) {
	if (n == 1) {
		impl->isReduced1 = true;
		impl->isReduced2 = false;
	}
	else if (n == 2) {
		impl->isReduced1 = false;
		impl->isReduced2 = true;
	}
	else {
		impl->isReduced1 = false;
		impl->isReduced2 = false;
	}
}

void ModelObj::SetIsPhysicalObject(bool isPhysicalObject)
{
	impl->isPhysicalObject = isPhysicalObject;
}

std::vector<glm::vec3> ModelObj::GetBoundingBox() const
{
	std::vector<glm::vec3> output;

	for (const auto& v : impl->boundingBox.vertices) {
		auto v4 = (impl->M * glm::vec4(v, 1));
		output.push_back(glm::vec3(v4.x, v4.y, v4.z));
	}

	return output;
}
