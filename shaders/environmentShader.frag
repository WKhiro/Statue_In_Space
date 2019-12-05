#version 330 core

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube cube_texture;

out vec4 FragColor;

void main()
{
	vec3 I = normalize(Position - cameraPos);
	vec3 R = reflect(I, normalize(Normal));
	FragColor = vec4(texture(cube_texture, R).rgb, 1.0);
}