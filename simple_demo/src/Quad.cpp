#include "Quad.h"

#include <ml_logging.h>

#include <GLES3/gl3.h>

#include "glm/glm.hpp"

#include <vector>

#include "ShaderUtils.h"

#include "lodepng.h"

class QuadImpl
{
public:
	GLuint vertex_array_id;
	GLuint vertex_buffer;
	GLuint program_id;

	GLuint texture_id = 0;
	GLuint sampler_id = 0;

	int number_of_vertices = 0;
};

Quad::Quad()
{
	impl = new QuadImpl();
}

Quad::~Quad()
{
	if (impl) {
		delete impl;
		impl = nullptr;
	}
}

void Quad::InitContents()
{
	float radius = 0.01f;
	float ratio = 3.0f / 4.0f;

	std::vector<glm::vec3> vertices;

	for (int i = 0; i < 32; ++i) {
		float t = (float)i / 32.0f * 2 * M_PI;
		float t2 = (float)(i + 1) / 32.0f * 2 * M_PI;

		float s1 = radius * sinf(t);
		float c1 = radius * ratio * cosf(t);

		float s2 = radius * sinf(t2);
		float c2 = radius * ratio * cosf(t2);

		vertices.push_back(glm::vec3(0, 0, 0));
		vertices.push_back(glm::vec3(c1, s1, 0));
		vertices.push_back(glm::vec3(c2, s2, 0));
	}

	impl->number_of_vertices = vertices.size();

	/*
	float vertices[] = {
		-radius * ratio,  radius, 0.0f,
		-radius * ratio, -radius, 0.0f,
		 radius * ratio, -radius, 0.0f,

		-radius * ratio,  radius, 0.0f,
		 radius * ratio, -radius, 0.0f,
		 radius * ratio,  radius, 0.0f,
	};
	*/

	impl->program_id = LoadShaders(
		"assets/shaders/basic.vert",
		"assets/shaders/basic.frag"
	);

	glGenVertexArrays(1, &impl->vertex_array_id);
	glBindVertexArray(impl->vertex_array_id);

	glGenBuffers(1, &impl->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, impl->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glUseProgram(impl->program_id);

	glUseProgram(0);
}

void Quad::DestroyContents()
{
	if (impl->texture_id != 0)
		glDeleteTextures(1, &impl->texture_id);

	glDeleteBuffers(1, &impl->vertex_buffer);

	glDeleteVertexArrays(1, &impl->vertex_array_id);

	glDeleteProgram(impl->program_id);
}

void Quad::Draw()
{
	glUseProgram(impl->program_id);

	glBindVertexArray(impl->vertex_array_id);

	glDrawArrays(GL_TRIANGLES, 0, impl->number_of_vertices);

	glBindVertexArray(0);

	glUseProgram(0);
}