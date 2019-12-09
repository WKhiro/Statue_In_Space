#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "Window.h"

class Geometry
{
private:
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> normals;
	std::vector<float> textures;

	GLuint sphereVBO, sphereVBO2, sphereVBO3, sphereVAO, sphereEBO;

public:
	Geometry(std::string objFilename);
	~Geometry();

	void draw(glm::mat4 model, GLuint shader);
	void parse(std::string objFilename);
	void update();
};

#endif
