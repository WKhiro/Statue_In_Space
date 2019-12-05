#ifndef _WINDOW_H_
#define _WINDOW_H_

#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string>

#include "shader.h"
#include "stb_image.h"
#include "SkyBox.h"
#include "SceneGraph.h"
#include "Light.h"

class Geometry;

class Window
{
public:
	static int width;
	static int height;
	static glm::mat4 projection; 
	static glm::mat4 view; 
	static double oldX, oldY;

	static int Window::normalColoring;

	static glm::vec3 Window::currentPos;

	static glm::vec3 Window::old_pos;
	static glm::vec3 Window::new_pos;
	static float Window::carT;
	static glm::mat4 Window::roller;

	static std::vector<Geometry*> sbuff;
	static GLuint sbuffIndex;
	static GLuint sbuffOn;

	static bool initializeProgram();
	static bool initializeObjects();
	static void cleanUp();
	static GLFWwindow* createWindow(int width, int height);
	static void resizeCallback(GLFWwindow* window, int width, int height);
	static void idleCallback();
	static void displayCallback(GLFWwindow*);
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	static glm::vec3 trackBallMap(glm::vec2 point);
	static unsigned int loadCubeMap(std::vector<std::string> faces);
};

#endif
