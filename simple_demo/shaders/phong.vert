#version 330 core

layout (location = 0) in vec3 vertexPosition_modelspace;
layout (location = 1) in vec3 vertexNormal_modelspace;

out vec3 Position_worldspace;
out vec3 Normal_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 LightDirection_cameraspace;

uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;

out vec3 normal;

void main() {
	gl_Position = MVP * vec4(vertexPosition_modelspace, 1);

	vec3 vertexPosition_cameraspace = (V * (M * vec4(vertexPosition_modelspace, 1)).xyz;
	EyeDirection_cameraspace = vec3(0, 0, 0) - vertexPosition_cameraspace;

	vec3 LightDirection = vec4(1, 1, 1);
	LightDirection_cameraspace = V * normalize(LightDirection);
	Normal_cameraspace = (V * M * vec4(vertexNormal_modelspace, 0)).xyz;
}