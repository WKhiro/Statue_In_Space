#include "Geometry.h"
#include "Window.h"

Geometry::Geometry(std::string objFilename)
{
	// Parse the spheres
	parse(objFilename);

	glGenVertexArrays(1, &geometryVAO);

	glGenBuffers(1, &geometryVBO);
	glGenBuffers(1, &geometryVBO2);
	glGenBuffers(1, &geometryVBO3);
	glGenBuffers(1, &geometryEBO);

	glBindVertexArray(geometryVAO);

	glBindBuffer(GL_ARRAY_BUFFER, geometryVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, geometryVBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*normals.size(), normals.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, geometryVBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * textures.size(), textures.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(GLfloat), (GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometryEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Geometry::~Geometry()
{
	glDeleteVertexArrays(1, &geometryVAO);
	glDeleteBuffers(1, &geometryVBO);
	glDeleteBuffers(1, &geometryVBO2);
	glDeleteBuffers(1, &geometryEBO);
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
			if (label == "vt")
			{
				GLfloat textureX, textureY;
				ss >> textureX >> textureY;

				// Process the textures.
				textures.push_back(textureX);
				textures.push_back(textureY);
			}
			if (label == "f") // Only used for sphere.obj; change format if needed
			{
				unsigned int v_faceX, v_faceY, v_faceZ;
				unsigned int vn_faceX, vn_faceY, vn_faceZ;
				char ignore; // ignores "/"
				int ignore2; // ignores the value itself
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
}

void Geometry::draw(glm::mat4 model, GLuint shader)
{
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);
	glBindVertexArray(geometryVAO);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Geometry::update() {}