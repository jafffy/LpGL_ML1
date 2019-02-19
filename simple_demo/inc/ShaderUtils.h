#ifndef SHADERUTILS_H_
#define SHADERUTILS_H_

#ifndef _WIN32
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#else
#include <glad/glad.h>
#endif

GLuint LoadShaders(const char* verex_file_path, const char* fragment_file_path);

#endif // SHADERUTILS_H_