#version 330 core

in vec3 normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 vertex_color;

out vec3 out_color;
uniform mat4 M;
uniform mat4 V;

uniform vec3 color;

void main()
{
	vec3 LightColor = vec3(1, 1, 1);
	float LightPower = 0.5f;

	vec3 MaterialDiffuseColor = vec3(1, 1, 1);
	vec3 MaterialAmbientColor = vec3(0.1, 0.1, 0.1) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.3, 0.3, 0.3);

	vec3 n = normalize(normal_cameraspace);
	vec3 l = normalize(normalize(vec3(1, 1, 1)));
	float cosTheta = clamp(dot(n, l), 0, 1);

	vec3 E = normalize(EyeDirection_cameraspace);
	vec3 R = reflect(-l, n);
	float cosAlpha = clamp(dot(E, R), 0, 1);

	out_color = MaterialAmbientColor + 
		MaterialDiffuseColor * LightColor * LightPower * cosTheta +
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha, 5);
}