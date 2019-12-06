#include "SceneGraph.h"
#include "Window.h"

Group::Group() 
{
	Group::M = glm::mat4(1.0f);
}

void Group::draw(glm::mat4 C) 
{
	for (int i = 0; i < children.size(); i++) 
	{
		Group::children[i]->draw(C*M);
	}
}

void Group::addChild(Node* child) 
{
	Group::children.push_back(child);
}

Group::~Group() {}
void Group::update() {}
void Group::removeChild(Node* child) {}


Transform::Transform(glm::mat4 C) 
{
	Transform::M = C;
}

void Transform::draw(glm::mat4 C) 
{
	for (int i = 0; i < children.size(); i++) 
	{
		Transform::children[i]->draw(C * Transform::M);
	}
}

void Transform::addChild(Node* child) 
{
	Transform::children.push_back(child);
}

Transform::~Transform() {}
void Transform::update() {}
void Transform::removeChild(Node* child) {}


Geometry::Geometry(std::string objFilename, GLuint program, GLuint environment, int color)
{
	environmentMap = 0;

	// Interpolation point
	if (color == 0) 
	{
		diffuse = { 0.5f, 0.0f, 0.0f };
	}
	// Approximating point
	else if (color == 1) 
	{
		diffuse = { 0.0f, 0.0f, 0.5f };
	} 
	// Roller Coaster sphere
	else 
	{
		diffuse = { 0.5f, 0.5f, 0.5f };
	}

	specular = { 0.5f, 0.5f, 0.5f };
	ambient = { 0.0f, 0.0f, 0.0f };
	shininess = 0.5f;
	
	initial = glm::mat4(1.0f);
	center = { 0.0f,0.0f,0.0f };

	// Parse the spheres
	parse(objFilename);

	shader = program;
	environmentShader = environment;

	glGenVertexArrays(1, &sphereVAO);

	glGenBuffers(1, &sphereVBO);
	glGenBuffers(1, &sphereVBO2);
	glGenBuffers(1, &sphereEBO);

	glBindVertexArray(sphereVAO);

	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*normals.size(), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Geometry::~Geometry()
{
	glDeleteVertexArrays(1, &sphereVAO);
	glDeleteBuffers(1, &sphereVBO);
	glDeleteBuffers(1, &sphereVBO2);
	glDeleteBuffers(1, &sphereEBO);
}

void Geometry::parse(std::string objFilename)
{
	// Max and min dimension trackers
	GLfloat maxX = 0.0f, maxY = 0.0f, maxZ = 0.0f;
	GLfloat minX = 0.0f, minY = 0.0f, minZ = 0.0f;

	std::ifstream objFile(objFilename); // The obj file we are reading.

	// Check whether the file can be opened.
	if (objFile.is_open())
	{
		std::string line; // A line in the file.

		// Read lines from the file.
		while (std::getline(objFile, line))
		{
			// Turn the line into a string stream for processing.
			std::stringstream ss;
			ss << line;

			// Read the first word of the line.
			std::string label;
			ss >> label;

			// If the line is about vertex (starting with a "v").
			if (label == "v")
			{
				// Read the later three float numbers and use them as the 
				// coordinates.
				GLfloat pointX, pointY, pointZ;

				ss >> pointX >> pointY >> pointZ;

				// Get the max and min coordinates for all dimensions
				if (pointX > maxX) { maxX = pointX; }
				if (pointX < minX) { minX = pointX; }
				if (pointY > maxY) { maxY = pointY; }
				if (pointY < minY) { minY = pointY; }
				if (pointZ > maxZ) { maxZ = pointZ; }
				if (pointZ < minZ) { minZ = pointZ; }

				// Process the vertices.
				vertices.push_back(pointX);
				vertices.push_back(pointY);
				vertices.push_back(pointZ);
			}
			if (label == "vn")
			{
				GLfloat normalX, normalY, normalZ;
				ss >> normalX >> normalY >> normalZ;

				// Process the normals.
				normals.push_back(normalX);
				normals.push_back(normalY);
				normals.push_back(normalZ);
			}
			if (label == "f") // Only used for sphere.obj; change format if needed
			{
				unsigned int v_faceX, v_faceY, v_faceZ;
				unsigned int vn_faceX, vn_faceY, vn_faceZ;
				char ignore;
				unsigned ignore2;
				ss >> v_faceX >> ignore >> ignore2 >> ignore >> vn_faceX;
				ss >> v_faceY >> ignore >> ignore2 >> ignore >> vn_faceY;
				ss >> v_faceZ >> ignore >> ignore2 >> ignore >> vn_faceZ;
				indices.push_back(v_faceX - 1);
				indices.push_back(v_faceY - 1);
				indices.push_back(v_faceZ - 1);
			}
		}
	}
	else
	{
		std::cerr << "Can't open the file " << objFilename << std::endl;
	}
	objFile.close();

	GLfloat maxDistance = maxX - minX;

	if ((maxY - minY) > maxDistance) 
	{
		maxDistance = maxY - minY;
	}

	if ((maxZ - minZ) > maxDistance) 
	{
		maxDistance = maxZ - minZ;
	}

	initial = glm::mat4(1.0f);//glm::scale(glm::mat4(1.0f), glm::vec3((2.0f/ maxDistance), (2.0f / maxDistance), (2.0f/ maxDistance)));
}

void Geometry::draw(glm::mat4 C)
{
	glm::mat4 modelview = Window::view * C * initial;

	if (environmentMap) // Roller Coaster
	{
		glUseProgram(environmentShader);
		glm::mat4 currentModel = initial;

		projectionLoc = glGetUniformLocation(environmentShader, "projection");
		viewLoc = glGetUniformLocation(environmentShader, "view");
		modelLoc = glGetUniformLocation(environmentShader, "model");

		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &Window::projection[0][0]);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &Window::view[0][0]);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &currentModel[0][0]);
		glUniform3fv(glGetUniformLocation(environmentShader, "cameraPos"), 1, &(Window::currentPos[0]));

		glBindVertexArray(sphereVAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	else // Points
	{
		glUseProgram(shader);

		projectionLoc = glGetUniformLocation(shader, "projection");
		modelViewLoc = glGetUniformLocation(shader, "modelview");

		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &Window::projection[0][0]);
		glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, &modelview[0][0]);

		glUniform3fv(glGetUniformLocation(shader, "diffuseVal"), 1, &(diffuse[0]));
		glUniform3fv(glGetUniformLocation(shader, "specularVal"), 1, &(specular[0]));
		glUniform3fv(glGetUniformLocation(shader, "ambientVal"), 1, &(ambient[0]));
		glUniform1f(glGetUniformLocation(shader, "shininessVal"), shininess);
		glUniform1i(glGetUniformLocation(shader, "light.normalColoring"), Window::normalColoring);
		glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, &Window::view[0][0]);

		glBindVertexArray(sphereVAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

}

void Geometry::update() {}
void Geometry::addChild(Node* child) {}
void Geometry::removeChild(Node* child) {}
