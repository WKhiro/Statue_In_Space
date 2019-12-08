#include "Window.h"
#include <time.h>
#include <string.h>

using namespace irrklang;

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
unsigned int depthMapFBO;
unsigned int depthMap;
unsigned int planeVAO;


float angle = 45;

unsigned int hdrFBO;
unsigned int colorBuffers[2];
unsigned int rboDepth;
unsigned int pingpongFBO[2];
unsigned int pingpongColorbuffers[2];
std::vector<glm::vec3> lightPositions;
std::vector<glm::vec3> lightColors;
float exposure = 1.0f;
bool bloomToggle = true;
bool bloomKeyPressed = false;

int bloomShadowToggle = true;
// Used for FPS controls
GLFWwindow* window;
glm::mat4 extra;

const char* window_title = "Shower Thoughts";

int Window::width;
int Window::height;
unsigned int loadTexture(const char* path, bool gammaCorrection);
ISoundEngine* SoundEngine = createIrrKlangDevice();
ISoundEngine* rainEngine = createIrrKlangDevice();
ISound* playingSound;

double Window::oldX = 0;
double Window::oldY = 0;

bool firstMouse;
bool movement = false;
double fov = 90.0f;
float yaw = -90.0f;
float pitch = 0.0f;

// Shaders 
GLint shader;
GLint skyShader;
GLint selection;
GLint environment;
GLint bloom;
GLint shaderLight;
GLint shaderBlur;
GLint shaderFinal;
GLint shadow;
GLint shadowDepth;
GLint shadowDebug;
GLuint vao, vbo;
unsigned int wood;

Geometry* suit;
Geometry* planet;
glm::vec3 Window::currentPos;
int Window::normalColoring = 0;

int mouse = 0;
bool raining = true;

glm::vec3 eye(0, 0, 5);
glm::vec3 center(0, 0, -1);
glm::vec3 up(0.0f, 1.0f, 0.0f);

glm::vec3 initialE(0, 0, 5);
glm::vec3 initialC(0, 0, -1);
glm::vec3 initialU(0.0f, 1.0f, 0.0f);

glm::mat4 Window::projection;
glm::mat4 Window::view = glm::lookAt(eye, eye+center, up);
// lighting info
// -------------
glm::vec3 lightPos;
#define MAXX 5000

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}


typedef struct 
{
	// Life
	bool alive;	// is the particle alive?
	float life;	// particle lifespan
	float fade; // decay
	// color
	float red;
	float green;
	float blue;
	// Position/direction
	float xpos;
	float ypos;
	float zpos;
	// Velocity/Direction, only goes down in y dir
	float vel;
	// Gravity
	float gravity;
} particles;

particles par_sys[MAXX];

bool Window::initializeProgram() 
{
	wood = loadTexture("wood.png", true); // note that we're loading the texture as an SRGB texture
	playingSound = rainEngine->play2D("audio/rain.mp3", true, false, true);
	skyShader = LoadShaders("shaders/toon.vert", "shaders/toon.frag");

	shadow = LoadShaders("shaders/shadowMap.vert", "shaders/shadowMap.frag");
	shadowDepth = LoadShaders("shaders/shadowDepth.vert", "shaders/shadowDepth.frag");
	shadowDebug = LoadShaders("shaders/debug.vert", "shaders/debug.frag");

	bloom = LoadShaders("shaders/bloom.vert", "shaders/bloom.frag");
	shaderLight = LoadShaders("shaders/bloom.vert", "shaders/lightBox.frag");
	shaderBlur = LoadShaders("shaders/blur.vert", "shaders/blur.frag");
	shaderFinal = LoadShaders("shaders/bloomFinal.vert", "shaders/bloomFinal.frag");

	if (!skyShader)
	{
		std::cerr << "Failed to initialize shader program (skyShader.frag/skyShader.vert)" << std::endl;
		return false;
	}

	return true;
}

void initParticles(int i) 
{
	par_sys[i].alive = true;
	par_sys[i].life = 5.0;
	par_sys[i].fade = float(rand() % 100); // 1000.0f + 0.003f;

	par_sys[i].xpos = (float)(rand() % 20) - (float)(rand() % 20);
	par_sys[i].ypos = (float)(rand() % 20);
	par_sys[i].zpos = (float)(rand() % 20) - (float)(rand() % 20);

	par_sys[i].vel = 0.0;
	par_sys[i].gravity = -0.8;
}

void drawRain(int direction) 
{
	float x, y, z;
	glm::mat4 model, MVP;
	glUseProgram(skyShader);
	glUniform3f(glGetUniformLocation(skyShader, "set_color"), 0.0f, 1.0f, 0.0f);

	for (int i = 0; i < MAXX; i++) 
	{
		if (par_sys[i].alive) 
		{
			x = par_sys[i].xpos + eye.x / 2;
			y = par_sys[i].ypos + eye.y / 2;
			z = par_sys[i].zpos + eye.z / 2;

			model = glm::translate(glm::mat4(1.0f), { x, y, z });
			glm::mat4 MVP = Window::projection * Window::view * model;

			glUniformMatrix4fv(glGetUniformLocation(skyShader, "MVP"), 1, GL_FALSE, &MVP[0][0]);

			// Draw particles
			glBegin(GL_POINTS);
			glVertex3f(x, y, z);
			glEnd();

			// Movement
			if (direction)
			{
				par_sys[i].ypos += par_sys[i].vel / (2.0 * 1000);
			}
			else
			{
				par_sys[i].xpos += par_sys[i].vel / (2.0 * 1000);
			}

			par_sys[i].vel += par_sys[i].gravity;
			// Decay
			par_sys[i].life -= par_sys[i].fade;

			if (par_sys[i].ypos <= -10) 
			{
				par_sys[i].life = -1.0;
			}
			//Revive
			if (par_sys[i].life < 0.0 && raining) 
			{
				initParticles(i);
			}
			else if (par_sys[i].life < 0.0 && !raining)
			{
				par_sys[i].alive = false;
			}
		}
		else if (par_sys[i].alive == false && raining)
		{
				initParticles(i);
		}
	}
}

unsigned int loadTexture(char const* path, bool gammaCorrection)
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

void renderScene(GLint shader)
{
	// floor
	glm::mat4 model = glm::mat4(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	/*// cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);
	renderCube();*/
}


bool Window::initializeObjects()
{
	glEnable(GL_DEPTH_TEST);
	lightPos = glm::vec3(-2.0f, 4.0f, -1.0f);

	// Roller Coaster
	suit = new Geometry("nanosuit.obj");
	planet = new Geometry("planet.obj");

	for (int i = 0; i < MAXX; i++) {
		initParticles(i);
	}
	float planeVertices[] = {
		// positions            // normals         // texcoords
		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}
	// create and attach depth buffer (renderbuffer)
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// ping-pong-framebuffer for blurring
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader configuration
// --------------------
	glUseProgram(shadow);
	glUniform1i(glGetUniformLocation(shadow, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shadow, "shadowMap"), 1);
	glUseProgram(shadowDebug);
	glUniform1i(glGetUniformLocation(shadowDebug, "depthMap"), 0);

	lightPositions.push_back(glm::vec3(15.0f, 0.5f, -10.5f));
	lightPositions.push_back(glm::vec3(15.0f, 0.5f, 10.5f));
	lightPositions.push_back(glm::vec3(-15.0f, 0.5f, 10.5f));
	lightPositions.push_back(glm::vec3(-15.0f, 0.5f, -10.5f));
	// colors
	lightColors.push_back(glm::vec3(5.0f, 5.0f, 5.0f));
	lightColors.push_back(glm::vec3(10.0f, 0.0f, 0.0f));
	lightColors.push_back(glm::vec3(0.0f, 0.0f, 15.0f));
	lightColors.push_back(glm::vec3(0.0f, 5.0f, 0.0f));

	// shader configuration
	// --------------------
	glUseProgram(bloom);
	glUniform1i(glGetUniformLocation(bloom, "diffuseTexture"), 0);
	glUseProgram(shaderBlur);
	glUniform1i(glGetUniformLocation(shaderBlur, "image"), 0);
	glUseProgram(shaderFinal);
	glUniform1i(glGetUniformLocation(shaderFinal, "scene"), 0);
	glUniform1i(glGetUniformLocation(shaderFinal, "bloomBlur"), 1);
	return true;
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}


// Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::cleanUp()
{
	glDeleteProgram(shader);
	glDeleteProgram(skyShader);
	glDeleteProgram(environment);
	SoundEngine->drop();
	rainEngine->drop();
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
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		eye += center * 0.05f;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		eye -= center * 0.05f;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		eye -= glm::normalize(glm::cross(center, up)) * 0.05f;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		eye += glm::normalize(glm::cross(center, up)) * 0.05f;
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		eye = initialE;
		center = initialC;
		up = initialU;
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
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
	else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		exposure += 0.1f;
	}
}

void Window::displayCallback(GLFWwindow* window)
{
	glm::mat4 model = glm::mat4(1.0f);
	angle += 1;
	if (bloomShadowToggle)
	{
		std::string firstStr = "lights[";
		std::string secondStr = "].Position";
		std::string thirdStr = "].Color";
		std::string position, color;

		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawRain(1);
		drawRain(0);

		glUseProgram(bloom);
		glUniformMatrix4fv(glGetUniformLocation(bloom, "projection"), 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(bloom, "view"), 1, GL_FALSE, &view[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wood);

		// Setup for lighting uniforms
		for (int i = 0; i < lightPositions.size(); i++)
		{
			position = firstStr + std::to_string(i) + secondStr;
			color = firstStr + std::to_string(i) + thirdStr;
			glUniform3fv(glGetUniformLocation(bloom, position.c_str()), 1, &lightPositions[i][0]);
			glUniform3fv(glGetUniformLocation(bloom, color.c_str()), 1, &lightColors[i][0]);
		}
		glUniform3f(glGetUniformLocation(bloom, "viewPos"), eye.x, eye.y, eye.z);

		renderScene(bloom);

		// finally show all the light sources as bright cubes
		glUseProgram(shaderLight);
		glUniformMatrix4fv(glGetUniformLocation(shaderLight, "projection"), 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(shaderLight, "view"), 1, GL_FALSE, &view[0][0]);

		for (unsigned int i = 0; i < lightPositions.size(); i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(lightPositions[i]));
			model = glm::scale(model, glm::vec3(1.0f));
			model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
			glUniformMatrix4fv(glGetUniformLocation(shaderLight, "model"), 1, GL_FALSE, &model[0][0]);
			glUniform3fv(glGetUniformLocation(shaderLight, "lightColor"), 1, &lightColors[i][0]);
			planet->draw(model, shaderLight);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. blur bright fragments with two-pass Gaussian Blur 
		// --------------------------------------------------
		bool horizontal = true, first_iteration = true;
		unsigned int amount = 10;
		glUseProgram(shaderBlur);
		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			glUniform1i(glGetUniformLocation(shaderBlur, "horizontal"), horizontal);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
			renderQuad();
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
		// --------------------------------------------------------------------------------------------------------------------------

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderFinal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
		glUniform1i(glGetUniformLocation(shaderFinal, "bloom"), bloomToggle);
		glUniform1f(glGetUniformLocation(shaderFinal, "exposure"), exposure);
		renderQuad();
		std::cout << "bloom: " << (bloom ? "on" : "off") << "| exposure: " << exposure << std::endl;


		//lightPos = glm::vec3(eye.x, eye.y, eye.z);
		// 1. render depth of scene to texture (from light's perspective)
			// --------------------------------------------------------------
	}
	else
	{
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	//lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	glUseProgram(shadowDepth);
	glUniformMatrix4fv(glGetUniformLocation(shadowDepth, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wood);
	renderScene(shadowDepth);

	suit->draw(glm::mat4(1.0f), shadowDepth);
	for (int i = 0; i < lightPositions.size(); i++)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(lightPositions[i]));
		model = glm::scale(model, glm::vec3(1.0f));
		model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
		planet->draw(model, shadowDepth);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// reset viewport
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2. render scene as normal using the generated depth/shadow map  
	// --------------------------------------------------------------
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shadow);
	glUniformMatrix4fv(glGetUniformLocation(shadow, "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shadow, "view"), 1, GL_FALSE, &view[0][0]);
	// set light uniforms
	glUniform3f(glGetUniformLocation(shadow, "viewPos"), eye.x, eye.y, eye.z);
	glUniform3f(glGetUniformLocation(shadow, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glUniformMatrix4fv(glGetUniformLocation(shadow, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wood);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	renderScene(shadow);
	suit->draw(glm::mat4(1.0f), shadow);

	for (int i = 0; i < lightPositions.size(); i++)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(lightPositions[i]));
		model = glm::scale(model, glm::vec3(1.0f));
		model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
		planet->draw(model, shadow);
	}

	// render Depth map to quad for visual debugging
	// ---------------------------------------------
	glUseProgram(shadowDebug);
	glUniform1f(glGetUniformLocation(shadowDebug, "near_plane"), near_plane);
	glUniform1f(glGetUniformLocation(shadowDebug, "far_plane"), far_plane);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glViewport(width * 0.8, 0, width * 0.2, height * 0.2);
	renderQuad();
	glViewport(0, 0, width, height);
	drawRain(1);
	drawRain(0);
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
		case GLFW_KEY_SPACE:
			SoundEngine->stopAllSounds();
			if (bloomToggle)
			{
				SoundEngine->play2D("audio/off.mp3", GL_FALSE);
			}
			else
			{
				SoundEngine->play2D("audio/on.mp3", GL_FALSE);
			}
			bloomToggle = !bloomToggle;
			break;
		case GLFW_KEY_N: // Normal coloring
			if (normalColoring) 
			{
				normalColoring = 0;
			}
			else 
			{
				normalColoring = 1;
			}
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
		case GLFW_KEY_P: // Pause and play sounds
			SoundEngine->stopAllSounds();
			if (raining)
			{
				raining = false;
				playingSound->setIsPaused(true);
				SoundEngine->play2D("audio/off.mp3", GL_FALSE);
			}
			else
			{
				SoundEngine->play2D("audio/on.mp3", GL_FALSE);
				raining = true;
				playingSound->setIsPaused(false);
			}
			break;
		default:
			break;
		}
	}
}