#include <windows.h>
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <commctrl.h>

#include <4dm.h>
using namespace fdm; // 4dm.h

#include <openvr.h>
using namespace vr;

#include "VRTex.h"

#include <glm/gtx/string_cast.hpp>

initDLL

#include "Constants.h"
#include "VR.h"

using namespace VRStuff;

#include "matConverts.h"
#include "Player.h"

#include "4DKeyBinds.h"

#ifndef NONVR
$hookStatic(Mat5, m4, createCamera, const glm::vec4& eye, const glm::vec4& forward, const glm::vec4& up, const glm::vec4& right, const glm::vec4& over)
{
	glm::vec4 eyeN = eye;

	if (curEye == VEye_Left)
		eyeN += !use3DOrientation ? eyeLP : eyeLP3D;
	else
		eyeN += !use3DOrientation ? eyeRP : eyeRP3D;

	lastEye = eyeN;

	//return lastCamera = original(eyeN, forward, up, right, over);
	if (!use3DOrientation)
	{
		if (keepXZCameraPosition)
			eyeN -= hmd4DPos;

		return lastCamera = original(eyeN, -*(glm::vec4*)orientation[2], *(glm::vec4*)orientation[1], -*(glm::vec4*)orientation[0], over);
	}
	return lastCamera = original(eyeN, -*(glm::vec4*)orientation3D[2], *(glm::vec4*)orientation3D[1], -*(glm::vec4*)orientation3D[0], over);
}

#endif

bool initializedAssets = false;

void changeMode(const std::string& modeI, bool showDialog = true)
{
	std::string mode = modeI;
	if (mode != "VR" && mode != "NON-VR")
		mode = "NON-VR";

	dumpConfig(true, mode);

	std::ifstream infoFile(infoPath);
	std::string s = "";
	if (infoFile.is_open())
	{
		std::ostringstream ss;
		ss << infoFile.rdbuf();
		s = ss.str();
		infoFile.close();
	}
	size_t modDLLPos = s.find("modDLL: \"");
	if (modDLLPos != std::string::npos)
	{
		modDLLPos += 9;
		size_t modDLLEnd = s.find_first_of('\"', modDLLPos);

		if (modDLLEnd != std::string::npos)
		{
			s.replace(modDLLPos, modDLLEnd - modDLLPos, modeDLLs.at(mode));

			std::ofstream infoFileO(infoPath);
			if (infoFileO.is_open())
				infoFileO << s;
			infoFileO.close();
		}
	}

	// restart
	Sleep(1000);
	system("\"./4D Miner.exe\"");
	exit(0);
	return;
}

std::wstring widen(const std::string& str)
{
	std::wostringstream wstm;
	const std::ctype<wchar_t>& ctfacet = std::use_facet<std::ctype<wchar_t>>(wstm.getloc());
	for (size_t i = 0; i < str.size(); ++i)
		wstm << ctfacet.widen(str[i]);
	return wstm.str();
}

$hook(void, StateIntro, init, StateManager& s)
{
	std::string modPath = fdm::getModPath(fdm::modID);
	if (modPath.empty())
		modPath = "./mods/4DVR/";
	std::filesystem::path modPathF = modPath;

	configPath = (modPathF / "config.json").string();
	infoPath = (modPathF / "info.json5").string();

	nlohmann::json configJson
	{
		{ "mode", "VR" },
		{ "heightRatio", heightRatio },
		{ "showDialog", true },
		{ "justRestarted", false },
		{ "wDepth", wOffset },
		{ "wEyeDistance", eyeWGap },
		{ "desktopView", desktopView },
	};

	if (!std::filesystem::exists(configPath))
	{
		std::ofstream configFileO(configPath);
		if (configFileO.is_open())
		{
			configFileO << configJson.dump(4);
			configFileO.close();
		}
	}

	std::ifstream configFileI(configPath);
	if (configFileI.is_open())
	{
		configJson = nlohmann::json::parse(configFileI);
		configFileI.close();
	}

	if (!configJson.contains("showDialog"))
		configJson["showDialog"] = false;
	if (!configJson.contains("mode"))
		configJson["mode"] = "NON-VR";
	if (!configJson.contains("heightRatio"))
		configJson["heightRatio"] = heightRatio;
	if (!configJson.contains("wDepth"))
		configJson["wDepth"] = wOffset;
	if (!configJson.contains("wEyeDistance"))
		configJson["wEyeDistance"] = eyeWGap;
	if (!configJson.contains("desktopView"))
		configJson["desktopView"] = desktopView;

	auto args = fdm::getLaunchArguments();
	bool dontDialog = false;
	if (std::find(args.cbegin(), args.cend(), "-vr") != args.cend())
	{
		configJson["mode"] = "VR";
		dontDialog = true;
	}
	else if (std::find(args.cbegin(), args.cend(), "-nonvr") != args.cend())
	{
		configJson["mode"] = "NON-VR";
		dontDialog = true;
	}

	if (!dontDialog && configJson["showDialog"].get<bool>() && (!configJson.contains("justRestarted") || !configJson["justRestarted"].get<bool>()))
	{
		std::string pszContentStr = std::format("Which mode do you want to run 4DVR in?\n(You can turn off this dialog and choose the mode in the settings menu or in \"{}config.json\")", modPath);
		std::wstring pszContentWStr = widen(pszContentStr);
		int btnPressed = 0;
		const TASKDIALOG_BUTTON buttons[] = { {IDYES, L"VR"}, {IDNO, L"NON-VR"} };
		TASKDIALOGCONFIG cfg{ 0 };
		cfg.cbSize = sizeof(cfg);
		cfg.hInstance = HINST_COMMCTRL;
		cfg.pszWindowTitle = L"4DVR Mode Switcher (current: " wVR_MODE L")";
		cfg.pszContent = pszContentWStr.c_str();
		cfg.pszMainIcon = TD_INFORMATION_ICON;
		cfg.cButtons = ARRAYSIZE(buttons);
		cfg.pButtons = buttons;
		TaskDialogIndirect(&cfg, &btnPressed, NULL, NULL);

		// switch the mode and restart

		if (btnPressed == IDYES && VR_MODE == "NON-VR") // VR
		{
			configJson["mode"] = "VR";
		}
		else if (btnPressed == IDNO && VR_MODE == "VR") // NON-VR
		{
			configJson["mode"] = "NON-VR";
		}
	}

	if (configJson["mode"] != VR_MODE)
	{
		changeMode(configJson["mode"], configJson["showDialog"]);
	}

	configJson["justRestarted"] = false;
	std::ofstream configFileO(configPath);
	if (configFileO.is_open())
	{
		configFileO << configJson.dump(4);
		configFileO.close();
	}

	showDialogSetting = configJson["showDialog"];
	heightRatio = configJson["heightRatio"];
	wOffset = configJson["wDepth"];
	eyeWGap = configJson["wEyeDistance"];
	desktopView = configJson["desktopView"];

	original(self, s);

	stateManager = &s;

	// initialize opengl stuff
	glewExperimental = true;
	glewInit();
	glfwInit();

#ifndef NONVR
	bool initialized = initVR();
	Console::printLine("4DVR: ", Console::Mode(Console::GREEN, Console::BRIGHT), "initVR() Status:", Console::Mode(Console::YELLOW, Console::BRIGHT), initialized ? "true" : "false");

	StateSettings::instanceObj.setFullscreenMode(s.window, false);

	glfwSwapInterval(0);
	glfwSetWindowSize(s.window, WIDTH_UI, HEIGHT_UI);
	//glfwSetWindowAttrib(s.window, GLFW_RESIZABLE, false);
	glfwSetWindowAspectRatio(s.window, 16, 9);

	//StateSettings::instanceObj->fullscreenButton.clickable = false;
#endif
	if (!initializedAssets)
	{
		MeshBuilder mesh{ BlockInfo::HYPERCUBE_FULL_INDEX_COUNT };

		mesh.addBuff(BlockInfo::hypercube_full_verts, sizeof(BlockInfo::hypercube_full_verts));
		mesh.addAttr(GL_UNSIGNED_BYTE, 4, sizeof(glm::u8vec4));

		mesh.addBuff(BlockInfo::hypercube_full_normals, sizeof(BlockInfo::hypercube_full_normals));
		mesh.addAttr(GL_FLOAT, 1, sizeof(GLfloat));

		mesh.setIndexBuff(BlockInfo::hypercube_full_indices, sizeof(BlockInfo::hypercube_full_indices));

		handRenderer = MeshRenderer();
		handRenderer.setMesh(&mesh);
		handShader = ShaderManager::get("tetSolidColorNormalShader");

		crosshairRenderer = MeshRenderer();
		crosshairRenderer.setMesh(&GlobalShapes::hypercube);
		crosshairShader = ShaderManager::get("tetShader");

		initializedAssets = true;

		ShaderManager::load("4dvr_tex2Din3DShader", "assets/shaders/tex2D.vs", "assets/shaders/tex2D.fs");
		ShaderManager::load("4dvr_textShader", "assets/shaders/text.vs", "assets/shaders/text.fs", "assets/shaders/text.gs");

#ifndef NONVR
		// replace the skybox shader
		const Shader* skyboxShader = ShaderManager::get("skyboxShader");
		glDeleteProgram(skyboxShader->id());
		ShaderManager::shaders.erase("skyboxShader");
		delete skyboxShader;

		ShaderManager::load("skyboxShader", "assets/shaders/skybox.vs", "../../assets/shaders/skybox.fs");
#endif
		// fix butterflies bruh
		const Shader* butterflyShader = ShaderManager::get("butterflyShader");
		butterflyShader->use();
		glUniform4f(glGetUniformLocation(butterflyShader->id(), "inColor"), 1, 1, 1, 0.5f);
		glUniform1i(glGetUniformLocation(butterflyShader->id(), "size"), 16);
	}
}

inline static int getY(gui::Element* element)
{
	// im comparing typeid name strings instead of using dynamic_cast because typeids of 4dminer and typeids of 4dm.h are different
	if (0 == strcmp(typeid(*element).name(), "class gui::Button"))
		return ((gui::Button*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::CheckBox"))
		return ((gui::CheckBox*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Image"))
		return ((gui::Image*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Slider"))
		return ((gui::Slider*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::Text"))
		return ((gui::Text*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class gui::TextInput"))
		return ((gui::TextInput*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Button"))
		return ((gui::Button*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::CheckBox"))
		return ((gui::CheckBox*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Image"))
		return ((gui::Image*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Slider"))
		return ((gui::Slider*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::Text"))
		return ((gui::Text*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class fdm::gui::TextInput"))
		return ((gui::TextInput*)element)->yOffset;
	else if (0 == strcmp(typeid(*element).name(), "class Dropdown"))
		return MBO(int, element, 224);
	return 0;
}

// i am a stupid ass so im gonna put the init in render(update)
static bool initializedSettings = false;
$hook(void, StateSettings, init, StateManager& s)
{
	initializedSettings = false;
	original(self, s);
}
$hook(void, StateSettings, render, StateManager& s)
{
	original(self, s);

	if (!initializedSettings)
	{
		int lowestY = 0;
		for (auto& e : self->mainContentBox.elements)
		{
			if (e == &self->secretButton) // skip the secret button
				continue;

			lowestY = std::max(getY(e), lowestY);
		}
		int oldLowest = lowestY;

		static gui::Text vrTitle{};
		vrTitle.setText("4DVR Options:");
		vrTitle.alignX(gui::ALIGN_CENTER_X);
		vrTitle.offsetY(lowestY += 100);
		vrTitle.size = 2;

		static gui::Button switchButton{};
		switchButton.setText("Switch to " VR_MODE_OTHER);
		switchButton.alignX(gui::ALIGN_CENTER_X);
		switchButton.offsetY(lowestY += 75);
		switchButton.width = 275;
		switchButton.height = 50;
		switchButton.callback = [](void* user)
			{
				changeMode(VR_MODE_OTHER, showDialogSetting);
			};

		static gui::CheckBox showDialogToggle{};
		showDialogToggle.setText("Show Switch Dialog");
		showDialogToggle.alignX(gui::ALIGN_CENTER_X);
		showDialogToggle.offsetY(lowestY += 100);
		showDialogToggle.checked = showDialogSetting;
		showDialogToggle.callback = [](void* user, bool checked)
			{
				showDialogSetting = checked;

				dumpConfig();
			};

#ifndef NONVR
		static gui::Button calibrateHeightButton{};
		calibrateHeightButton.setText(std::format("Calibrate Height ({}%)", (int)round(heightRatio * 100)));
		calibrateHeightButton.alignX(gui::ALIGN_CENTER_X);
		calibrateHeightButton.offsetY(lowestY += 75);
		calibrateHeightButton.width = 375;
		calibrateHeightButton.height = 50;
		calibrateHeightButton.user = &calibrateHeightButton;
		calibrateHeightButton.callback = [](void* user)
			{
				heightRatio = hmdPose[1][3] / Player::HEIGHT;

				dumpConfig();

				((gui::Button*)user)->setText(std::format("Calibrate Height ({}%)", (int)round(heightRatio * 100)));
			};

		static gui::Slider wEyeDistSlider{};
		static gui::CheckBox wDepthToggle{};
		wDepthToggle.setText("W-Depth Perception");
		wDepthToggle.alignX(gui::ALIGN_CENTER_X);
		wDepthToggle.offsetY(lowestY += 100);
		wDepthToggle.checked = wOffset;
		wDepthToggle.callback = [](void* user, bool checked)
			{
				wOffset = checked;
				wEyeDistSlider.clickable = wOffset;

				dumpConfig();
			};

		wEyeDistSlider.value = floorf(eyeWGap * 1000 * 10) - 30 * 10;
		wEyeDistSlider.setText(std::format("W Eye Distance: {:.1f}mm", eyeWGap * 1000.f));
		wEyeDistSlider.alignX(gui::ALIGN_CENTER_X);
		wEyeDistSlider.offsetY(lowestY += 100);
		wEyeDistSlider.clickable = wOffset;
		wEyeDistSlider.range = (80 - 30) * 10;
		wEyeDistSlider.width = wEyeDistSlider.range + 200; // 700
		wEyeDistSlider.user = &wEyeDistSlider;
		wEyeDistSlider.callback = [](void* user, int value)
			{
				eyeWGap = ((float)value / 10.f + 30.f) / 1000.f;
				((gui::Slider*)user)->setText(std::format("W Eye Distance: {:.1f}mm", eyeWGap * 1000.f));
				dumpConfig();
			};

		static gui::CheckBox desktopViewToggle{};
		desktopViewToggle.setText("Render Desktop View");
		desktopViewToggle.alignX(gui::ALIGN_CENTER_X);
		desktopViewToggle.offsetY(lowestY += 100);
		desktopViewToggle.checked = desktopView;
		desktopViewToggle.callback = [](void* user, bool checked)
			{
				desktopView = checked;

				dumpConfig();
			};

#endif
		self->mainContentBox.addElement(&vrTitle);
		self->mainContentBox.addElement(&switchButton);
		self->mainContentBox.addElement(&showDialogToggle);
#ifndef NONVR
		self->mainContentBox.addElement(&calibrateHeightButton);
		self->mainContentBox.addElement(&wDepthToggle);
		self->mainContentBox.addElement(&wEyeDistSlider);
		self->mainContentBox.addElement(&desktopViewToggle);
#endif
		self->secretButton.yOffset += lowestY - oldLowest;
		self->mainContentBox.scrollH += lowestY - oldLowest;

		initializedSettings = true;
	}
}

$exec
{
#ifndef NONVR
	atexit(shutdownVR);
#endif
}
