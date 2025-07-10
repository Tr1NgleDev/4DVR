#pragma once

#include <4dm.h>

class FontRenderer3D
{
private:
	fdm::stl::string text = "";
	glm::mat4 model{ 1 };
	const fdm::Tex2D* texture = nullptr;
	const fdm::Shader* shader = nullptr;
	uint32_t VAO = 0;
	uint32_t charVBO = 0;

public:
	glm::ivec2 charSize{ 8, 8 };
	int fontSize = 1;
	glm::vec3 pos{ 0 };
	float scale = 1.0f / 16.0f;
	glm::vec4 color{ 1 };
	bool centered = false;

	FontRenderer3D() {}
	FontRenderer3D(const FontRenderer3D& other) = delete;
	FontRenderer3D& operator=(const FontRenderer3D& other) = delete;
	FontRenderer3D(const fdm::Tex2D* texture, const fdm::Shader* shader);
	FontRenderer3D(FontRenderer3D&& other) noexcept;
	FontRenderer3D& operator=(FontRenderer3D&& other) noexcept;
	void setText(const fdm::stl::string& text);
	void setPos(const glm::vec3& pos);
	void setColor(const glm::vec4& color);
	void setScale(float scale);
	void setModel(const glm::mat4& m);
	const glm::mat4& getModel() const;
	void updateModel();
	void render(const glm::mat4& view);
	glm::vec2 getSize() const;
	~FontRenderer3D();
};
