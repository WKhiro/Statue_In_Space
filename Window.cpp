#include "Window.h"
#include <time.h>

using namespace irrklang;

GLFWwindow* window;

const char* window_title = "Shower Thoughts";

int Window::width;
int Window::height;

ISoundEngine* SoundEngine = createIrrKlangDevice();

double Window::oldX = 0;
double Window::oldY = 0;

bool firstMouse;
bool movement = false;
double fov = 60.0f;
float yaw = -90.0f;
float pitch = 0.0f;

// Shaders 
GLint shader;
GLint skyShader;
GLint selection;
GLint environment;
GLuint vao, vbo;

unsigned int cubeMapTexture;
SkyBox * skyBox;

Geometry* rollerCoaster;

glm::vec3 Window::currentPos;
int Window::normalColoring = 0;

float angle;
int mouse = 0;
bool pause = false;
unsigned int val;

glm::vec3 eye(0, 20, 20);
glm::vec3 center(0, 20, 0);
glm::vec3 up(0.0f, 1.0f, 0.0f);

glm::mat4 Window::projection;
glm::mat4 Window::view = glm::lookAt(eye, eye+center, up);

#define MAXX 10000

typedef struct {
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
}particles;

particles par_sys[MAXX];

std::vector<std::string> faces = 
{
	"skyboxtextures/right.jpg",
	"skyboxtextures/left.jpg",
	"skyboxtextures/top.jpg",
	"skyboxtextures/bottom.jpg",
	"skyboxtextures/front.jpg",
	"skyboxtextures/back.jpg"
};

bool Window::initializeProgram() 
{
	SoundEngine->play2D("audio/rain.mp3", GL_TRUE);
	shader = LoadShaders("shaders/shader.vert", "shaders/shader.frag");
	skyShader = LoadShaders("shaders/skyShader.vert", "shaders/skyShader.frag");
	selection = LoadShaders("shaders/selectionShader.vert", "shaders/selectionShader.frag");
	environment = LoadShaders("shaders/environmentShader.vert", "shaders/environmentShader.frag");

	if (!shader)
	{
		std::cerr << "Failed to initialize shader program (shader.frag/shader.vert)" << std::endl;
		return false;
	}
	if (!skyShader)
	{
		std::cerr << "Failed to initialize shader program (skyShader.frag/skyShader.vert)" << std::endl;
		return false;
	}
	if (!selection)
	{
		std::cerr << "Failed to initialize shader program (selectionShader.frag/selectionShader.vert)" << std::endl;
		return false;
	}
	if (!environment)
	{
		std::cerr << "Failed to initialize shader program (environmentShader.frag/environmentShader.vert)" << std::endl;
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

	par_sys[i].red = 1.0;
	par_sys[i].green = 0.0;
	par_sys[i].blue = 0.0;

	par_sys[i].vel = 0.0;
	par_sys[i].gravity = -0.8;//-0.8;

}

void drawRain() {
	float x, y, z;
	for (int loop = 0; loop < MAXX; loop++) 
	{
		if (par_sys[loop].alive == true) 
		{
			x = par_sys[loop].xpos + eye.x/2;
			y = par_sys[loop].ypos + eye.y/2;
			z = par_sys[loop].zpos + eye.z/2; //zoom
			// Draw particles
			glBegin(GL_POINTS);
			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(x, y, z);
			glVertex3f(x, y + 0.5, z);
			glEnd();

			// Movement
			// Adjust slowdown for speed!
			par_sys[loop].ypos += par_sys[loop].vel / (2.0 * 1000);
			par_sys[loop].vel += par_sys[loop].gravity;
			// Decay
			par_sys[loop].life -= par_sys[loop].fade;

			if (par_sys[loop].ypos <= -10) 
			{
				par_sys[loop].life = -1.0;
			}
			//Revive
			if (par_sys[loop].life < 0.0) 
			{
				initParticles(loop);
			}
		}
	}
}

bool Window::initializeObjects()
{
	glEnable(GL_DEPTH_TEST);

	// Roller Coaster
	rollerCoaster = new Geometry("nanosuit.obj", shader, 0, 2);
	rollerCoaster->environmentMap = 0;

	for (int loop = 0; loop < MAXX; loop++) {
		initParticles(loop);
	}

	cubeMapTexture = loadCubeMap(faces);
	skyBox = new SkyBox();

	return true;
}

// Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::cleanUp()
{
	glDeleteProgram(shader);
	glDeleteProgram(skyShader);
	glDeleteProgram(selection);
	glDeleteProgram(environment);
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
		eye += center * 0.05f;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		eye -= center * 0.05f;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		eye -= glm::normalize(glm::cross(center, up)) * 0.05f;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		eye += glm::normalize(glm::cross(center, up)) * 0.05f;

}

void Window::displayCallback(GLFWwindow* window)
{
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawRain();
	rollerCoaster->draw(glm::mat4(1.0f));
	// Skybox
	//skyBox->draw(skyShader, cubeMapTexture);

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

//get cursor position
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
		case GLFW_KEY_P: // Pause
			SoundEngine->play2D("audio/faucet.mp3", GL_FALSE);
			break;
		default:
			break;
		}
	}
}

unsigned int Window::loadCubeMap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
