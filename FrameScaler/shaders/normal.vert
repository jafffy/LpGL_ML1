#version 330 core

layout (location = 0) in vec3 vertexPosition_modelspace;
layout (location = 1) in vec3 normal_modelspace;

out vec3 normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 vertex_color;

uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 color;

void main() {
	gl_Position = MVP * vec4(vertexPosition_modelspace, 1);

	vec3 vertexPosition_cameraspace = (V * M * vec4(vertexPosition_modelspace, 1)).xyz;
	EyeDirection_cameraspace = vec3(0, 0, 0) - vertexPosition_cameraspace;

	normal_cameraspace = (V * M * vec4(normal_modelspace, 0)).xyz;

	vertex_color = color;
}