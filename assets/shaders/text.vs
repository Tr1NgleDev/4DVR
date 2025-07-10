#version 330 core

layout (location = 0) in int ch;

out int gsChars;

void main()
{
    gsChars = ch;
    // this vertex shader is pretty much useless
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
}