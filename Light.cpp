#include "Light.h"
#include "Window.h"

Light::Light()
{
	color = { 1.0f, 1.0f, 1.0f };
	direction = { 0.0f, -1.0f, 0.0f };
}

void Light::draw(GLuint program)
{
	glUseProgram(program);

	glUniform1i(glGetUniformLocation(program, "light.normalColoring"), Window::normalColoring);
	glUniform3fv(glGetUniformLocation(program, "viewPos"), 1, &Window::view[0][0]);
	glUniform3fv(glGetUniformLocation(program, "light.color"), 1, &color[0]);
	glUniform3fv(glGetUniformLocation(program, "light.direction"), 1, &direction[0]);
}

Light::~Light() {}
