#include "Window.h"
#include <time.h>
#include <string.h>

using namespace irrklang;

#define MAXPARTICLES 5000

const char* window_title = "Statue in Space";
GLFWwindow* window;
int Window::width, Window::height;

typedef struct
{
	bool alive;
	float lifespan, fade, velocity, gravity;
	float x, y, z;
} particles;

particles starSystem[MAXPARTICLES];

float floorVertices[] =
{
	// Positions          // Normals         // Texture coordinates
	10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
   -10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
   -10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

	10.0f, 0.0f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
   -10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
	10.0f, 0.0f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
};

float quadVertices[] =
{
	// Positions         // Texture coordinates
	-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
	 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
	 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
};

const unsigned int shadowWidth = 1024, shadowHeight = 1024;

unsigned int floorVAO, floorVBO, quadVAO, quadVBO,
depthMapFBO, depthMap, hdrFBO, colorBuffers[2], rboDepth,
pingpongFBO[2], pingpongColorbuffers[2];

glm::vec3 lightPos, orbitCoordinates;

std::vector<glm::vec3> lightPositions, lightColors;

bool bloomToggle = true, bloomShadowToggle = true,
bloomKeyPressed = false, lightMovement = false,
firstMouse = false, movement = false,
shadowToggle = true, starsToggle = true;

float angle, radian, radius, 
orbitX, orbitY, orbitZ,
exposure = 1.0f, 
yaw = -90.0f, pitch = 0.0f;

double Window::oldX = 0.0f, Window::oldY = 0.0f, 
fov = 90.0f;

// Shaders 
GLint starShader, 
shadow, shadowDepth, shadowDebug,
bloom, shaderLight, shaderBlur, shaderFinal;

// Sound
ISoundEngine* SoundEngine = createIrrKlangDevice();
ISoundEngine* StarEngine = createIrrKlangDevice();
ISound* playingSound;

// Textures
unsigned int metal;

// Models
Geometry* suit;
Geometry* planet;

// Camera coordinates
glm::vec3 eye(0.0f, 7.0f, 15.0f);
glm::vec3 center(0.0f, 0.0f, -1.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

// Used to reset camera
glm::vec3 initialE(0.0f, 7.0f, 15.0f);
glm::vec3 initialC(0.0f, 0.0f, -1.0f);
glm::vec3 initialU(0.0f, 1.0f, 0.0f);

glm::mat4 Window::projection;
glm::mat4 Window::view = glm::lookAt(eye, eye + center, up);

unsigned int Window::loadTexture(char const* path, bool gammaCorrection)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum internalFormat;
		GLenum dataFormat;
		if (nrComponents == 1)
		{
			internalFormat = dataFormat = GL_RED;
		}
		else if (nrComponents == 3)
		{
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
			dataFormat = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
			dataFormat = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	return textureID;
}

void Window::initParticles(int i)
{
	starSystem[i].alive = true;
	starSystem[i].lifespan = 5.0;
	starSystem[i].fade = float(rand() % 100);

	starSystem[i].x = (float)(rand() % 20) - (float)(rand() % 20);
	starSystem[i].y = (float)(rand() % 20);
	starSystem[i].z = (float)(rand() % 20) - (float)(rand() % 20);

	starSystem[i].velocity = 0.0;
	starSystem[i].gravity = -0.8;
}

void Window::drawParticles(int directionOfParticles)
{
	float x, y, z;
	glm::mat4 model, MVP;

	glUseProgram(starShader);
	glUniform3f(glGetUniformLocation(starShader, "set_color"), 0.0f, 1.0f, 0.0f);

	for (int i = 0; i < MAXPARTICLES; i++)
	{
		if (starSystem[i].alive)
		{
			x = starSystem[i].x + eye.x / 2;
			y = starSystem[i].y + eye.y / 2;
			z = starSystem[i].z + eye.z / 2;

			model = glm::translate(glm::mat4(1.0f), { x, y, z });
			MVP = Window::projection * Window::view * model;

			glUniformMatrix4fv(glGetUniformLocation(starShader, "MVP"), 1, GL_FALSE, &MVP[0][0]);

			// Draw particles
			glBegin(GL_POINTS);
			glVertex3f(x, y, z);
			glEnd();

			// Direction of movement
			if (directionOfParticles)
			{
				starSystem[i].y += starSystem[i].velocity / (2.0 * 1000);
			}
			else
			{
				starSystem[i].x += starSystem[i].velocity / (2.0 * 1000);
			}

			// Speed and lifespan
			starSystem[i].velocity += starSystem[i].gravity;
			starSystem[i].lifespan -= starSystem[i].fade;

			if (starSystem[i].y <= -10)
			{
				starSystem[i].lifespan = -1.0;
			}

			// Bring back the particles if they're turned on
			if (starSystem[i].lifespan < 0.0 && starsToggle)
			{
				initParticles(i);
			}
			else if (starSystem[i].lifespan < 0.0 && !starsToggle)
			{
				starSystem[i].alive = false;
			}
		}
		else if (starSystem[i].alive == false && starsToggle)
		{
			initParticles(i);
		}
	}
}

void Window::renderFloor(GLint shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);
	glBindVertexArray(floorVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Window::renderQuad()
{
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

bool Window::initializeProgram() 
{
	metal = loadTexture("textures/metal.png", true);
	playingSound = StarEngine->play2D("audio/rain.mp3", true, false, true);
	starShader = LoadShaders("shaders/stars.vert", "shaders/stars.frag");

	shadow = LoadShaders("shaders/shadowMap.vert", "shaders/shadowMap.frag");
	shadowDepth = LoadShaders("shaders/shadowDepth.vert", "shaders/shadowDepth.frag");
	shadowDebug = LoadShaders("shaders/debug.vert", "shaders/debug.frag");

	bloom = LoadShaders("shaders/bloom.vert", "shaders/bloom.frag");
	shaderLight = LoadShaders("shaders/bloom.vert", "shaders/lightBox.frag");
	shaderBlur = LoadShaders("shaders/blur.vert", "shaders/blur.frag");
	shaderFinal = LoadShaders("shaders/bloomFinal.vert", "shaders/bloomFinal.frag");
	
	if (!starShader)
	{
		std::cerr << "Failed to initialize shader program (starShader.frag/starShader.vert)" << std::endl;
		return false;
	}
	if (!shadow)
	{
		std::cerr << "Failed to initialize shader program (shadowMap.frag/shadowMap.vert)" << std::endl;
		return false;
	}
	if (!shadowDepth)
	{
		std::cerr << "Failed to initialize shader program (shadowDepth.frag/shadowDepth.vert)" << std::endl;
		return false;
	}
	if (!shadowDebug)
	{
		std::cerr << "Failed to initialize shader program (debug.frag/debug.vert)" << std::endl;
		return false;
	}
	if (!bloom)
	{
		std::cerr << "Failed to initialize shader program (bloom.frag/bloom.vert)" << std::endl;
		return false;
	}
	if (!shaderLight)
	{
		std::cerr << "Failed to initialize shader program (lightBox.frag/bloom.vert)" << std::endl;
		return false;
	}
	if (!shaderBlur)
	{
		std::cerr << "Failed to initialize shader program (blur.frag/blur.vert)" << std::endl;
		return false;
	}
	if (!shaderFinal)
	{
		std::cerr << "Failed to initialize shader program (bloomFinal.frag/bloomFinal.vert)" << std::endl;
		return false;
	}

	return true;
}

bool Window::initializeObjects()
{
	glEnable(GL_DEPTH_TEST);

	// Default light position
	lightPos = glm::vec3(4.0f, 3.0f, 3.0f);

	suit = new Geometry("objects/nanosuit.obj");
	planet = new Geometry("objects/planet.obj");

	for (int i = 0; i < MAXPARTICLES; i++) 
	{
		initParticles(i);
	}

	// Generate buffer for floor
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);
	glBindVertexArray(floorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// Depth map for shadows
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Bloom setup
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// Create 2 float color buffers (normal rendering / brightness threshold values)
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// Attach texture
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}

	// Attach depth buffer
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Ping-pong FBO for blur
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Framebuffer not complete!" << std::endl;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Configure bloom shaders
	glUseProgram(bloom);
	glUniform1i(glGetUniformLocation(bloom, "diffuseTexture"), 0);
	glUseProgram(shaderBlur);
	glUniform1i(glGetUniformLocation(shaderBlur, "image"), 0);
	glUseProgram(shaderFinal);
	glUniform1i(glGetUniformLocation(shaderFinal, "scene"), 0);
	glUniform1i(glGetUniformLocation(shaderFinal, "bloomBlur"), 1);

	// Configure shadow shaders
	glUseProgram(shadow);
	glUniform1i(glGetUniformLocation(shadow, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shadow, "shadowMap"), 1);
	glUseProgram(shadowDebug);
	glUniform1i(glGetUniformLocation(shadowDebug, "depthMap"), 0);

	// Set orb colors
	lightColors.push_back(glm::vec3(5.0f, 5.0f, 5.0f));
	lightColors.push_back(glm::vec3(5.0f, 0.0f, 0.0f));
	lightColors.push_back(glm::vec3(5.0f, 2.0f, 0.0f));
	lightColors.push_back(glm::vec3(0.0f, 5.0f, 0.0f));
	lightColors.push_back(glm::vec3(5.0f, 5.0f, 0.0f));
	lightColors.push_back(glm::vec3(0.0f, 5.0f, 5.0f));
	lightColors.push_back(glm::vec3(5.0f, 0.0f, 5.0f));

	return true;
}

// Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::cleanUp()
{
	glDeleteProgram(starShader);
	glDeleteProgram(shadow);
	glDeleteProgram(shadowDepth);
	glDeleteProgram(shadowDebug);
	glDeleteProgram(bloom);
	glDeleteProgram(shaderLight);
	glDeleteProgram(shaderBlur);
	glDeleteProgram(shaderFinal);
	suit->~Geometry();
	planet->~Geometry();
	SoundEngine->drop();
	StarEngine->drop();
}

GLFWwindow* Window::createWindow(int width, int height)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__ // Because Apple hates comforming to standards
	// Ensure that minimum OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the GLFW window
	window = glfwCreateWindow(width, height, window_title, NULL, NULL);

	// Check if the window could not be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		fprintf(stderr, "Either GLFW is not installed or your graphics card does not support modern OpenGL.\n");
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window
	glfwMakeContextCurrent(window);

	// Set swap interval to 1
	glfwSwapInterval(1);

	// Get the width and height of the framebuffer to properly resize the window
	glfwGetFramebufferSize(window, &width, &height);
	// Call the resize callback to make sure things get drawn immediately
	Window::resizeCallback(window, width, height);

	return window;
}

void Window::resizeCallback(GLFWwindow* window, int width, int height)
{
#ifdef __APPLE__
	// In case your Mac has a retina display.
	glfwGetFramebufferSize(window, &width, &height);
#endif
	Window::width = width;
	Window::height = height;
	// Set the viewport size.
	glViewport(0, 0, width, height);

	// Set the projection matrix.
	Window::projection = glm::perspective(glm::radians(fov),
		double(width) / (double)height, 1.0, 1000.0);
}

void Window::idleCallback()
{
	Window::projection = glm::perspective(glm::radians(fov),
		double(width) / (double)height, 1.0, 1000.0);
	Window::view = glm::lookAt(eye, eye + center, up);

	// FPS movement controls / lighting controls
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		if (lightMovement && !bloomShadowToggle)
		{
			lightPos.y += 0.05;
		}
		else
		{
			eye += center * 0.1f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		if (lightMovement && !bloomShadowToggle)
		{
			lightPos.y -= 0.05;
		}
		else
		{
			eye -= center * 0.1f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		if (lightMovement && !bloomShadowToggle)
		{
			lightPos.x -= 0.05;
		}
		else
		{
			eye -= glm::normalize(glm::cross(center, up)) * 0.1f;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		if (lightMovement && !bloomShadowToggle)
		{
			lightPos.x += 0.05;
		}
		else
		{
			eye += glm::normalize(glm::cross(center, up)) * 0.1f;
		}
	}

	if (lightMovement && !bloomShadowToggle)
	{
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			lightPos.z -= 0.05;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			lightPos.z += 0.05;
		}
	}

	// Resets camera view
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		eye = initialE;
		center = initialC;
		up = initialU;
	}

	// Exposure for bloom
	if (bloomShadowToggle)
	{
		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		{
			if (exposure > 0.0f)
			{
				exposure -= 0.1f;
			}
			else
			{
				exposure = 0.0f;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		{
			exposure += 0.1f;
		}
	}
}

void Window::displayCallback(GLFWwindow* window)
{
	glm::mat4 model = glm::mat4(1.0f);

	// Reset the light positions
	lightPositions.clear();

	// Increment angle for orbiting
	angle += 1.0f;

	if (angle > 360.0f)
	{
		angle = 0.0f;
	}
	radian = glm::radians(angle);

	// Calculate orbit for all orbs
	radius = 0.1f;
	orbitX = (radius * cosf(radian));
	orbitY = 1.0f;
	orbitZ = 4.0f + (radius * sinf(radian));

	orbitCoordinates = glm::vec3(orbitX, orbitY, orbitZ);
	lightPositions.push_back(orbitCoordinates);

	radius = 3.0f;
	orbitX = (radius * cosf(radian));
	orbitY = 12.0f;
	orbitZ = (radius * sinf(radian));

	orbitCoordinates = glm::vec3(orbitX, orbitY, orbitZ);
	lightPositions.push_back(orbitCoordinates);

	radius = 2.0f;
	orbitX = (radius * cosf(radian)) * -1.0f;
	orbitY = 14.0f;
	orbitZ = (radius * sinf(radian)) * -1.0f;

	orbitCoordinates = glm::vec3(orbitX, orbitY, orbitZ);
	lightPositions.push_back(orbitCoordinates);

	radius = 1.0f;
	orbitX = 3.5f + (radius * cosf(radian));
	orbitY = 8.0f;
	orbitZ = 1.2f + (radius * sinf(radian));

	orbitCoordinates = glm::vec3(orbitX, orbitY, orbitZ);
	lightPositions.push_back(orbitCoordinates);

	orbitX = -3.5f + (radius * cosf(radian)) * -1.0f;
	orbitZ = 1.2f + (radius * sinf(radian)) * -1.0f;

	orbitCoordinates = glm::vec3(orbitX, orbitY, orbitZ);
	lightPositions.push_back(orbitCoordinates);

	orbitX = -1.6f + (radius * cosf(radian));
	orbitY = 1.5f;
	orbitZ = -0.5f + (radius * sinf(radian));

	orbitCoordinates = glm::vec3(orbitX, orbitY, orbitZ);
	lightPositions.push_back(orbitCoordinates);

	orbitX = 1.6f + (radius * cosf(radian)) * -1.0f;
	orbitZ = -0.5f + (radius * sinf(radian)) * -1.0f;

	orbitCoordinates = glm::vec3(orbitX, orbitY, orbitZ);
	lightPositions.push_back(orbitCoordinates);

	// Bloom is on
	if (bloomShadowToggle)
	{
		std::string firstStr = "lights[";
		std::string secondStr = "].Position";
		std::string thirdStr = "].Color";
		std::string position, color;

		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawParticles(0);
		drawParticles(1);

		glUseProgram(bloom);
		glUniformMatrix4fv(glGetUniformLocation(bloom, "projection"), 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(bloom, "view"), 1, GL_FALSE, &view[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, metal);
		suit->draw(glm::mat4(1), bloom);

		// Setup for lighting uniforms
		for (int i = 0; i < lightPositions.size(); i++)
		{
			position = firstStr + std::to_string(i) + secondStr;
			color = firstStr + std::to_string(i) + thirdStr;
			glUniform3fv(glGetUniformLocation(bloom, position.c_str()), 1, &lightPositions[i][0]);
			glUniform3fv(glGetUniformLocation(bloom, color.c_str()), 1, &lightColors[i][0]);
		}
		glUniform3f(glGetUniformLocation(bloom, "viewPos"), eye.x, eye.y, eye.z);
		renderFloor(bloom);

		// Make light orbs
		glUseProgram(shaderLight);
		glUniformMatrix4fv(glGetUniformLocation(shaderLight, "projection"), 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shaderLight, "view"), 1, GL_FALSE, &view[0][0]);

		for (int i = 0; i < lightPositions.size(); i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(lightPositions[i]));
			model = glm::scale(model, glm::vec3(0.1f));
			glUniformMatrix4fv(glGetUniformLocation(shaderLight, "model"), 1, GL_FALSE, &model[0][0]);
			glUniform3fv(glGetUniformLocation(shaderLight, "lightColor"), 1, &lightColors[i][0]);
			planet->draw(model, shaderLight);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Create blur with two-pass Gaussian Blur 
		bool horizontal = true, first_iteration = true;
		unsigned int amount = 10;
		glUseProgram(shaderBlur);

		for (int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			glUniform1i(glGetUniformLocation(shaderBlur, "horizontal"), horizontal);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
			renderQuad();
			horizontal = !horizontal;
			if (first_iteration)
			{
				first_iteration = false;
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderFinal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
		glUniform1i(glGetUniformLocation(shaderFinal, "bloom"), bloomToggle);
		glUniform1f(glGetUniformLocation(shaderFinal, "exposure"), exposure);
		renderQuad();
	}
	else // Shadows are on
	{
		// Render depth of scene to texture based on light
		glm::mat4 lightProjection, lightView, lightSpaceMatrix;
		float near_plane = 1.0f, far_plane = 7.5f;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 20.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		// Render scene from light's POV
		glUseProgram(shadowDepth);
		glUniformMatrix4fv(glGetUniformLocation(shadowDepth, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

		glViewport(0, 0, shadowWidth, shadowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, metal);
		renderFloor(shadowDepth);
		suit->draw(glm::mat4(1.0f), shadowDepth);

		for (int i = 0; i < lightPositions.size(); i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(lightPositions[i]));
			model = glm::scale(model, glm::vec3(0.1f));
			planet->draw(model, shadowDepth);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Reset viewport
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render scene normally using shadow map  
		glUseProgram(shadow);
		glUniformMatrix4fv(glGetUniformLocation(shadow, "projection"), 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shadow, "view"), 1, GL_FALSE, &view[0][0]);
		// Set lighting uniforms
		glUniform3f(glGetUniformLocation(shadow, "viewPos"), eye.x, eye.y, eye.z);
		glUniform3f(glGetUniformLocation(shadow, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(glGetUniformLocation(shadow, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
		glUniform1i(glGetUniformLocation(shadow, "shadowToggle"), shadowToggle);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, metal);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		renderFloor(shadow);
		suit->draw(glm::mat4(1.0f), shadow);

		for (int i = 0; i < lightPositions.size(); i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(lightPositions[i]));
			model = glm::scale(model, glm::vec3(0.1f));
			planet->draw(model, shadow);
		}

		// Debug window
		glUseProgram(shadowDebug);
		glUniform1f(glGetUniformLocation(shadowDebug, "near_plane"), near_plane);
		glUniform1f(glGetUniformLocation(shadowDebug, "far_plane"), far_plane);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glViewport(width * 0.7, height * 0.7, width * 0.3, height * 0.3);
		renderQuad();
		glViewport(0, 0, width, height);
		drawParticles(0);
		drawParticles(1);
	}

	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 90.0f)
	{
		fov -= yoffset;
	}
	if (fov <= 1.0f)
	{
		fov = 1.0f;
	}
	if (fov >= 90.0f)
	{
		fov = 90.0f;
	}
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		movement = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		movement = false;
		firstMouse = true;
	}
}

void Window::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (movement == true)
	{
		glfwGetCursorPos(window, &xpos, &ypos);

		if (firstMouse)
		{
			oldX = xpos;
			oldY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - oldX;
		float yoffset = oldY - ypos; // reversed since y-coordinates go from bottom to top

		float sensitivity = 0.2f; // change this value to your liking
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		// Prevents screen flipping for the pitch
		if (pitch > 89.0f)
		{
			pitch = 89.0f;
		}
		if (pitch < -89.0f)
		{
			pitch = -89.0f;
		}

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		center = glm::normalize(front);

		oldX = xpos;
		oldY = ypos;
	}
}

glm::vec3 Window::trackBallMap(glm::vec2 point)
{
	glm::vec3 v;
	float d;
	v.x = (2.0f * point.x - width) / width;
	v.y = (height - 2.0f * point.y) / height;
	v.z = 0;
	d = glm::length(v);
	d = (d < 1.0) ? d : 1.0f;
	v.z = sqrtf(1.001f - d * d);
	v = glm::normalize(v);
	return v;
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Check for a key press
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		case GLFW_KEY_SPACE: // Toggles shadows or bloom based on current scene
			SoundEngine->stopAllSounds();
			if (bloomShadowToggle)
			{
				if (bloomToggle)
				{
					SoundEngine->play2D("audio/off.mp3", GL_FALSE);
				}
				else
				{
					SoundEngine->play2D("audio/on.mp3", GL_FALSE);
				}
				bloomToggle = !bloomToggle;
			}
			else
			{
				if (shadowToggle)
				{
					SoundEngine->play2D("audio/off.mp3", GL_FALSE);
				}
				else
				{
					SoundEngine->play2D("audio/on.mp3", GL_FALSE);
				}
				shadowToggle = !shadowToggle;
			}
			break;
		case GLFW_KEY_M: // Toggles WASD to move the lighting
			lightMovement = !lightMovement;
			break;
		case GLFW_KEY_T: // Toggle between bloom and shadows
			if (bloomShadowToggle)
			{
				bloomShadowToggle = false;
			}
			else
			{
				bloomShadowToggle = true;
			}
			break;
		case GLFW_KEY_P: // Toggles stars
			SoundEngine->stopAllSounds();
			if (starsToggle)
			{
				starsToggle = false;
				playingSound->setIsPaused(true);
				SoundEngine->play2D("audio/off.mp3", GL_FALSE);
			}
			else
			{
				SoundEngine->play2D("audio/on.mp3", GL_FALSE);
				starsToggle = true;
				playingSound->setIsPaused(false);
			}
			break;
		default:
			break;
		}
	}
}