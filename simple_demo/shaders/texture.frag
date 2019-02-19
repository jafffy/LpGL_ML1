#version 330 core

in vec2 uv_interpolated;

// out vec4 color;
out vec3 color;

uniform sampler2D MyTextureSampler;

void main()
{
	color = vec3(1, 0, 0);
}