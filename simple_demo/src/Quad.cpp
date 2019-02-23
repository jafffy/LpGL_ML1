#include <vector>

#if defined(ML1_DEVICE)
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#elif defined(ML1_OSX)

#include <GL/glew.h>

#endif

#include "glm/glm.hpp"

#include "Quad.h"

#include "ShaderUtils.h"
#include "lodepng.h"

#include "LpGLEngine.h"

class QuadImpl
{
public:
	GLuint vertex_array_id{};
	GLuint vertex_buffer{};
	GLuint red_program_id{};
	GLuint green_program_id{};

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
    auto t = static_cast<float>((float) i / 32.0f * 2 * M_PI);
    auto t2 = static_cast<float>((float) (i + 1) / 32.0f * 2 * M_PI);

		float s1 = radius * sinf(t);
		float c1 = radius * ratio * cosf(t);

		float s2 = radius * sinf(t2);
		float c2 = radius * ratio * cosf(t2);

    vertices.emplace_back(0, 0, -0.5);
    vertices.emplace_back(c1, s1, -0.5);
    vertices.emplace_back(c2, s2, -0.5);
  }

  impl->number_of_vertices = static_cast<int>(vertices.size());

	impl->red_program_id = LoadShaders(
    "assets/shaders/basic.vert",
    "assets/shaders/red.frag" // Red aiming
	);

	impl->green_program_id = LoadShaders(
		"assets/shaders/basic.vert",
		"assets/shaders/green.frag" // Green aiming
	);

	glGenVertexArrays(1, &impl->vertex_array_id);
	glBindVertexArray(impl->vertex_array_id);

	glGenBuffers(1, &impl->vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, impl->vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glUseProgram(impl->red_program_id);

	glUseProgram(0);
}

void Quad::DestroyContents()
{
	if (impl->texture_id != 0)
		glDeleteTextures(1, &impl->texture_id);

	glDeleteBuffers(1, &impl->vertex_buffer);

	glDeleteVertexArrays(1, &impl->vertex_array_id);

	glDeleteProgram(impl->red_program_id);
	glDeleteProgram(impl->green_program_id);
}

void Quad::Draw()
{
	if (LpGLEngine::instance().IsOn)
		glUseProgram(impl->green_program_id);
	else
		glUseProgram(impl->red_program_id);

	glBindVertexArray(impl->vertex_array_id);

	glDrawArrays(GL_TRIANGLES, 0, impl->number_of_vertices);

	glBindVertexArray(0);

	glUseProgram(0);
}