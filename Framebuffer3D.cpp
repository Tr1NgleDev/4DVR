#include "Framebuffer3D.h"

void Framebuffer3D::render()
{
	shader->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex2D);
	glUniform1i(glGetUniformLocation(shader->id(), "image"), 0);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void Framebuffer3D::init(GLsizei width, GLsizei height, bool alphaChannel)
{
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &tex2D);
	glBindTexture(GL_TEXTURE_2D, tex2D);
	glTexImage2D(GL_TEXTURE_2D, 0, alphaChannel ? GL_RGBA : GL_RGB, width, height, 0, alphaChannel ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex2D, 0);

	glGenTextures(1, &depthTex2D);
	glBindTexture(GL_TEXTURE_2D, depthTex2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shader = ShaderManager::get("4dvr_tex2Din3DShader");

	uint8_t verts[8]
	{
		0,0,
		1,0,
		1,1,
		0,1,
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_UNSIGNED_BYTE, GL_FALSE, 2 * sizeof(uint8_t), nullptr); // pos and uv
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Framebuffer3D::cleanup()
{
	if (FBO)
	{
		glDeleteTextures(1, &tex2D);
		glDeleteTextures(1, &depthTex2D);
		glDeleteFramebuffers(1, &FBO);

		FBO = 0;
		tex2D = 0;
		depthTex2D = 0;

		glBindVertexArray(VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
		VBO = 0;
	}
}
