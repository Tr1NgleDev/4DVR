#pragma once

#include <4dm.h>
using namespace fdm;

#include <openvr.h>

class VRTex
{
public:
	uint32_t fbo = NULL;
	uint32_t tex = NULL, texDepth = NULL;

	~VRTex();
	void init(int width, int height);
	void drawEye(vr::EVREye eye);
	void use();
};