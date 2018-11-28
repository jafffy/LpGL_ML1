#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 uv;

out vec2 uv_interpolated;

void main()
{
	gl_Position.xyz = vertexPosition_modelspace;
	gl_Position.w = 1.0;

	uv_interpolated = uv;
}