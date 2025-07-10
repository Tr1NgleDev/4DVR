#version 330 core

in vec2 fsUV;
in vec2 fsQuadUV;

out vec4 color;

uniform sampler2D texSampler;
uniform vec4 inColor;

void main()
{
	color = texture(texSampler, fsUV) * inColor;
	//if (fsQuadUV.x <= 0.01 || fsQuadUV.x >= 1.0 - 0.01 || fsQuadUV.y <= 0.01 || fsQuadUV.y >= 1.0 - 0.01) color = vec4(fsQuadUV, 0.0, 1.0);
	if (color.a <= 0.0001)
		discard;
}