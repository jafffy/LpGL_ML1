
#include "ModelObj.h"
#include "BoundingBox.h"
#include "Camera.h"

#include <functional>
#include <unordered_map>

#include <ml_logging.h>

#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/intersect.hpp"

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

	bool isLastCulled = false;

	bool isCulled = false;
	bool isVisible = true;

	glm::vec3 velocity = glm::vec3(0, 0, 0);
	glm::vec3 acceleration = glm::vec3(0, 0, 0);

	BoundingSphere boundingSphere;

	float dynamics = 1.0f;

	glm::vec2 lastProjectedPosition = glm::vec2(0, 0);
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

static void model_load(std::string path, std::string base_path, std::function<void (const glm::vec3&, const glm::vec3&)> callback)
{
	ML_LOG(Info, "Begin model load: %s", path.c_str());

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), base_path.c_str(), false);

	if (!ret) {
		ML_LOG(Error, "%s/%s is not loaded", base_path.c_str(), path.c_str());
	}

	/*
	Simplify::load_obj(path.c_str());

	int target_count = Simplify::triangles.size() >> 5;
	double agressiveness = 7.0;

	ML_LOG(Info, "Start");
	Simplify::simplify_mesh(target_count, agressiveness, false);
	ML_LOG(Info, "End");
	*/

	for (auto shape : shapes) {
		size_t index_offset = 0;

		for (auto f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
			auto fnum = shape.mesh.num_face_vertices[f];

			for (int i = 0; i < fnum; ++i) {
				auto idx = shape.mesh.indices[index_offset + i];

				float x = attrib.vertices[3 * idx.vertex_index + 0];
				float y = attrib.vertices[3 * idx.vertex_index + 1];
				float z = -attrib.vertices[3 * idx.vertex_index + 2];

				float nx = attrib.normals[3 * idx.normal_index + 0];
				float ny = attrib.normals[3 * idx.normal_index + 1];
				float nz = attrib.normals[3 * idx.normal_index + 2];

				callback(glm::vec3(x, y, z), glm::vec3(nx, ny, nz));
			}

			index_offset += fnum;
		}
	}

	ML_LOG(Info, "End model load: %s", path.c_str());
}

struct MeshSet
{
public:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> vertices_reduced_1;
	std::vector<glm::vec3> vertices_reduced_2;
	BoundingBox3D boundingBox;
};

static std::unordered_map<std::string, MeshSet*> meshMap;

void ModelObj::Load(std::string path, std::string reduced1_path, std::string reduced2_path, std::string base_path)
{
	if (meshMap.find(path) == meshMap.end()) {
		model_load(path, base_path,
			[&](const glm::vec3& pos, const glm::vec3& normal) {
			impl->vertices.push_back(pos);
			impl->vertices.push_back(normal);

			impl->boundingBox.AddPoint(pos);
		});

		impl->boundingBox.BuildGeometry();

		impl->boundingSphere.position = (impl->boundingBox.Max + impl->boundingBox.Min) * 0.5f;
		impl->boundingSphere.radius = glm::length(impl->boundingBox.Max - impl->boundingBox.Min) * 0.5f;

		model_load(reduced1_path, base_path,
			[&](const glm::vec3& pos, const glm::vec3& normal) {
			impl->vertices_reduced_1.push_back(pos);
			impl->vertices_reduced_1.push_back(normal);
		});
		model_load(reduced2_path, base_path,
			[&](const glm::vec3& pos, const glm::vec3& normal) {
			impl->vertices_reduced_2.push_back(pos);
			impl->vertices_reduced_2.push_back(normal);
		});

		auto* meshSet = new MeshSet();
		meshSet->vertices = impl->vertices;
		meshSet->vertices_reduced_1 = impl->vertices_reduced_1;
		meshSet->vertices_reduced_2 = impl->vertices_reduced_2;
		meshSet->boundingBox = impl->boundingBox;

		meshMap[path] = meshSet;
	}
	else {
		auto* meshSet = meshMap[path];
		impl->vertices = meshSet->vertices;
		impl->vertices_reduced_1 = meshSet->vertices_reduced_1;
		impl->vertices_reduced_2 = meshSet->vertices_reduced_2;
		impl->boundingBox = meshSet->boundingBox;
	}
}

void ModelObj::SetShaders(std::string vertex_shader_path, std::string frag_shader_path)
{
  impl->vertex_shader_path = std::move(vertex_shader_path);
  impl->frag_shader_path = std::move(frag_shader_path);
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
	glm::mat4 model = glm::translate(impl->position)
		* glm::orientate4(impl->rotation)
		* glm::scale(impl->scale);

	impl->MVP = Camera::Instance().P * Camera::Instance().V * model;
	impl->M = model;
	impl->V = Camera::Instance().V;
}

void ModelObj::Render()
{
  if (impl->isCulled)
		return;

	GLuint target_vaid = impl->vertex_array_id;
  unsigned long target_num_vertices;

  target_num_vertices = impl->vertices.size();

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

void ModelObj::SetLastProjectedPosition(glm::vec2 projectedPosition)
{
	impl->lastProjectedPosition = projectedPosition;
}

const glm::vec2& ModelObj::GetLastProjectedPosition() const
{
	return impl->lastProjectedPosition;
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
	impl->isLastCulled = impl->isCulled;

	impl->isCulled = isCulled;
}

bool ModelObj::IsCulled() const
{
	return impl->isCulled;
}

void ModelObj::SetVisible(bool isVisible)
{
	impl->isVisible = isVisible;
}

bool ModelObj::IsVisible() const
{
	return impl->isVisible;
}

bool ModelObj::IsLastCulled() const
{
	return impl->isLastCulled;
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

std::vector<glm::vec3> ModelObj::GetBoundingBox() const
{
	std::vector<glm::vec3> output;

	for (const auto& v : impl->boundingBox.vertices) {
		auto v4 = (impl->M * glm::vec4(v, 1));
    output.emplace_back(v4.x, v4.y, v4.z);
	}

	return output;
}

void ModelObj::InitalizeLpGLForLpGL() {
  impl->isReduced1 = false;
  impl->isReduced2 = false;

  impl->isLastCulled = false;
  impl->isCulled = false;

  impl->lastProjectedPosition = glm::vec2(0, 0);
}
