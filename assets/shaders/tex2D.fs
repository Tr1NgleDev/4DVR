#version 330 core

in vec2 uv;
out vec4 color;

uniform sampler2D image;

void main()
{    
    color = texture(image, uv);
}