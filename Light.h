#ifndef _LIGHT_H_
#define _LIGHT_H_

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

#include "Window.h"

class Light
{
public:
	Light();
	~Light();
	void draw(GLuint);

	glm::vec3 color;
	glm::vec3 direction;
};

#endif
