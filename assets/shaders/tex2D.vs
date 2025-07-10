#version 330 core

layout (location = 0) in vec2 vertex;

out vec2 uv;

uniform mat4 P;
uniform mat4 MV;

void main()
{
    uv = vertex;
    gl_Position = P * MV * vec4(vertex, 0.0, 1.0);
}