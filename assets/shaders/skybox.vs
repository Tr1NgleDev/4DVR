#version 330 core

layout(location = 0) in vec4 vert;
layout(location = 1) in vec2 uv;

out vec2 fsUV;

uniform mat4 MVP; // unused
uniform mat4 MV;
uniform mat4 P;

void main()
{
	fsUV = uv;

	gl_Position = P * MV * vert;
}