#version 330 core

in vec3 Normal;
in vec3 FragPos;

struct Light
{
	int normalColoring;
	int type;

	vec3 color;
	vec3 direction;
};

uniform vec3 viewPos;
uniform Light light;
uniform vec3 diffuseVal;
uniform vec3 specularVal;
uniform vec3 ambientVal;
uniform float shininessVal;

out vec4 color;

void main()
{
	if (light.normalColoring == 1) 
	{
		color = vec4(0, 0, 1, 1.0f);
	}
	else 
	{
		vec3 lightColor;
		vec3 lightDir;
		float denom;

		lightColor = light.color;
		lightDir = normalize(-light.direction);

		vec3 norm = normalize(Normal);
		vec3 viewDir = normalize(viewPos - FragPos);
		vec3 diffuse = lightColor*diffuseVal*(max(dot(norm, lightDir), 0.0));
		vec3 reflectDir = 2*max(dot(norm, lightDir), 0.0)*norm-lightDir;
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininessVal);
		vec3 specular = lightColor*specularVal*spec;
		vec3 ambient = lightColor*ambientVal;
		color = vec4((diffuse + ambient + specular), 1.0);
	}
}

