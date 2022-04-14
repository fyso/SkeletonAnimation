#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 ViewPos;
uniform vec3 LightDir;

void main()
{
	vec3 LightColor = vec3(1.0f);
	vec3 ObjectColor = vec3(0.627f, 0.576f, 0.741f);
	vec3 Norm = normalize(Normal);

	vec3 Ambient = 0.2 * LightColor;

	float Diff = max(dot(-LightDir, Norm), 0.0f);
	vec3 Diffuse = Diff * LightColor;

	vec3 Color = (Ambient + Diffuse) * ObjectColor;
	vec3 Color = vec3(0.0f��1.0f);
	FragColor = vec4(Color, 1.0);
}