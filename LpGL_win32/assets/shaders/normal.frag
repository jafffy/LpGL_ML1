#version 330 core

in vec3 normal_cameraspace;
out vec3 EyeDirection_cameraspace;

out vec3 color;
uniform mat4 M;
uniform mat4 V;

void main()
{
	vec3 LightColor = vec3(1, 1, 1);
	float LightPower = 50.0f;

	vec3 MaterialDiffuseColor = vec3(1, 1, 1);
	vec3 MaterialAmbientColor = vec3(0.1, 0.1, 0.1) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.3, 0.3, 0.3);

	vec3 n = normalize(normal_cameraspace);
	vec3 l = normalize(normalize(vec3(1, 1, 1)));
	float cosTheta = clamp(dot(n, l), 0, 1);

	vec3 E = normalize(EyeDirection_cameraspace);
	vec3 R = reflect(-l, n);
	float cosAlpha = clamp(dot(E, R), 0, 1);

	color = MaterialAmbientColor + 
		MaterialDiffuseColor * LightColor * LightPower * cosTheta +
		MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha, 5);
}