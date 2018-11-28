#include "Quad.h"

#include "CImg.h"

#include <ml_logging.h>

#include <GLES3/gl3.h>

#include "glm/glm.hpp"

#include <vector>

#include "ShaderUtils.h"

using namespace cimg_library;

class QuadImpl
{
public:
	GLuint vertex_array_id;
	GLuint vertex_buffer;
	GLuint program_id;
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
	float vertices[] = {
		-1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,

		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f
	};

	impl->program_id = LoadShaders(
		"assets/shaders/basic.vert",
		"assets/shaders/basic.frag"
	);

	glGenVertexArrays(1, &impl->vertex_array_id);
	glBindVertexArray(impl->vertex_array_id);

	glGenBuffers(1, &impl->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, impl->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
}

void Quad::DestroyContents()
{
	glDeleteBuffers(1, &impl->vertex_buffer);

	glDeleteVertexArrays(1, &impl->vertex_array_id);

	glDeleteProgram(impl->program_id);
}

void Quad::SetTexture(const char* path)
{
	CImg<unsigned char> image(path);
}

void Quad::Draw()
{
	glUseProgram(impl->program_id);

	glBindVertexArray(impl->vertex_array_id);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);

	glUseProgram(0);
}