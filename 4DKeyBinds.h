#pragma once

#include <4dm.h>
#include <string>
#include <type_traits>
#include <windows.h>

enum KeyBindsScope
{
	GLOBAL,
	PLAYER,
	STATEGAME,
	STATETITLESCREEN,
	TEXTINPUT
};

namespace glfw
{
	enum Keys
	{
		Unknown = -1,
		Space = 32,
		Apostrophe = 39,
		Comma = 44,
		Minus = 45,
		Period = 46,
		Slash = 47,
		Alpha0 = 48,
		Alpha1 = 49,
		Alpha2 = 50,
		Alpha3 = 51,
		Alpha4 = 52,
		Alpha5 = 53,
		Alpha6 = 54,
		Alpha7 = 55,
		Alpha8 = 56,
		Alpha9 = 57,
		SemiColon = 59,
		Equal = 61,
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,
		LeftBracket = 91,
		Backslash = 92,
		RightBracket = 93,
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		Numpad0 = 320,
		Numpad1 = 321,
		Numpad2 = 322,
		Numpad3 = 323,
		Numpad4 = 324,
		Numpad5 = 325,
		Numpad6 = 326,
		Numpad7 = 327,
		Numpad8 = 328,
		Numpad9 = 329,
		NumpadDecimal = 330,
		NumpadDivide = 331,
		NumpadMultiply = 332,
		NumpadSubtract = 333,
		NumpadAdd = 334,
		NumpadEnter = 335,
		NumpadEqual = 336,
		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	};
}

namespace KeyBinds
{
	using BindCallback = std::add_pointer<void(GLFWwindow* window, int action, int mods)>::type;
	inline const fdm::stl::string id = "tr1ngledev.4dkeybinds";

	inline bool isLoaded()
	{
		return fdm::isModLoaded(id);
	}

	// creates a bind with name "bindNamespace:bindName"
	inline void addBind(const fdm::stl::string& bindNamespace, const fdm::stl::string& bindName, glfw::Keys defaultKey, KeyBindsScope scope, BindCallback callback)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(const fdm::stl::string&, int, int, BindCallback)>(fdm::getModFuncPointer(id, "addBind"))
			(std::format("{}:{}", bindNamespace, bindName), (int)defaultKey, (int)scope, callback);
	}

	inline void hookBind(const fdm::stl::string& bindNamespace, const fdm::stl::string& bindName, KeyBindsScope scope, BindCallback callback)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(const fdm::stl::string&, KeyBindsScope, BindCallback)>(fdm::getModFuncPointer(id, "hookBind"))
			(std::format("{}:{}", bindNamespace, bindName), scope, callback);
	}

	inline void triggerBind(const fdm::stl::string& bindNamespace, const fdm::stl::string& bindName, KeyBindsScope scope, int action, int mods)
	{
		if (!isLoaded())
			return;
		reinterpret_cast<void(__stdcall*)(const fdm::stl::string&, KeyBindsScope, int, int)>(fdm::getModFuncPointer(id, "triggerBind"))
			(std::format("{}:{}", bindNamespace, bindName), scope, action, mods);
	}

	inline std::vector<std::pair<fdm::stl::string, fdm::stl::string>> getBinds(KeyBindsScope scope)
	{
		if (!isLoaded())
			return {};
		std::vector<std::pair<fdm::stl::string, fdm::stl::string>> result{};
		reinterpret_cast<void(__stdcall*)(KeyBindsScope, std::vector<std::pair<fdm::stl::string, fdm::stl::string>>*)>(fdm::getModFuncPointer(id, "getBinds"))
			(scope, &result);
		return result;
	}
}
