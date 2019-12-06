#include "SkyBox.h"
#include "Window.h"

SkyBox::SkyBox()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyBoxVertices), &skyBoxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

SkyBox::~SkyBox()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void SkyBox::draw(GLuint skyShader, unsigned int cubemapTexture)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	glUseProgram(skyShader);

	GLint projectLoc = glGetUniformLocation(skyShader, "projection");
	GLint viewLoc = glGetUniformLocation(skyShader, "view");
	GLint skybox = glGetUniformLocation(skyShader, "skybox");

	glUniformMatrix4fv(projectLoc, 1, GL_FALSE, &Window::projection[0][0]);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &Window::view[0][0]);
	glUniform1i(skybox, 0);	

	glBindVertexArray(VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}