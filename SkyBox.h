#ifndef _SKY_BOX_H_
#define _SKY_BOX_H_

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

#include "Window.h"

class SkyBox
{
private:
	float skyBoxVertices[108] = {
		-500.0f,  500.0f, -500.0f,
		-500.0f, -500.0f, -500.0f,
		500.0f, -500.0f, -500.0f,
		500.0f, -500.0f, -500.0f,
		500.0f,  500.0f, -500.0f,
		-500.0f,  500.0f, -500.0f,

		-500.0f, -500.0f,  500.0f,
		-500.0f, -500.0f, -500.0f,
		-500.0f,  500.0f, -500.0f,
		-500.0f,  500.0f, -500.0f,
		-500.0f,  500.0f,  500.0f,
		-500.0f, -500.0f,  500.0f,

		500.0f, -500.0f, -500.0f,
		500.0f, -500.0f,  500.0f,
		500.0f,  500.0f,  500.0f,
		500.0f,  500.0f,  500.0f,
		500.0f,  500.0f, -500.0f,
		500.0f, -500.0f, -500.0f,

		-500.0f, -500.0f,  500.0f,
		-500.0f,  500.0f,  500.0f,
		500.0f,  500.0f,  500.0f,
		500.0f,  500.0f,  500.0f,
		500.0f, -500.0f,  500.0f,
		-500.0f, -500.0f,  500.0f,

		-500.0f,  500.0f, -500.0f,
		500.0f,  500.0f, -500.0f,
		500.0f,  500.0f,  500.0f,
		500.0f,  500.0f,  500.0f,
		-500.0f,  500.0f,  500.0f,
		-500.0f,  500.0f, -500.0f,

		-500.0f, -500.0f, -500.0f,
		-500.0f, -500.0f,  500.0f,
		500.0f, -500.0f, -500.0f,
		500.0f, -500.0f, -500.0f,
		-500.0f, -500.0f,  500.0f,
		500.0f, -500.0f,  500.0f
	};
public:
	SkyBox();
	~SkyBox();

	GLuint VBO, VAO;
	void draw(GLuint program, unsigned int cubeMapTexture);
};

#endif
