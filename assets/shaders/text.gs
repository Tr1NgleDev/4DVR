#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;

in int gsChars[];

out vec2 fsUV;
out vec2 fsQuadUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 P;
uniform vec2 charSize;

void main()
{
	float charOffsetX = float(gl_PrimitiveIDIn * charSize.x);

	vec2 charStartPos = vec2(charOffsetX, 0.0);
	vec2 charEndPos = vec2(charOffsetX + charSize.x, charSize.y);

	vec2 charStartUV = vec2(float(gsChars[0]) / 128.0, 1.0);
	vec2 charEndUV = vec2(float(gsChars[0] + 1) / 128.0, 0.0);

	mat4 MVP = P * view * model;

	// create the triangles
	fsUV = vec2(charStartUV.x, charStartUV.y);
	fsQuadUV = vec2(0.0, 0.0);
	gl_Position = MVP * vec4(charStartPos.x, charStartPos.y, 0.0, 1.0);
	EmitVertex();

	fsUV = vec2(charEndUV.x, charStartUV.y);
	fsQuadUV = vec2(1.0, 0.0);
	gl_Position = MVP * vec4(charEndPos.x, charStartPos.y, 0.0, 1.0);
	EmitVertex();

	fsUV = vec2(charStartUV.x, charEndUV.y);
	fsQuadUV = vec2(0.0, 1.0);
	gl_Position = MVP * vec4(charStartPos.x, charEndPos.y, 0.0, 1.0);
	EmitVertex();

	fsUV = vec2(charEndUV.x, charStartUV.y);
	fsQuadUV = vec2(1.0, 0.0);
	gl_Position = MVP * vec4(charEndPos.x, charStartPos.y, 0.0, 1.0);
	EmitVertex();

	fsUV = vec2(charStartUV.x, charEndUV.y);
	fsQuadUV = vec2(0.0, 1.0);
	gl_Position = MVP * vec4(charStartPos.x, charEndPos.y, 0.0, 1.0);
	EmitVertex();

	fsUV = charEndUV;
	fsQuadUV = vec2(1.0, 1.0);
	gl_Position = MVP * vec4(charEndPos.x, charEndPos.y, 0.0, 1.0);
	EmitVertex();

	EndPrimitive();
}