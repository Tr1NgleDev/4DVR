#pragma once

#include <4dm.h>
#include <openvr.h>
using namespace fdm;
using namespace vr;

inline static glm::mat4 ToMat4(const HmdMatrix44_t& v)
{
	glm::mat4 result{};

	result[0][0] = v.m[0][0];
	result[0][1] = v.m[0][1];
	result[0][2] = v.m[0][2];
	result[0][3] = v.m[0][3];

	result[1][0] = v.m[1][0];
	result[1][1] = v.m[1][1];
	result[1][2] = v.m[1][2];
	result[1][3] = v.m[1][3];

	result[2][0] = v.m[2][0];
	result[2][1] = v.m[2][1];
	result[2][2] = v.m[2][2];
	result[2][3] = v.m[2][3];

	result[3][0] = v.m[3][0];
	result[3][1] = v.m[3][1];
	result[3][2] = v.m[3][2];
	result[3][3] = v.m[3][3];

	return result;
}
inline static glm::mat4 ToMat4(const fdm::m4::Mat5 & v)
{
	glm::mat4 result{};

	result[0][0] = v[0][0];
	result[0][1] = v[0][1];
	result[0][2] = v[0][2];
	result[0][3] = 0.f;

	result[1][0] = v[1][0];
	result[1][1] = v[1][1];
	result[1][2] = v[1][2];
	result[1][3] = 0.f;

	result[2][0] = v[2][0];
	result[2][1] = v[2][1];
	result[2][2] = v[2][2];
	result[2][3] = 0.f;

	result[3][0] = v[4][0];
	result[3][1] = v[4][1];
	result[3][2] = v[4][2];
	result[3][3] = 1.f;

	return result;
}
inline static glm::mat4 ToMat34(const HmdMatrix34_t& v)
{
	glm::mat4 result{};

	result[0][0] = v.m[0][0];
	result[0][1] = v.m[0][1];
	result[0][2] = v.m[0][2];
	result[0][3] = v.m[0][3];

	result[1][0] = v.m[1][0];
	result[1][1] = v.m[1][1];
	result[1][2] = v.m[1][2];
	result[1][3] = v.m[1][3];

	result[2][0] = v.m[2][0];
	result[2][1] = v.m[2][1];
	result[2][2] = v.m[2][2];
	result[2][3] = v.m[2][3];

	result[3][0] = 0.f;
	result[3][1] = 0.f;
	result[3][2] = 0.f;
	result[3][3] = 1.f;

	return result;
}

inline static m4::Mat5 ToMat5(const glm::mat4& m, bool i = false)
{
	m4::Mat5 result{ 1 };

	if (!i)
	{
		result[0][0] = m[0][0];
		result[0][1] = m[0][1];
		result[0][2] = m[0][2];
		result[0][3] = m[0][3];

		result[1][0] = m[1][0];
		result[1][1] = m[1][1];
		result[1][2] = m[1][2];
		result[1][3] = m[1][3];

		result[2][0] = m[2][0];
		result[2][1] = m[2][1];
		result[2][2] = m[2][2];
		result[2][3] = m[2][3];

		result[4][0] = m[3][0];
		result[4][1] = m[3][1];
		result[4][2] = m[3][2];
		//result[4][3] = m[3][3];
	}
	else
	{
		result[0][0] = m[0][0];
		result[0][1] = m[1][0];
		result[0][2] = m[2][0];
		result[0][3] = m[3][0];

		result[1][0] = m[0][1];
		result[1][1] = m[1][1];
		result[1][2] = m[2][1];
		result[1][3] = m[3][1];

		result[2][0] = m[0][2];
		result[2][1] = m[1][2];
		result[2][2] = m[2][2];
		result[2][3] = m[3][2];

		result[4][0] = m[0][3];
		result[4][1] = m[1][3];
		result[4][2] = m[2][3];
		//result[4][3] = m[3][3];
	}

	return result;
}