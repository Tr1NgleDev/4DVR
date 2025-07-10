#include "FontRenderer3D.h"

using namespace fdm;

FontRenderer3D::FontRenderer3D(const fdm::Tex2D* texture, const fdm::Shader* shader)
	: texture(texture), shader(shader)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &charVBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, charVBO);

	glBufferData(GL_ARRAY_BUFFER, this->text.size(), this->text.data(), GL_STATIC_DRAW);
	glVertexAttribIPointer(0, 1, GL_BYTE, 1, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
FontRenderer3D::FontRenderer3D(FontRenderer3D&& other) noexcept
{
	text = other.text;
	charSize = other.charSize;
	fontSize = other.fontSize;
	pos = other.pos;
	color = other.color;
	model = other.model;
	centered = other.centered;
	texture = other.texture;
	shader = other.shader;
	VAO = other.VAO;
	charVBO = other.charVBO;
	scale = other.scale;

	other.text = "";
	other.charSize = { 8, 8 };
	other.fontSize = 1;
	other.pos = glm::vec3{ 0 };
	other.color = glm::vec4{ 1 };
	other.model = glm::mat4{ 1 };
	other.centered = false;
	other.texture = nullptr;
	other.shader = nullptr;
	other.VAO = 0;
	other.charVBO = 0;
	other.scale = 1;
}
FontRenderer3D& FontRenderer3D::operator=(FontRenderer3D&& other) noexcept
{
	text = other.text;
	charSize = other.charSize;
	fontSize = other.fontSize;
	pos = other.pos;
	color = other.color;
	model = other.model;
	centered = other.centered;
	texture = other.texture;
	shader = other.shader;
	VAO = other.VAO;
	charVBO = other.charVBO;
	scale = other.scale;

	other.text = "";
	other.charSize = { 8, 8 };
	other.fontSize = 1;
	other.pos = glm::vec3{ 0 };
	other.color = glm::vec4{ 1 };
	other.model = glm::mat4{ 1 };
	other.centered = false;
	other.texture = nullptr;
	other.shader = nullptr;
	other.VAO = 0;
	other.charVBO = 0;
	other.scale = 1;

	return *this;
}
void FontRenderer3D::setText(const fdm::stl::string& text)
{
	if (this->text == text)
		return;

	this->text = text;

	if (!VAO || !charVBO)
	{
		return;
	}

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, charVBO);

	glBufferData(GL_ARRAY_BUFFER, this->text.size(), this->text.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	updateModel();
}
void FontRenderer3D::setPos(const glm::vec3& pos)
{
	this->pos = pos;
}
void FontRenderer3D::setColor(const glm::vec4& color)
{
	this->color = color;
}
void FontRenderer3D::setScale(float scale)
{
	this->scale = scale;
}
void FontRenderer3D::setModel(const glm::mat4& m)
{
	model = m;
}
const glm::mat4& FontRenderer3D::getModel() const
{
	return model;
}
void FontRenderer3D::updateModel()
{
	glm::mat4 m{ 1 };
	glm::vec3 p = pos;

	if (centered)
	{
		p -= glm::vec3(getSize() * 0.5f, 0.0f);
	}

	m = glm::translate(m, p);

	setModel(m);
}
glm::vec2 FontRenderer3D::getSize() const
{
	return glm::vec2
	{
		fontSize * text.size() * charSize.x * scale,
		fontSize * charSize.y * scale
	};
}
void FontRenderer3D::render(const glm::mat4& view)
{
	texture->use();
	shader->use();

	glUniform4fv(glGetUniformLocation(shader->id(), "inColor"), 1, &color[0]);
	glUniform2f(glGetUniformLocation(shader->id(), "charSize"), charSize.x * fontSize * scale, charSize.y * fontSize * scale);
	glUniformMatrix4fv(glGetUniformLocation(shader->id(), "model"), 1, false, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->id(), "view"), 1, false, &view[0][0]);

	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, text.size());
	glBindVertexArray(0);
}
FontRenderer3D::~FontRenderer3D()
{
	if (VAO)
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &charVBO);
		VAO = 0;
		charVBO = 0;
	}
}
