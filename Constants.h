#pragma once

#ifdef NONVR
#define VR_MODE "NON-VR"
#define VR_MODE_OTHER "VR"
#define wVR_MODE L"NON-VR"
#else
#define VR_MODE "VR"
#define VR_MODE_OTHER "NON-VR"
#define wVR_MODE L"VR"
#endif

#include <fstream>

inline static constexpr float ITEM_SCALE = 0.2;
inline static constexpr int WIDTH_UI = 1280;
inline static constexpr int HEIGHT_UI = 720;

using Mat5 = m4::Mat5;

inline static constexpr const char* controllerComponentNames[2]{ "tip", "handgrip" };

inline static constexpr int punchFrames = 10;
inline static constexpr float punchAccelerationThreshold = 16.f;
inline static constexpr int punchFalsePositivesMax = 12;
inline static constexpr double punchTimeout = 0.3f;

inline static Mat5 rotorX = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() * 0.5f });
inline static Mat5 rotorY = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() * 0.5f });
inline static Mat5 rotorZ = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 1, 0, 0 }), glm::pi<float>() * 0.5f });
inline static Mat5 rotorX180 = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() });
inline static Mat5 rotorY180 = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() });
inline static Mat5 rotorZ180 = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 1, 0, 0 }), glm::pi<float>() });
inline static Mat5 rotorX45 = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() * 0.25f });
inline static Mat5 rotorY45 = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() * 0.25f });
inline static Mat5 rotorZ45 = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 1, 0, 0 }), glm::pi<float>() * 0.25f });
inline static Mat5 rotorX22p5 = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() * 0.125f });
inline static Mat5 rotorY22p5 = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() * 0.125f });
inline static Mat5 rotorZ22p5 = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 1, 0, 0 }), glm::pi<float>() * 0.125f });

inline static m4::BiVector4 wedgeN(const glm::vec4& u, const glm::vec4& v)
{
	return m4::wedge(v, u);
}

inline static Mat5 rotorXN = Mat5(m4::Rotor{ wedgeN(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() * 0.5f });
inline static Mat5 rotorYN = Mat5(m4::Rotor{ wedgeN(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() * 0.5f });
inline static Mat5 rotorZN = Mat5(m4::Rotor{ wedgeN(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 1, 0, 0 }), glm::pi<float>() * 0.5f });
inline static Mat5 rotorX180N = Mat5(m4::Rotor{ wedgeN(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() });
inline static Mat5 rotorY180N = Mat5(m4::Rotor{ wedgeN(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() });
inline static Mat5 rotorZ180N = Mat5(m4::Rotor{ wedgeN(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 1, 0, 0 }), glm::pi<float>() });
inline static Mat5 rotorX45N = Mat5(m4::Rotor{ wedgeN(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() * 0.25f });
inline static Mat5 rotorY45N = Mat5(m4::Rotor{ wedgeN(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() * 0.25f });
inline static Mat5 rotorZ45N = Mat5(m4::Rotor{ wedgeN(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 1, 0, 0 }), glm::pi<float>() * 0.25f });
inline static Mat5 rotorX22p5N = Mat5(m4::Rotor{ wedgeN(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() * 0.125f });
inline static Mat5 rotorY22p5N = Mat5(m4::Rotor{ wedgeN(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::pi<float>() * 0.125f });
inline static Mat5 rotorZ22p5N = Mat5(m4::Rotor{ wedgeN(glm::vec4{ 1, 0, 0, 0 }, glm::vec4{ 0, 1, 0, 0 }), glm::pi<float>() * 0.125f });
