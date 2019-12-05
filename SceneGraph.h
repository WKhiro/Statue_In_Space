#ifndef _SCENE_GRAPH_H_
#define _SCENE_GRAPH_H_

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

class Node
{
public:
	virtual void draw(glm::mat4 C) = 0;
	virtual void update() = 0;
	virtual void addChild(Node * child) = 0;
	virtual void removeChild(Node * child) = 0;

	glm::mat4 M;
};

class Group : public Node
{
private:
	glm::mat4 M;
	std::vector<Node*> children;
public:
	Group();
	~Group();
	void update();
	void draw(glm::mat4 C);
	void addChild(Node* child);
	void removeChild(Node* child);
};

class Transform : public Node
{
private:
	glm::mat4 M;
	std::vector<Node*> children;
public:
	Transform(glm::mat4 transform);
	~Transform();
	void draw(glm::mat4 C);
	void addChild(Node * child);
	void removeChild(Node * child);
	void update();
};

class Geometry : public Node
{
private:
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	std::vector<float> normals;

	GLuint sbuffIndex, shader, selectionShader, environmentShader;
	GLuint sphereVBO, sphereVBO2, sphereVAO, sphereEBO;
	GLuint projectionLoc, viewLoc, modelLoc, modelViewLoc;

	glm::vec3 diffuse, specular, ambient;
	float shininess;

	glm::mat4 M;
public:
	Geometry(std::string objFilename, GLuint program, GLuint environment, int color);
	~Geometry();

	void draw(glm::mat4 C);
	void parse(std::string objFilename);
	void addChild(Node * child);
	void removeChild(Node * child);
	void update();

	glm::vec3 center;
	glm::mat4 initial;
	int environmentMap, interp;
};

#endif
