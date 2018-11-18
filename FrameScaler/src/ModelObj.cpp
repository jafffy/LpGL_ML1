#include "Experiment.h"
#include "ModelObj.h"
#include "qds.h"
#include "BoundingBox.h"
#include "Camera.h"

#include <string>
#include <vector>
#include <cfloat>

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

	std::vector<glm::vec3> vertices;

	glm::vec3 bounding_box[8];

	bool is_culled = false;

	glm::mat4 MVP;
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

	glm::vec3 min_point(FLT_MAX), max_point(-FLT_MAX);

	for (auto shape : shapes) {
		for (auto idx : shape.mesh.indices) {
			float x = attrib.vertices[3 * idx.vertex_index + 0];
			float y = attrib.vertices[3 * idx.vertex_index + 1];
			float z = attrib.vertices[3 * idx.vertex_index + 2];

			if (x < min_point.x) {
				min_point.x = x;
			}
			if (y < min_point.y) {
				min_point.y = y;
			}
			if (z < min_point.z) {
				min_point.z = z;
			}

			if (x > max_point.x) {
				max_point.x = x;
			}
			if (y > max_point.y) {
				max_point.y = y;
			}
			if (z > max_point.z) {
				max_point.z = z;
			}

			impl->vertices.push_back(glm::vec3(x, y, z));
		}
	}

	impl->bounding_box[0] = glm::vec3(min_point.x, max_point.y, max_point.z);
	impl->bounding_box[1] = glm::vec3(min_point.x, max_point.y, min_point.z);
	impl->bounding_box[2] = glm::vec3(max_point.x, max_point.y, min_point.z);
	impl->bounding_box[3] = max_point;
	impl->bounding_box[4] = glm::vec3(min_point.x, min_point.y, max_point.z);
	impl->bounding_box[5] = min_point;
	impl->bounding_box[6] = glm::vec3(max_point.x, min_point.y, min_point.z);
	impl->bounding_box[7] = glm::vec3(max_point.x, min_point.y, max_point.z);
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

void ModelObj::Update()
{
	glm::mat4 model = glm::translate(impl->position)
		* glm::orientate4(impl->rotation)
		* glm::scale(impl->scale);
	// glm::mat4 P = glm::perspective(90.0f, Camera::Instance().ratio, 0.5f, 100.0f);
	impl->MVP = Camera::Instance().P * Camera::Instance().V * model;
	/*
	auto MVP = P * Camera::Instance().V * model;

#ifdef CULLING
	glm::vec2 min_point(FLT_MAX), max_point(-FLT_MAX);
	float min_z = FLT_MAX, max_z = -FLT_MAX;

	// Application level culling
	for (auto& pt : impl->bounding_box) {
		auto pt_on_proj = MVP * glm::vec4(pt, 1);
		float x = pt_on_proj.x;
		float y = pt_on_proj.y;
		float z = pt_on_proj.z;

		if (x < min_point.x) {
			min_point.x = x;
		}
		if (y < min_point.y) {
			min_point.y = y;
		}
		if (z < min_z) {
			min_z = z;
		}

		if (x > max_point.x) {
			max_point.x = x;
		}
		if (y > max_point.y) {
			max_point.y = y;
		}
		if (z > max_z) {
			max_z = z;
		}
	}

	BoundingBox screen_bb(glm::vec2(-1, -1), glm::vec2(1, 1));
	BoundingBox projected_bb(min_point, max_point);

	if (projected_bb.is_intersect_with(screen_bb)
		|| projected_bb.is_contained_in(screen_bb)
		|| screen_bb.is_contained_in(projected_bb)) {
		qds_insert_bounding_box(projected_bb.min_point, projected_bb.max_point);
		impl->is_culled = false;
	}
	else {
		impl->is_culled = true;
	}
#endif
*/
}

void ModelObj::Render()
{
	// if (impl->is_culled)
		// return;

	glUseProgram(impl->program_id);

	glUniformMatrix4fv(impl->matrix_id, 1, GL_FALSE, glm::value_ptr(impl->MVP));

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
