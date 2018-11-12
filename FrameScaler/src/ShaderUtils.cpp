#include "ShaderUtils.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <ml_logging.h>

static GLuint create_shader(const char* shader_file_path, GLenum shader_type)
{
	GLint compile_result = GL_FALSE;
	int len_compile_info_log;

	GLuint vertex_shader_id = glCreateShader(shader_type);

	std::string vertex_shader_code;
	std::ifstream vertex_shader_stream(shader_file_path, std::ios::in);
	if (vertex_shader_stream.is_open()) {
		std::stringstream sstr;
		sstr << vertex_shader_stream.rdbuf();
		vertex_shader_code = sstr.str();
	}
	else {
		ML_LOG(Error, "Failed to open %s\n", shader_file_path);
		return -1;
	}

	char const * vertex_shader_pointer = vertex_shader_code.c_str();

	glShaderSource(vertex_shader_id, 1, &vertex_shader_pointer, nullptr);
	glCompileShader(vertex_shader_id);

	glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &compile_result);
	glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &len_compile_info_log);
	
	if (len_compile_info_log > 0) {
		std::vector<char> vertex_shader_error_mesage(len_compile_info_log);
		glGetShaderInfoLog(vertex_shader_id, len_compile_info_log, nullptr, &vertex_shader_error_mesage[0]);
		ML_LOG(Error, "%s\n", &vertex_shader_error_mesage[0]);
		return -1;
	}

	return vertex_shader_id;
}

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
	GLuint vertex_shader_id = create_shader(vertex_file_path, GL_VERTEX_SHADER);
	GLuint fragment_shader_id = create_shader(fragment_file_path, GL_FRAGMENT_SHADER);

	GLuint program_id = glCreateProgram();
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);

	GLint link_result = GL_FALSE;
	int len_link_info_log;
	glGetProgramiv(program_id, GL_LINK_STATUS, &link_result);
	glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &len_link_info_log);
	
	if (len_link_info_log > 0) {
		std::vector<char> program_error_message(len_link_info_log + 1);
		glGetProgramInfoLog(program_id, len_link_info_log, nullptr, &program_error_message[0]);
		ML_LOG(Error, "%s\n", &program_error_message[0]);
	}

	glDetachShader(program_id, vertex_shader_id);
	glDetachShader(program_id, fragment_shader_id);

	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	return program_id;
}
