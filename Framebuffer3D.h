#pragma once

#include <4dm.h>
using namespace fdm;

// works like fdm::Framebuffer but placed in 3D space
class Framebuffer3D
{
public:
	const Shader* shader = nullptr;
	uint32_t FBO = 0;
	uint32_t tex2D = 0;
	uint32_t depthTex2D = 0;

	uint32_t VAO = 0, VBO = 0;

	Framebuffer3D() { }
	Framebuffer3D(const Framebuffer3D& other)
	{
		this->shader = other.shader;
		this->FBO = other.FBO;
		this->tex2D = other.tex2D;
		this->depthTex2D = other.depthTex2D;
		this->VAO = other.VAO;
		this->VBO = other.VBO;
	}
	Framebuffer3D(Framebuffer3D&& other) noexcept
	{
		this->shader = other.shader;
		this->FBO = other.FBO;
		this->tex2D = other.tex2D;
		this->depthTex2D = other.depthTex2D;
		this->VAO = other.VAO;
		this->VBO = other.VBO;
	}
	Framebuffer3D& operator=(const Framebuffer3D& other)
	{
		this->shader = other.shader;
		this->FBO = other.FBO;
		this->tex2D = other.tex2D;
		this->depthTex2D = other.depthTex2D;
		this->VAO = other.VAO;
		this->VBO = other.VBO;

		return *this;
	}
	Framebuffer3D& operator=(Framebuffer3D&& other) noexcept
	{
		this->shader = other.shader;
		this->FBO = other.FBO;
		this->tex2D = other.tex2D;
		this->depthTex2D = other.depthTex2D;
		this->VAO = other.VAO;
		this->VBO = other.VBO;

		other.shader = nullptr;
		other.FBO = 0;
		other.tex2D = 0;
		other.depthTex2D = 0;
		other.VAO = 0;
		other.VBO = 0;
		
		return *this;
	}
	const Shader* getShader() { return shader; }
	void setShader(const Shader* shader) { this->shader = shader; }
	void render();
	void init(GLsizei width, GLsizei height, bool alphaChannel = true);
	Framebuffer3D(GLsizei width, GLsizei height, bool alphaChannel = true) { init(width, height, alphaChannel); }
	void cleanup();
	void use() const { glBindFramebuffer(GL_FRAMEBUFFER, FBO); }
	~Framebuffer3D() { cleanup(); }
};
