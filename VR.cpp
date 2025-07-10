#include "VR.h"
#include "matConverts.h"
#include "Constants.h"
#include "4DKeyBinds.h"
#include "Player.h"
#include <glm/gtx/orthonormalize.hpp>

VRStuff::_Actions VRStuff::Actions = {};
vr::IVRSystem* VRStuff::HMD = nullptr;
vr::TrackedDevicePose_t VRStuff::trackedDevicePose[vr::k_unMaxTrackedDeviceCount] = {};

VRTex VRStuff::eyeL = {};
VRTex VRStuff::eyeR = {};

glm::vec4 VRStuff::eyeLP{};
glm::vec4 VRStuff::eyeRP{};

glm::vec4 VRStuff::eyeLPN{};
glm::vec4 VRStuff::eyeRPN{};

glm::vec4 VRStuff::eyeLP3D{};
glm::vec4 VRStuff::eyeRP3D{};

glm::vec4 VRStuff::eyeLPN3D{};
glm::vec4 VRStuff::eyeRPN3D{};

bool VRStuff::wOffset = false;
float VRStuff::eyeWGap = 0.075f;
bool VRStuff::desktopView = true;

unsigned int VRStuff::eyeW = 1, VRStuff::eyeH = 1;

VRStuff::VREye VRStuff::curEye = VEye_Left;

glm::mat4 VRStuff::prjL = glm::identity<glm::mat4>();
glm::mat4 VRStuff::prjR = glm::identity<glm::mat4>();
glm::mat4 VRStuff::vEyeL = glm::identity<glm::mat4>();
glm::mat4 VRStuff::vEyeR = glm::identity<glm::mat4>();
glm::mat4 VRStuff::trackedPoses[vr::k_unMaxTrackedDeviceCount]{};
glm::mat4 VRStuff::controllerTransforms[2][2]{ glm::identity<glm::mat4>(), glm::identity<glm::mat4>()};
float VRStuff::controllerYZAngle = 0;
glm::vec3 VRStuff::trackedVelocities[vr::k_unMaxTrackedDeviceCount]{};
glm::vec3 VRStuff::trackedAngularVelocities[vr::k_unMaxTrackedDeviceCount]{};
glm::mat4 VRStuff::hmdPose = glm::identity<glm::mat4>();
glm::mat3 VRStuff::hmdDir = glm::identity<glm::mat3>();
glm::mat3 VRStuff::cDir = glm::identity<glm::mat3>();

vr::TrackedDeviceIndex_t VRStuff::lCInd = 1;
vr::TrackedDeviceIndex_t VRStuff::rCInd = 2;
vr::VRInputValueHandle_t VRStuff::cHandles[2]{ 1, 2 };

MeshRenderer VRStuff::handRenderer{};
MeshRenderer VRStuff::crosshairRenderer{};
const Shader* VRStuff::handShader = nullptr;
const Shader* VRStuff::crosshairShader = nullptr;
Mat5 VRStuff::lastCamera{ 1 };
glm::vec4 VRStuff::lastEye{};

Mat5 VRStuff::orientation{ 1 };
Mat5 VRStuff::orientation3D{ 1 };
glm::vec3 VRStuff::hmd3DPos{ 0 };
glm::vec4 VRStuff::hmd4DPos{ 0 };
bool VRStuff::keepXZCameraPosition = false;
glm::vec4 VRStuff::lastHMDPos{ 0,0,0,0 };
bool VRStuff::use3DOrientation = true;

Framebuffer3D VRStuff::uiFB{ };

StateManager* VRStuff::stateManager = nullptr;

bool VRStuff::takeAScreenshot = false;
double VRStuff::takeAScreenshotTime = 0;

glm::vec3 VRStuff::planePosition = VRStuff::menuPlanePosition;
glm::vec3 VRStuff::planeScale{ 2.f, 2.f, 1.f };
glm::vec3 VRStuff::planeNormal{ 0,0,-1 };
glm::vec3 VRStuff::planeUp{ 0,1,0 };

std::string VRStuff::configPath = "./mods/4DVR/config.json";
std::string VRStuff::infoPath = "./mods/4DVR/info.json5";
std::unordered_map<std::string, std::string> VRStuff::modeDLLs
{
	{ "VR", "4DVR.dll" },
	{ "NON-VR", "4DVR-NONVR.dll" }
};
bool VRStuff::showDialogSetting = false;
float VRStuff::heightRatio = 1.7f / Player::HEIGHT;

std::array<std::vector<VR::Haptic*>, 2> VRStuff::activeHaptics{};

using namespace vr;

#include "matConverts.h"

std::string VRStuff::getActionName(VRActionHandle_t handle)
{
	if (handle == Actions.Movement)
		return "Movement";
	else if (handle == Actions.Look4D)
		return "Look4D";
	else if (handle == Actions.Use)
		return "Use";
	else if (handle == Actions.Break)
		return "Break";
	else if (handle == Actions.Crouch)
		return "Crouch";
	else if (handle == Actions.Sprint)
		return "Sprint";
	else if (handle == Actions.Jump)
		return "Jump";
	else if (handle == Actions.Inventory)
		return "Inventory";
	else if (handle == Actions.ItemUp)
		return "ItemUp";
	else if (handle == Actions.ItemDown)
		return "ItemDown";
	else if (handle == Actions.StrafeW)
		return "StrafeWN";
	else if (handle == Actions.StrafeWP)
		return "StrafeWP";
	else if (handle == Actions.Pause)
		return "Pause";
	else if (handle == Actions.Press)
		return "Press";
	else if (handle == Actions.Action)
		return "Action";
	else if (handle == Actions.Scroll)
		return "Scroll";

	return "Unknown";
}

std::string VRStuff::EVRInputErrorToString(EVRInputError error)
{
	switch (error)
	{
	case VRInputError_None:
		return "No error";
	case VRInputError_NameNotFound:
		return "Name not found";
	case VRInputError_WrongType:
		return "Wrong type";
	case VRInputError_InvalidHandle:
		return "Invalid handle";
	case VRInputError_InvalidParam:
		return "Invalid parameter";
	case VRInputError_NoSteam:
		return "Steam not running";
	case VRInputError_MaxCapacityReached:
		return "Maximum capacity reached";
	case VRInputError_IPCError:
		return "IPC error";
	case VRInputError_NoActiveActionSet:
		return "No active action set";
	case VRInputError_InvalidDevice:
		return "Invalid device";
	case VRInputError_InvalidSkeleton:
		return "Invalid skeleton";
	case VRInputError_InvalidBoneCount:
		return "Invalid bone count";
	case VRInputError_InvalidCompressedData:
		return "Invalid compressed data";
	case VRInputError_NoData:
		return "No data";
	case VRInputError_BufferTooSmall:
		return "Buffer too small";
	case VRInputError_MismatchedActionManifest:
		return "Mismatched action manifest";
	case VRInputError_MissingSkeletonData:
		return "Missing skeleton data";
	case VRInputError_InvalidBoneIndex:
		return "Invalid bone index";
	case VRInputError_InvalidPriority:
		return "Invalid priority";
	case VRInputError_PermissionDenied:
		return "Permission denied";
	case VRInputError_InvalidRenderModel:
		return "Invalid render model";
	default:
		return "Unknown";
	}
}

bool VRStuff::getActionBool(VRActionHandle_t action)
{
	InputDigitalActionData_t data{};
	EVRInputError err = VRInput()->GetDigitalActionData(action, &data, sizeof(InputDigitalActionData_t), k_ulInvalidInputValueHandle);
	if (err == VRInputError_None && data.bActive)
		return data.bState;
	else if (err != VRInputError_None)
		Console::printLine("4DVR: ", Console::Mode(Console::GREEN, Console::BRIGHT), "getActionBool(", getActionName(action), "{", action, "}) Error:", Console::Mode(Console::YELLOW, Console::BRIGHT), EVRInputErrorToString(err));
	return false;
}
float VRStuff::getActionFloat(VRActionHandle_t action)
{
	InputAnalogActionData_t data{};
	EVRInputError err = VRInput()->GetAnalogActionData(action, &data, sizeof(InputAnalogActionData_t), k_ulInvalidInputValueHandle);
	if (err == VRInputError_None && data.bActive)
		return data.x;
	else if (err != VRInputError_None)
		Console::printLine("4DVR: ", Console::Mode(Console::GREEN, Console::BRIGHT), "getActionFloat(", getActionName(action), "{", action, "}) Error:", Console::Mode(Console::YELLOW, Console::BRIGHT), EVRInputErrorToString(err));
	return 0;
}
glm::vec2 VRStuff::getActionVec2(VRActionHandle_t action)
{
	InputAnalogActionData_t data{};
	EVRInputError err = VRInput()->GetAnalogActionData(action, &data, sizeof(InputAnalogActionData_t), k_ulInvalidInputValueHandle);
	if (err == VRInputError_None && data.bActive)
		return glm::vec2(data.x, data.y);
	else if (err != VRInputError_None)
		Console::printLine("4DVR: ", Console::Mode(Console::GREEN, Console::BRIGHT), "getActionVec2(", getActionName(action), "{", action, "}) Error:", Console::Mode(Console::YELLOW, Console::BRIGHT), EVRInputErrorToString(err));
	return glm::vec2(0);
}
glm::vec3 VRStuff::getActionVec3(VRActionHandle_t action)
{
	InputAnalogActionData_t data{};
	EVRInputError err = VRInput()->GetAnalogActionData(action, &data, sizeof(InputAnalogActionData_t), k_ulInvalidInputValueHandle);
	if (err == VRInputError_None && data.bActive)
		return glm::vec3(data.x, data.y, data.z);
	else if (err != VRInputError_None)
		Console::printLine("4DVR: ", Console::Mode(Console::GREEN, Console::BRIGHT), "getActionVec3(", getActionName(action), "{", action, "}) Error:", Console::Mode(Console::YELLOW, Console::BRIGHT), EVRInputErrorToString(err));
	return glm::vec3(0);
}

void VRStuff::shutdownVR()
{
	if (HMD)
	{
		VR_Shutdown();

		HMD = NULL;
	}
}

bool VRStuff::initVR()
{
	initKeyBindsActions();

	std::filesystem::path modPath = std::filesystem::path{ fdm::getModPath(fdm::modID) };
	// generate action manifest and default bindings
	{
		nlohmann::json actionManifest;
		{
			std::ifstream input{ modPath / "assets/action-manifest.json" };
			if (input.is_open())
			{
				actionManifest = nlohmann::json::parse(input);
				input.close();
			}
			else
			{
				MessageBoxA(0, "Couldn't load \"assets/action-manifest.json\"!", "4DVR Fail!", MB_ICONERROR);
				exit(0);
				return false;
			}

			for (auto& action : customActions)
			{
				nlohmann::json actionObj
				{
					{ "name", action.second.id },
				};

				switch (action.second.type)
				{
				case VR::ACTION_BOOL:
					actionObj["type"] = "boolean";
					break;
				case VR::ACTION_FLOAT:
					actionObj["type"] = "vector1";
					break;
				case VR::ACTION_VEC2:
					actionObj["type"] = "vector2";
					break;
				}
				switch (action.second.requirement)
				{
				case VR::ACTION_MANDATORY:
					actionObj["requirement"] = "mandatory";
					break;
				case VR::ACTION_SUGGESTED:
					actionObj["requirement"] = "suggested";
					break;
				case VR::ACTION_OPTIONAL:
					actionObj["requirement"] = "optional";
					break;
				}
				actionManifest["actions"].push_back(actionObj);

				for (auto& localization : actionManifest["localization"])
				{
					localization[action.second.id] = action.second.name;
				}
			}

			std::ofstream output{ modPath / "action-manifest.json" };
			if (output.is_open())
			{
				output << actionManifest.dump(1, '\t');
				output.close();
			}
		}
		nlohmann::json defaultBindings;
		{
			std::ifstream input{ modPath / "assets/default-bindings.json" };
			if (input.is_open())
			{
				defaultBindings = nlohmann::json::parse(input);
				input.close();
			}
			else
			{
				MessageBoxA(0, "Couldn't load \"assets/default-bindings.json\"!", "4DVR Fail!", MB_ICONERROR);
				exit(0);
				return false;
			}

			for (auto& bindSet : customDefaultBindings)
			{
				std::string setName = "";
				switch (bindSet.first)
				{
				case VR::ACTION_SET_UI:
					setName = "/actions/menu";
					break;
				default:
				case VR::ACTION_SET_INGAME:
					setName = "/actions/main";
					break;
				}

				for (auto& binding : bindSet.second)
				{
					nlohmann::json bindingObj
					{
						{ "inputs", binding.inputs },
						{ "parameters", binding.parameters },
						{ "mode", binding.mode },
						{ "path", binding.path },
					};

					defaultBindings["bindings"][setName]["sources"].push_back(bindingObj);
				}
			}

			std::ofstream output{ modPath / "default-bindings.json" };
			if (output.is_open())
			{
				output << defaultBindings.dump(1, '\t');
				output.close();
			}
		}
	}

	// load the app manifest and assign the app key
	std::filesystem::path pathApp = std::filesystem::absolute(modPath / "4dvr.vrmanifest");
	std::string pathAppStr = pathApp.string();

	// init openvr runtime
	EVRInitError er = VRInitError_None;
	HMD = VR_Init(&er, VRApplication_Scene);

	if (er != VRInitError_None)
	{
		HMD = NULL;
		printf("4DVR: ERROR: Unable to init OpenVR runtime: %s\n", VR_GetVRInitErrorAsEnglishDescription(er));
		return false;
	}

	VRApplications()->AddApplicationManifest(pathAppStr.c_str());
	VRApplications()->IdentifyApplication(VRApplications()->GetCurrentSceneProcessId(), "tr1ngledev.4dvr");

	HMD->GetRecommendedRenderTargetSize(&eyeW, &eyeH);

	float l, r, t, b;

	HMD->GetProjectionRaw(Eye_Left, &l, &r, &t, &b);
	prjL = composeProjection(l, r, t, b, 0.003f, 1000.f);

	HMD->GetProjectionRaw(Eye_Right, &l, &r, &t, &b);
	prjR = composeProjection(l, r, t, b, 0.003f, 1000.f);

	vEyeL = glm::inverse(ToMat34(HMD->GetEyeToHeadTransform(Eye_Left)));
	vEyeR = glm::inverse(ToMat34(HMD->GetEyeToHeadTransform(Eye_Right)));

	// init openvr compositor
	if (!VRCompositor())
	{
		printf("4DVR: ERROR: OpenVR Compositor Initialization Failed.\n");
		return false;
	}

	VRCompositor()->WaitGetPoses(trackedDevicePose, k_unMaxTrackedDeviceCount, NULL, 0);

	lCInd = HMD->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_LeftHand);
	rCInd = HMD->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_RightHand);


	// setup steamvr input actions
	std::filesystem::path pathInput = std::filesystem::absolute(modPath / "action-manifest.json");
	std::string pathInputStr = pathInput.string();

	VRInput()->SetActionManifestPath(pathInputStr.c_str());

	VRInput()->GetActionSetHandle("/actions/main", &Actions.MainSet);
	VRInput()->GetActionHandle("/actions/main/in/Movement", &Actions.Movement);
	VRInput()->GetActionHandle("/actions/main/in/Look4D", &Actions.Look4D);
	VRInput()->GetActionHandle("/actions/main/in/Use", &Actions.Use);
	VRInput()->GetActionHandle("/actions/main/in/Break", &Actions.Break);
	VRInput()->GetActionHandle("/actions/main/in/Crouch", &Actions.Crouch);
	VRInput()->GetActionHandle("/actions/main/in/Sprint", &Actions.Sprint);
	VRInput()->GetActionHandle("/actions/main/in/Jump", &Actions.Jump);
	VRInput()->GetActionHandle("/actions/main/in/Inventory", &Actions.Inventory);
	VRInput()->GetActionHandle("/actions/main/in/Pause", &Actions.Pause);
	VRInput()->GetActionHandle("/actions/main/in/ItemUp", &Actions.ItemUp);
	VRInput()->GetActionHandle("/actions/main/in/ItemDown", &Actions.ItemDown);
	VRInput()->GetActionHandle("/actions/main/in/StrafeW", &Actions.StrafeW);
	VRInput()->GetActionHandle("/actions/main/in/StrafeWP", &Actions.StrafeWP);
	VRInput()->GetActionHandle("/actions/main/in/QuickActions", &Actions.QuickActions);

	VRInput()->GetActionSetHandle("/actions/menu", &Actions.MenuSet);
	VRInput()->GetActionHandle("/actions/menu/in/Press", &Actions.Press);
	VRInput()->GetActionHandle("/actions/menu/in/Action", &Actions.Action);
	VRInput()->GetActionHandle("/actions/menu/in/Scroll", &Actions.Scroll);

	VRInput()->GetActionSetHandle("/actions/tutorial", &Actions.TutorialSet);
	VRInput()->GetActionHandle("/actions/tutorial/in/Move", &Actions.Move);
	VRInput()->GetActionHandle("/actions/tutorial/in/DragRotate", &Actions.DragRotate);
	VRInput()->GetActionHandle("/actions/tutorial/in/ForgMoveLeft", &Actions.ForgMoveLeft);
	VRInput()->GetActionHandle("/actions/tutorial/in/ForgMoveRight", &Actions.ForgMoveRight);
	VRInput()->GetActionHandle("/actions/tutorial/in/ForgRotateLeft", &Actions.ForgRotateLeft);
	VRInput()->GetActionHandle("/actions/tutorial/in/ForgRotateRight", &Actions.ForgRotateRight);
	VRInput()->GetActionHandle("/actions/tutorial/in/ForgJump", &Actions.ForgJump);

	VRInput()->GetActionHandle("/actions/main/out/LeftHaptics", &Actions.LeftHaptics);
	VRInput()->GetActionHandle("/actions/main/out/RightHaptics", &Actions.RightHaptics);

	for (auto& action : customActions)
	{
		VRInput()->GetActionHandle(action.second.id.c_str(), &action.second.ovrHandle);
	}

	VRInput()->GetInputSourceHandle("/user/hand/left", &cHandles[0]);
	VRInput()->GetInputSourceHandle("/user/hand/right", &cHandles[1]);

	eyeL = VRTex();
	eyeL.init(eyeW, eyeH);

	eyeR = VRTex();
	eyeR.init(eyeW, eyeH);

	handleEvents();

	return true;
}

glm::mat4 VRStuff::composeProjection(float fLeft, float fRight, float fTop, float fBottom, float zNear, float zFar)
{
	float idx = 1.0f / (fRight - fLeft);
	float idy = 1.0f / (fBottom - fTop);
	float idz = 1.0f / (zFar - zNear);
	float sx = fRight + fLeft;
	float sy = fBottom + fTop;

	glm::mat4 p;
	p[0][0] = 2 * idx;   p[1][0] = 0;     p[2][0] = sx * idx;    			p[3][0] = 0;
	p[0][1] = 0;       p[1][1] = 2 * idy; p[2][1] = sy * idy;    			p[3][1] = 0;
	p[0][2] = 0;       p[1][2] = 0;     p[2][2] = -(zFar + zNear) * idz;	p[3][2] = -2 * zFar * zNear * idz;
	p[0][3] = 0;       p[1][3] = 0;      p[2][3] = -1.0f;     			p[3][3] = 0;
	return p;
}

inline static gui::Window* lastWindow = nullptr;
gui::TextInput* VRStuff::activeTextInput = nullptr;

void VRStuff::processEvent(const VREvent_t& event)
{
	switch (event.eventType)
	{
	case VREvent_TrackedDeviceActivated:
	{
		printf("4DVR: Tracked Device %u attached.\n", event.trackedDeviceIndex);
		if (event.trackedDeviceIndex == HMD->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_LeftHand))
			lCInd = event.trackedDeviceIndex;
		else if (event.trackedDeviceIndex == HMD->GetTrackedDeviceIndexForControllerRole(TrackedControllerRole_RightHand))
			rCInd = event.trackedDeviceIndex;
	}break;
	case VREvent_TrackedDeviceDeactivated:
	{
		printf("4DVR: Tracked Device %u detached.\n", event.trackedDeviceIndex);
	}break;
	case VREvent_TrackedDeviceUpdated:
	{
		printf("4DVR: Tracked Device %u updated.\n", event.trackedDeviceIndex);
	}break;
	/*case VREvent_KeyboardCharInput:
	{
		auto w = reinterpret_cast<gui::Window*>(event.data.keyboard.uUserValue);
		for(int i = 0; i < 8; i++)
			activeTextInput->charInput(w, event.data.keyboard.cNewInput[i]);
	}break;*/
	case VREvent_KeyboardDone:
	{
		auto w = reinterpret_cast<gui::Window*>(event.data.keyboard.uUserValue);

		char keyboardBuffer[StateGame::MESSAGE_DISPLAY_MAX + 1]{ 0 };
		uint32_t len = vr::VROverlay()->GetKeyboardText(keyboardBuffer, StateGame::MESSAGE_DISPLAY_MAX);

		activeTextInput->text = stl::string(keyboardBuffer, keyboardBuffer + len);
		activeTextInput->cursorPos = len;
		if (w)
		{
			activeTextInput->charInput(w, ' ');
			activeTextInput->keyInput(w, GLFW_KEY_BACKSPACE, 0, GLFW_PRESS, 0);
			activeTextInput->keyInput(w, GLFW_KEY_BACKSPACE, 0, GLFW_RELEASE, 0);
		}

		if (activeTextInput == &StateGame::instanceObj.chatInput)
		{
			if (!activeTextInput->text.empty())
			{
				StateGame::instanceObj.world->sendChatMessage(&StateGame::instanceObj.player, activeTextInput->text);
				StateGame::instanceObj.addChatMessage(&StateGame::instanceObj.player, activeTextInput->text, 0xFFFFFF);
				activeTextInput->text = "";
				activeTextInput->cursorPos = 0;
			}
			StateGame::instanceObj.chatOpen = false;
		}

		activeTextInput->deselect();
		activeTextInput = nullptr;
	}break;
	case VREvent_KeyboardClosed:
	{
		activeTextInput = nullptr;
	}break;
	}
}
#ifndef NONVR
$hook(void, gui::TextInput, render, const gui::Window* w)
{
	lastWindow = (gui::Window*)w;
	return original(self, w);
}
$hook(void, gui::TextInput, select)
{
	original(self);

	if (self->active && self->editable)
	{
		if (VRStuff::activeTextInput != self)
		{
			VROverlay()->ShowKeyboard(
				self->passwordMode ? EGamepadTextInputMode::k_EGamepadTextInputModePassword : EGamepadTextInputMode::k_EGamepadTextInputModeNormal,
				EGamepadTextInputLineMode::k_EGamepadTextInputLineModeSingleLine,
				EKeyboardFlags::KeyboardFlag_Modal, "", StateGame::MESSAGE_DISPLAY_MAX, self->text.c_str(), reinterpret_cast<uint64_t>(lastWindow));

			VRStuff::activeTextInput = self;
		}
	}
	else
	{
		if (VRStuff::activeTextInput == self)
		{
			VROverlay()->HideKeyboard();
			VRStuff::activeTextInput = nullptr;
		}
	}
}
$hook(void, gui::TextInput, deselect)
{
	original(self);

	if (VRStuff::activeTextInput == self)
	{
		VROverlay()->HideKeyboard();
		VRStuff::activeTextInput = nullptr;
	}
}
#endif
bool VRStuff::pointingCursor = false;
void VRStuff::handleEvents()
{
	// Process SteamVR events
	VREvent_t event;
	while (HMD->PollNextEvent(&event, sizeof(event)))
		processEvent(event);

	VRActiveActionSet_t actionSet[3]{ {Actions.MainSet}, {Actions.MenuSet}, {Actions.TutorialSet} };
	VRInput()->UpdateActionState(actionSet, sizeof(VRActiveActionSet_t), 3);

	if (pointingCursor)
	{
		static bool prevPressDown = false;
		bool pressDown = getActionBool(Actions.Press);
		static bool prevActionDown = false;
		bool actionDown = getActionBool(Actions.Action);

		glm::vec2 scroll = getActionVec2(Actions.Scroll);

		if (scroll.x != 0 || scroll.y != 0)
			main_cpp::scrollCallback(stateManager->window, scroll.x, scroll.y);

		// simulate LMB
		if (pressDown && !prevPressDown)
			main_cpp::mouseButtonCallback(stateManager->window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
		//else if (pressDown && prevPressDown)
		//	main_cpp::mouseButtonCallback(stateManager->window, GLFW_MOUSE_BUTTON_LEFT, GLFW_REPEAT, 0);
		else if (!pressDown && prevPressDown)
			main_cpp::mouseButtonCallback(stateManager->window, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);

		// simulate RMB
		if (actionDown && !prevActionDown)
			main_cpp::mouseButtonCallback(stateManager->window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, 0);
		//else if (actionDown && prevActionDown)
		//	main_cpp::mouseButtonCallback(stateManager->window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_REPEAT, 0);
		else if (!actionDown && prevActionDown)
			main_cpp::mouseButtonCallback(stateManager->window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, 0);

		prevPressDown = pressDown;
		prevActionDown = actionDown;
	}

	/*for (TrackedDeviceIndex_t unDevice = 0; unDevice < k_unMaxTrackedDeviceCount; unDevice++)
	{
		VRControllerState_t state;
		HMD->GetControllerState(unDevice, &state, sizeof(state));
	}*/

	pointingCursor = false;

	// mix haptics
	{
		for (int i = 0; i < 2; ++i)
		{
			auto out = i == 0 ? Actions.LeftHaptics : Actions.RightHaptics;
			auto& haptics = activeHaptics[i];
			double time = glfwGetTime();

			float amplitude = 0.0f;
			float frequency = 0.0f;

			for (int j = 0; j < haptics.size();)
			{
				if (!haptics[j])
				{
					j = haptics.erase(haptics.begin() + j) - haptics.begin();
				}
				else
				{
					auto& haptic = haptics[j];

					if (haptic->startTime <= time)
					{
						float amp = haptic->amplitude(time);
						amplitude += amp;
						frequency += amp * haptic->frequency(time);
					}

					if (time - haptic->startTime > haptic->duration)
					{
						if (haptic->destroy)
							delete haptic;
						haptic = nullptr;
					}

					++j;
				}
			}

			if (amplitude > 0.001f)
			{
				frequency /= amplitude;

				amplitude = glm::clamp(amplitude, 0.0f, 1.0f);
				VRInput()->TriggerHapticVibrationAction(out, 0.0f, 0.02f, frequency, amplitude, k_ulInvalidInputValueHandle);
			}
		}
	}
}

void VRStuff::eraseAllHaptics()
{
	for (int i = 0; i < 2; ++i)
	{
		for (auto& haptic : activeHaptics[i])
		{
			if (haptic->destroy)
				delete haptic;
		}
		activeHaptics[i].clear();
	}
}

std::string VRStuff::GetTrackedDeviceString(TrackedDeviceIndex_t unDevice, TrackedDeviceProperty prop, TrackedPropertyError* peError)
{
	uint32_t unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char* pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}

static m4::Mat5 lastOrientationMult{ 1 };
void VRStuff::updateHMDMatrixPose(const glm::vec4& over, Mat5* orientationMult)
{
	// proper blending modes
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	if (!HMD) return;
	if (!HMD->IsTrackedDeviceConnected(k_unTrackedDeviceIndex_Hmd)) return;
	VRCompositor()->WaitGetPoses(trackedDevicePose, k_unMaxTrackedDeviceCount, NULL, 0);

	for (int nDevice = 0; nDevice < k_unMaxTrackedDeviceCount; nDevice++)
	{
		if (trackedDevicePose[nDevice].bDeviceIsConnected && trackedDevicePose[nDevice].bPoseIsValid)
		{
			trackedPoses[nDevice] = ToMat34(trackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			trackedVelocities[nDevice] = { trackedDevicePose[nDevice].vVelocity.v[0], trackedDevicePose[nDevice].vVelocity.v[1], trackedDevicePose[nDevice].vVelocity.v[2] };
			trackedAngularVelocities[nDevice] = { trackedDevicePose[nDevice].vAngularVelocity.v[0], trackedDevicePose[nDevice].vAngularVelocity.v[1], trackedDevicePose[nDevice].vAngularVelocity.v[2] };
		}
	}
	// update controller transforms
	{
		IVRRenderModels* renderModels = VRRenderModels();
		std::string renderModel[2]{"",""};

		if (HMD->IsTrackedDeviceConnected(lCInd))
			renderModel[0] = GetTrackedDeviceString(lCInd, ETrackedDeviceProperty::Prop_RenderModelName_String);
		if (HMD->IsTrackedDeviceConnected(rCInd))
			renderModel[1] = GetTrackedDeviceString(rCInd, ETrackedDeviceProperty::Prop_RenderModelName_String);

		for (int i = 0; i < 2; i++)
		{
			if (renderModel[i].empty()) continue;
			for (int j = 0; j < 2; j++)
			{
				RenderModel_ComponentState_t cState{};
				RenderModel_ControllerMode_State_t state{};
				if (renderModels->GetComponentStateForDevicePath(renderModel[i].c_str(), controllerComponentNames[j], cHandles[i], &state, &cState))
				{
					controllerTransforms[i][j] = ToMat34(cState.mTrackingToComponentLocal);
				}
			}
		}
		// controllerYZAngle update
		glm::mat4& tip = controllerTransforms[1][0];
		glm::mat4& handgrip = controllerTransforms[1][1];
		glm::vec3 tipF = glm::normalize(tip * glm::vec4{ 0,0,-1,0 });
		glm::vec3 handgripF = glm::normalize(handgrip * glm::vec4{ 0,0,-1,0 });
		float d0 = glm::acos(glm::abs(glm::dot(tipF, handgripF)));
		controllerYZAngle = -d0;
		//printf("%f\n", glm::degrees(controllerYZAngle));
		if (HMD->IsTrackedDeviceConnected(rCInd))
		{
			//printf("ang vel right controller:\t%f; %f; %f\n"
			//	"vel right controller:\t\t%f; %f; %f\n",
			//	trackedAngularVelocities[rCInd].x, trackedAngularVelocities[rCInd].y, trackedAngularVelocities[rCInd].z,
			//	trackedVelocities[rCInd].x, trackedVelocities[rCInd].y, trackedVelocities[rCInd].z);
		}
	}

	hmdPose = trackedPoses[k_unTrackedDeviceIndex_Hmd];

	hmdDir = glm::inverse(glm::mat3
		{
			hmdPose[0][0],hmdPose[1][0],hmdPose[2][0],
			hmdPose[0][1],hmdPose[1][1],hmdPose[2][1],
			hmdPose[0][2],hmdPose[1][2],hmdPose[2][2]
		});

	if (HMD->IsTrackedDeviceConnected(rCInd))
	{
		glm::mat4 controller = trackedPoses[rCInd];

		cDir = glm::inverse(glm::mat3
			{
				controller[0][0],controller[1][0],controller[2][0],
				controller[0][1],controller[1][1],controller[2][1],
				controller[0][2],controller[1][2],controller[2][2]
			});
	}

	vEyeL = ToMat34(HMD->GetEyeToHeadTransform(Eye_Left)) * hmdPose;
	vEyeR = ToMat34(HMD->GetEyeToHeadTransform(Eye_Right)) * hmdPose;
	/*
	if (gameState)
	{
		gameState->player.cameraPos.x = gameState->player.pos.x;
		gameState->player.cameraPos.y = gameState->player.pos.y;
		gameState->player.cameraPos.z = gameState->player.pos.z;
	}
	*/

	// updateHMDMatrixPose() (called every frame)

	orientation3D = { 1 };

	orientation3D[0][0] = hmdDir[0][0];
	orientation3D[0][1] = -hmdDir[1][0];
	orientation3D[0][2] = hmdDir[2][0];

	orientation3D[1][0] = -hmdDir[0][1];
	orientation3D[1][1] = hmdDir[1][1];
	orientation3D[1][2] = -hmdDir[2][1];

	orientation3D[2][0] = hmdDir[0][2];
	orientation3D[2][1] = -hmdDir[1][2];
	orientation3D[2][2] = hmdDir[2][2];

	orientation = orientation3D;
	//Mat5 mR = Mat5(m4::Rotor({ 0,0,0,1 }, over));
	
	if (orientationMult)
	{
		orientation = *orientationMult * orientation;
		lastOrientationMult = *orientationMult;
	}
	else
	{
		lastOrientationMult = { 1 };
	}
	//else
	//{
	//	orientation = mR * orientation3D;
	//}

	//if (gameState)
	//	gameState->player.orientation = orientation;

	Mat5 eyeLM{ 1 };
	Mat5 eyeRM{ 1 };

	eyeLM[0][0] = eyeRM[0][0] = hmdDir[0][0];
	eyeLM[0][1] = eyeRM[0][1] = -hmdDir[1][0];
	eyeLM[0][2] = eyeRM[0][2] = hmdDir[2][0];

	eyeLM[1][0] = eyeRM[1][0] = -hmdDir[0][1];
	eyeLM[1][1] = eyeRM[1][1] = hmdDir[1][1];
	eyeLM[1][2] = eyeRM[1][2] = -hmdDir[2][1];

	eyeLM[2][0] = eyeRM[2][0] = hmdDir[0][2];
	eyeLM[2][1] = eyeRM[2][1] = -hmdDir[1][2];
	eyeLM[2][2] = eyeRM[2][2] = hmdDir[2][2];

	eyeLM[4][0] = -vEyeL[0][3];
	eyeLM[4][1] = vEyeL[1][3];
	eyeLM[4][2] = -vEyeL[2][3];

	eyeRM[4][0] = -vEyeR[0][3];
	eyeRM[4][1] = vEyeR[1][3];
	eyeRM[4][2] = -vEyeR[2][3];

	eyeLP3D = *(glm::vec4*)eyeLM[4];
	eyeRP3D = *(glm::vec4*)eyeRM[4];

	if (wOffset)
	{
		eyeLM[4][3] = eyeWGap * 0.5f;
		eyeRM[4][3] = eyeWGap * -0.5f;
	}

	if (orientationMult)
	{
		eyeLM = *orientationMult * eyeLM;
		eyeRM = *orientationMult * eyeRM;
	}
	//else
	//{
	//	eyeLM = mR * eyeLM;
	//	eyeRM = mR * eyeRM;
	//}

	eyeLP = *(glm::vec4*)eyeLM[4];
	eyeRP = *(glm::vec4*)eyeRM[4];

	hmd3DPos = { -hmdPose[0][3],0,-hmdPose[2][3] };
	hmd4DPos = glm::vec4(hmd3DPos, 0);
	if (orientationMult)
		hmd4DPos = orientationMult->multiply(hmd4DPos, 0);

	/*
	if (gameState)
	{
		gameState->player.left = *(glm::vec4*)customOrientation[0];

		gameState->player.up = *(glm::vec4*)customOrientation[1];

		gameState->player.forward = *(glm::vec4*)customOrientation[2];
	}
	*/
}

void VRStuff::renderUIPlane()
{
	double t = glfwGetTime();

	use3DOrientation = true;
	Mat5 MV = m4::createCamera({ 0,0,0,0 }, { 0,0,1,0 }, { 0,1,0,0 }, { 1,0,0,0 }, { 0,0,0,1 });
	use3DOrientation = false;

	MV.translate(glm::vec4{ 0,0,0,0.01f });
	
	const Shader* shader = uiFB.getShader();
	shader->use();

	glm::mat4 MVm4 = ToMat4(MV);
	glm::mat4 rotation = glm::identity<glm::mat4>();
	rotation[0] = glm::vec4(glm::cross(planeUp, planeNormal), 0);
	rotation[1] = glm::vec4(planeUp, 0);
	rotation[2] = glm::vec4(planeNormal, 0);
	MVm4 = glm::translate(MVm4, planeCenter());
	MVm4 *= rotation;
	MVm4 = glm::translate(MVm4, planeOffset());
	MVm4 = glm::scale(MVm4, glm::vec3{ 1.f, (float)HEIGHT_UI / (float)WIDTH_UI, 1.f } * planeScale);

	glUniformMatrix4fv(glGetUniformLocation(shader->id(), "MV"), 1, GL_FALSE, &MVm4[0][0]);
	uiFB.render();
}

bool VRStuff::rayPlaneIntersection(const VRRay& ray, const glm::vec3& planeNormal, const glm::vec3& planeUp, const glm::vec3& planeCenter, glm::vec2& result)
{
	glm::vec3 planeCenterR = { planeCenter.x, planeCenter.y, planeCenter.z };
	float denom = glm::dot(ray.dir, planeNormal);

	if (glm::abs(denom) < 1e-6f)
		return false;

	float t = glm::dot(planeCenterR - ray.origin, planeNormal) / denom;

	if (t < 0)
		return false;

	glm::vec3 intersection = ray.origin + t * ray.dir;

	glm::vec3 planeX = glm::normalize(glm::cross(planeUp, planeNormal));
	glm::vec3 planeY = glm::cross(planeNormal, planeX);

	glm::vec3 intersectionToCenter = intersection - planeCenterR;
	result = glm::vec2(glm::dot(intersectionToCenter, planeX), glm::dot(intersectionToCenter, planeY));

	return true;
}

bool VRStuff::pointCursor(bool renderCursor, bool forceFocus, bool leftHand)
{
	static double xpos = 0, ypos = 0, lastTime = glfwGetTime();

	double time = glfwGetTime();
	double dt = time - lastTime;
	bool isFocused = glfwGetWindowAttrib(stateManager->window, GLFW_FOCUSED);

	if (!isFocused && forceFocus)
		isFocused = GLFW_TRUE;

	if (isFocused && HMD->IsTrackedDeviceConnected(leftHand ? lCInd : rCInd))
	{
		glm::mat4 controller = trackedPoses[leftHand ? lCInd : rCInd];
		controller = glm::rotate(glm::identity<glm::mat4>(), controllerYZAngle, { 1, 0, 0 }) * controller;
		controller = glm::transpose(glm::translate(glm::identity<glm::mat4>(), glm::vec3(leftHand ? -0.01f : 0.01f, 0.01f, 0.04f))) * controller;
		glm::mat4 dir = glm::inverse(glm::mat3
			{
				controller[0][0],controller[1][0],controller[2][0],
				controller[0][1],controller[1][1],controller[2][1],
				controller[0][2],controller[1][2],controller[2][2]
			});

		/*{
			glm::mat4 controllerL = trackedPoses[lCInd];
			controllerL = glm::rotate(glm::identity<glm::mat4>(), controllerYZAngle, { 1, 0, 0 }) * controllerL;
			glm::mat3 dirL = glm::inverse(glm::mat3
				{
					controllerL[0][0],controllerL[1][0],controllerL[2][0],
					controllerL[0][1],controllerL[1][1],controllerL[2][1],
					controllerL[0][2],controllerL[1][2],controllerL[2][2]
				});
			planeUp = { dirL[0][0],-dirL[1][0],dirL[2][0] };
			planeNormal = { dirL[0][2],-dirL[1][2],dirL[2][2] };
			planePosition = glm::vec3{ -controllerL[0][3], controllerL[1][3], -controllerL[2][3] } + planeOffset();
		}*/

		glm::vec3 origin{ -controller[0][3], controller[1][3], -controller[2][3] };
		glm::vec3 dirr{ dir[0][1], -dir[1][1], dir[2][1] };
		glm::vec2 p;

		if (rayPlaneIntersection({ origin, dirr }, planeNormal, planeUp, planeCenter(), p))
		{
			double xposTarget = ((p.x / ((float)WIDTH_UI / (float)WIDTH_UI * 0.5 * planeScale.x)) * 0.5 + 0.5) * WIDTH_UI;
			double yposTarget = ((-p.y / ((float)HEIGHT_UI / (float)WIDTH_UI * 0.5 * planeScale.y)) * 0.5 + 0.5) * HEIGHT_UI;
			if (xposTarget >= 0 && xposTarget <= WIDTH_UI && yposTarget >= 0 && yposTarget <= HEIGHT_UI)
			{
				xpos = ilerp(xpos, xposTarget, 0.25f, dt);
				ypos = ilerp(ypos, yposTarget, 0.25f, dt);
				glfwSetCursorPos(stateManager->window, xpos, ypos);
				pointingCursor = true;
			}
		}
	}
	lastTime = time;
	
	if (renderCursor)
	{
		::renderPointer();
	}

	return pointingCursor;
}

bool VRStuff::renderingUI = false;
std::unordered_map<VR::ActionHandle, VRStuff::KeyBindActionState> VRStuff::keybindsActions{};

void VRStuff::initKeyBindsActions()
{
	// add 4dkeybinds binds as actions
	if (KeyBinds::isLoaded())
	{
		auto playerBinds = KeyBinds::getBinds(KeyBindsScope::PLAYER);

		std::map<stl::string, std::vector<stl::string>> bindsMap;
		for (auto& bind : playerBinds)
		{
			bindsMap[bind.first].push_back(bind.second);
		}

		size_t i = 0;
		for (auto& ns : bindsMap)
		{
			if (ns.first == "4D Miner") continue;
			for (auto& bind : ns.second)
			{
				std::string id = std::format("{}{}", i, bind);
				std::string name = std::format("[{}] {}", ns.first, bind);

				VR::ActionHandle handle = addAction(KeyBinds::id, id, name, VR::ACTION_BOOL, VR::ACTION_SET_INGAME);
				if (handle)
				{
					keybindsActions[handle] = { false, false, ns.first, bind };
				}
			}
			++i;
		}
	}
}

#ifndef NONVR
$hook(void, gui::Interface, mouseInput, double x, double y)
{
	VRStuff::renderingUI = true;
	original(self, x, y);
}
$hook(void, gui::Interface, mouseButtonInput, int btn, int act, int mods)
{
	VRStuff::renderingUI = true;
	original(self, btn, act, mods);
}
$hook(void, gui::Interface, render)
{
	VRStuff::renderingUI = true;
	original(self);
}
$hook(void, gui::Interface, getSize, int* w, int* h)
{
	VRStuff::renderingUI = true;
	*w = WIDTH_UI;
	*h = HEIGHT_UI;
}
$hook(void, gui::ContentBox, render, gui::Window* w)
{
	VRStuff::renderingUI = true;
	original(self, w);
}

inline static void(__fastcall* glfwGetWindowSizeO)(GLFWwindow* window, int* w, int* h);
inline static void __fastcall glfwGetWindowSizeH(GLFWwindow* window, int* w, int* h)
{
	if (!VRStuff::renderingUI)
	{
		if (VRStuff::curEye != VRStuff::VEye_PC)
		{
			*w = VRStuff::eyeW;
			*h = VRStuff::eyeH;
			return;
		}
		return glfwGetWindowSizeO(window, w, h);
	}

	*w = WIDTH_UI;
	*h = HEIGHT_UI;
}

inline static void(__fastcall* glfwGetFramebufferSizeO)(GLFWwindow* window, int* w, int* h);
inline static void __fastcall glfwGetFramebufferSizeH(GLFWwindow* window, int* w, int* h)
{
	if (!VRStuff::renderingUI)
	{
		if (VRStuff::curEye != VRStuff::VEye_PC)
		{
			*w = VRStuff::eyeW;
			*h = VRStuff::eyeH;
			return;
		}
		return glfwGetFramebufferSizeO(window, w, h);
	}

	*w = WIDTH_UI;
	*h = HEIGHT_UI;
}

$exec
{
	Hook((LPVOID)GetProcAddress(GetModuleHandleA("glfw3.dll"), "glfwGetWindowSize"), (LPVOID)glfwGetWindowSizeH, (LPVOID*)&glfwGetWindowSizeO);
	Hook((LPVOID)GetProcAddress(GetModuleHandleA("glfw3.dll"), "glfwGetFramebufferSize"), (LPVOID)glfwGetFramebufferSizeH, (LPVOID*)&glfwGetFramebufferSizeO);
}

$hookStatic(void, main_cpp, framebufferSizeCallback, GLFWwindow* window, int w, int h)
{
	original(window, WIDTH_UI, HEIGHT_UI);
}

#endif

void VRStuff::dumpConfig(bool justRestarted, const std::string& vrMode)
{
	std::ofstream configFile(configPath);

	if (configFile.is_open())
	{
		configFile << (nlohmann::json{
			{ "mode", vrMode },
			{ "showDialog", showDialogSetting },
			{ "heightRatio", heightRatio },
			{ "justRestarted", justRestarted },
			{ "wDepth", wOffset },
			{ "wEyeDistance", eyeWGap },
			{ "desktopView", desktopView },
			}).dump(4);
		configFile.close();
	}
}

void VRStuff::updateProjection2D(const glm::mat4& proj)
{
	const Shader* qs = ShaderManager::get("quadShader");
	qs->use();
	glUniformMatrix4fv(glGetUniformLocation(qs->id(), "P"), 1, GL_FALSE, &proj[0][0]);

	const Shader* textShader = ShaderManager::get("textShader");
	textShader->use();
	glUniformMatrix4fv(glGetUniformLocation(textShader->id(), "P"), 1, GL_FALSE, &proj[0][0]);

	const Shader* tex2DShader = ShaderManager::get("tex2DShader");
	tex2DShader->use();
	glUniformMatrix4fv(glGetUniformLocation(tex2DShader->id(), "P"), 1, GL_FALSE, &proj[0][0]);

	const Shader* tex2DArrayShader = ShaderManager::get("tex2DArrayShader");
	tex2DArrayShader->use();
	glUniformMatrix4fv(glGetUniformLocation(tex2DArrayShader->id(), "P"), 1, GL_FALSE, &proj[0][0]);
}
void VRStuff::updateProjection(const glm::ivec2& size, const glm::ivec2& translate2D)
{
	StateTitleScreen::instanceObj.updateProjection(size.x, size.y);

	glm::mat4 proj2D = glm::ortho(0.f, (float)size.x, (float)size.y, 0.f, -1.f, 1.f);
	proj2D = glm::translate(proj2D, { translate2D.x, translate2D.y, 0 });
	updateProjection2D(proj2D);
}
void VRStuff::updateProjection(int width, int height)
{
	StateTitleScreen::instanceObj.updateProjection(width, height);

	glm::mat4 proj2D = glm::ortho(0.f, (float)width, (float)height, 0.f, -1.f, 1.f);
	updateProjection2D(proj2D);
	glViewport(0, 0, width, height);
}
fdm::m4::BiVector4 VRStuff::rotateBiVec4(const fdm::m4::BiVector4& b, const fdm::m4::Mat5& m)
{
	// some sort of "sandwich" idfk

	glm::mat4 B{ 0 };

	B[0][1] = b.xy;
	B[0][2] = b.xz;
	B[0][3] = b.xw;
	B[1][2] = b.yz;
	B[1][3] = b.yw;
	B[2][3] = b.zw;

	B[1][0] = -b.xy;
	B[2][0] = -b.xz;
	B[3][0] = -b.xw;
	B[2][1] = -b.yz;
	B[3][1] = -b.yw;
	B[3][2] = -b.zw;

	glm::mat4 m4x4
	{
		*(glm::vec4*)m[0],
		*(glm::vec4*)m[1],
		*(glm::vec4*)m[2],
		*(glm::vec4*)m[3]
	};
	// R * B * R^-1 or smth?
	glm::mat4 BP = m4x4 * B * glm::transpose(m4x4);

	return fdm::m4::BiVector4
	{
		BP[0][1],
		BP[0][2],
		BP[0][3],
		BP[1][2],
		BP[1][3],
		BP[2][3]
	};
}

nlohmann::ordered_map<std::string, VRStuff::ActionInfo> VRStuff::customActions{};
std::unordered_map<VR::ActionSet, std::vector<VRStuff::BindingInfo>> VRStuff::customDefaultBindings{};

extern "C"
{
	__declspec(dllexport) void addHapticEvent(VR::Controller controller, VR::Haptic* haptic)
	{
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return;
		if (!haptic) return;
		VRStuff::activeHaptics[controller].push_back(haptic);
	}
	__declspec(dllexport) void stopAllHaptics(VR::Controller controller)
	{
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return;
		for (auto& haptic : VRStuff::activeHaptics[controller])
		{
			if (haptic->destroy)
				delete haptic;
		}
		VRStuff::activeHaptics[controller].clear();
	}
	__declspec(dllexport) bool isHapticActive(VR::Controller controller, VR::Haptic* haptic)
	{
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return false;
		if (!haptic) return false;
		return std::find(VRStuff::activeHaptics[controller].cbegin(), VRStuff::activeHaptics[controller].cend(), haptic) != VRStuff::activeHaptics[controller].cend();
	}
	__declspec(dllexport) VR::ActionHandle addAction(
		const fdm::stl::string& modID,
		const fdm::stl::string& id,
		const fdm::stl::string& name,
		VR::ActionType type,
		VR::ActionSet set,
		VR::ActionRequirement requirement)
	{
		if (VRStuff::HMD) return 0;
		if (modID.empty()) return 0;
		if (!fdm::isModLoaded(modID)) return 0;
		if (id.empty()) return 0;
		if (name.empty()) return 0;
		if (type < 0 || type >= VR::ACTION_TYPE_COUNT) return 0;
		if (set < 0 || set >= VR::ACTION_SET_COUNT) return 0;
		if (requirement < 0 || requirement >= VR::ACTION_REQ_COUNT) return 0;

		std::string idNoSpaces = "";
		for (size_t i = 0; i < id.size(); ++i)
		{
			if ((id[i] >= '0' && id[i] <= '9') || (id[i] >= 'A' && id[i] <= 'Z') || (id[i] >= 'a' && id[i] <= 'z'))
			{
				idNoSpaces += id[i];
			}
		}

		std::string safeModID = "";
		for (size_t i = 0; i < modID.size(); ++i)
		{
			if ((modID[i] >= '0' && modID[i] <= '9') || (modID[i] >= 'A' && modID[i] <= 'Z') || (modID[i] >= 'a' && modID[i] <= 'z'))
			{
				safeModID += modID[i];
			}
		}

		std::string setName = "";
		switch (set)
		{
		case VR::ACTION_SET_UI:
			setName = "menu";
			break;
		default:
		case VR::ACTION_SET_INGAME:
			setName = "main";
			break;
		}

		std::string fullID = std::format("/actions/{}/in/{}{}", setName, modID, idNoSpaces);

		std::string fullName = std::format("[{}] {}", fdm::getModName(modID), name);

		VRStuff::customActions.emplace_back
		(
			fullID,
			VRStuff::ActionInfo
			{
				0,
				fullID,
				fullName,
				type,
				set,
				requirement
			}
		);

		return VRStuff::customActions.size();
	}
	__declspec(dllexport) bool addDefaultBindButton(
		VR::ActionHandle action,
		VR::Controller controller,
		VR::Button button,
		VR::ButtonInteraction interaction)
	{
		if (VRStuff::HMD) return false;
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return false;
		if (!action || action > VRStuff::customActions.size()) return false;
		if (button < 0 || button >= VR::BTN_COUNT) return false;
		if (interaction < 0 || interaction >= VR::BTN_INTERACTION_COUNT) return false;
		action -= 1;
		auto& info = (VRStuff::customActions.begin() + action)->second;
		if (info.type != VR::ACTION_BOOL) return false;
		
		std::string path = std::format("/user/hand/{}/input/", controller == VR::CONTROLLER_LEFT ? "left" : "right");
		switch (button)
		{
			case VR::BTN_A: // BTN_X
				if (controller == VR::CONTROLLER_RIGHT)
					path += "a";
				else
					path += "x";
				break;
			case VR::BTN_B: // BTN_Y
				if (controller == VR::CONTROLLER_RIGHT)
					path += "b";
				else
					path += "y";
				break;
			case VR::BTN_SYSTEM:
				path += "system";
				break;
			case VR::BTN_TRIGGER:
				path += "trigger";
				break;
			case VR::BTN_GRIP:
				path += "grip";
				break;
			case VR::BTN_JOYSTICK:
				path += "joystick";
				break;
		}

		std::string mode = "button";

		std::string input = "click";
		switch (interaction)
		{
			case VR::BTN_CLICK:
				input = "click";
				break;
			case VR::BTN_TOUCH:
				input = "touch";
				break;
			case VR::BTN_LONG:
				input = "long";
				break;
			case VR::BTN_HELD:
				input = "held";
				break;
			case VR::BTN_DOUBLE_PRESS:
				input = "double";
				break;
		}

		std::string output = info.id;

		VRStuff::BindingInfo bind
		{
			info.set,
			path,
			mode,
			nlohmann::json::object(),
			nlohmann::json
			{
				{
					input,
					{
						{ "output", output }
					}
				}
			}
		};

		VRStuff::customDefaultBindings[info.set].emplace_back(bind);
		return true;
	}
	__declspec(dllexport) bool addDefaultBindJoystick(VR::ActionHandle action, VR::Controller controller)
	{
		if (VRStuff::HMD) return false;
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return false;
		if (!action || action > VRStuff::customActions.size()) return false;
		action -= 1;
		auto& info = (VRStuff::customActions.begin() + action)->second;
		if (info.type != VR::ACTION_VEC2) return false;

		std::string path = std::format("/user/hand/{}/input/joystick", controller == VR::CONTROLLER_LEFT ? "left" : "right");
		
		std::string mode = "joystick";

		std::string input = "position";

		std::string output = info.id;

		VRStuff::BindingInfo bind
		{
			info.set,
			path,
			mode,
			nlohmann::json::object(),
			nlohmann::json
			{
				{
					input,
					{
						{ "output", output }
					}
				}
			}
		};

		VRStuff::customDefaultBindings[info.set].emplace_back(bind);
		return true;
	}
	__declspec(dllexport) bool addDefaultBindDPad(
		VR::ActionHandle actions[VR::DPAD_DIRECTION_COUNT],
		VR::Controller controller,
		float deadzone,
		float overlap)
	{
		if (VRStuff::HMD) return false;
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return false;
		if (!actions) return false;
		VR::ActionSet set = VR::ACTION_SET_INGAME;
		int count0 = 0;
		for (int i = 0; i < VR::DPAD_DIRECTION_COUNT; ++i)
		{
			if (!actions[i] || actions[i] > VRStuff::customActions.size()) { ++count0; continue; }
			auto& info = (VRStuff::customActions.begin() + (actions[i] - 1))->second;
			VR::ActionSet iSet = info.set;
			if (i > 0)
			{
				if (set != iSet)
				{
					return false;
				}
			}
			set = iSet;
			if (info.type != VR::ACTION_BOOL) return false;
		}
		if (count0 == VR::DPAD_DIRECTION_COUNT) return false;

		deadzone = glm::clamp(deadzone, 0.0f, 1.0f);
		overlap = glm::clamp(overlap, 0.0f, glm::pi<float>() * 0.5f);

		std::string path = std::format("/user/hand/{}/input/joystick", controller == VR::CONTROLLER_LEFT ? "left" : "right");

		std::string mode = "dpad";

		VRStuff::BindingInfo bind
		{
			set,
			path,
			mode,
			nlohmann::json
			{
				{"deadzone_pct", (int)glm::floor(deadzone * 100.0f)},
				{"overlap_pct", (int)glm::floor(overlap / (glm::pi<float>() * 0.5f) * 100.0f)},
				{"sub_mode", "touch"}
			},
			nlohmann::json::object()
		};
		if (actions[VR::DPAD_EAST] > 0 && actions[VR::DPAD_EAST] <= VRStuff::customActions.size())
		{
			bind.inputs["east"] =
			{
				{ "output", (VRStuff::customActions.begin() + (actions[VR::DPAD_EAST] - 1))->second.id }
			};
		}
		if (actions[VR::DPAD_NORTH] > 0 && actions[VR::DPAD_NORTH] <= VRStuff::customActions.size())
		{
			bind.inputs["north"] =
			{
				{ "output", (VRStuff::customActions.begin() + (actions[VR::DPAD_NORTH] - 1))->second.id }
			};
		}
		if (actions[VR::DPAD_SOUTH] > 0 && actions[VR::DPAD_SOUTH] <= VRStuff::customActions.size())
		{
			bind.inputs["south"] =
			{
				{ "output", (VRStuff::customActions.begin() + (actions[VR::DPAD_SOUTH] - 1))->second.id }
			};
		}
		if (actions[VR::DPAD_WEST] > 0 && actions[VR::DPAD_WEST] <= VRStuff::customActions.size())
		{
			bind.inputs["west"] =
			{
				{ "output", (VRStuff::customActions.begin() + (actions[VR::DPAD_WEST] - 1))->second.id }
			};
		}

		VRStuff::customDefaultBindings[set].emplace_back(bind);
		return true;
	}
	__declspec(dllexport) bool addDefaultBindTrigger(VR::ActionHandle action, VR::Controller controller, VR::Trigger trigger)
	{
		if (VRStuff::HMD) return false;
		if (!action || action > VRStuff::customActions.size()) return false;
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return false;
		if (trigger < 0 || trigger >= VR::TRIGGER_COUNT) return false;
		action -= 1;
		auto& info = (VRStuff::customActions.begin() + action)->second;
		if (info.type != VR::ACTION_FLOAT) return false;

		std::string path = std::format("/user/hand/{}/input/{}", controller == VR::CONTROLLER_LEFT ? "left" : "right", trigger == VR::TRIGGER ? "trigger" : "grip");

		std::string mode = "trigger";

		std::string input = "pull";

		std::string output = info.id;

		VRStuff::BindingInfo bind
		{
			info.set,
			path,
			mode,
			nlohmann::json::object(),
			nlohmann::json
			{
				{
					input,
					{
						{ "output", output }
					}
				}
			}
		};

		VRStuff::customDefaultBindings[info.set].emplace_back(bind);
		return true;
	}
	__declspec(dllexport) bool getInputBool(VR::ActionHandle action, bool* output)
	{
		if (!VRStuff::HMD) return false;
		if (!action || action > VRStuff::customActions.size()) return false;
		if (!output) return false;
		action -= 1;
		auto& info = (VRStuff::customActions.begin() + action)->second;
		if (!info.ovrHandle) return false;
		if (info.type != VR::ACTION_BOOL) return false;
		*output = VRStuff::getActionBool(info.ovrHandle);
		return true;
	}
	__declspec(dllexport) bool getInputFloat(VR::ActionHandle action, float* output)
	{
		if (!VRStuff::HMD) return false;
		if (!action || action > VRStuff::customActions.size()) return false;
		if (!output) return false;
		action -= 1;
		auto& info = (VRStuff::customActions.begin() + action)->second;
		if (!info.ovrHandle) return false;
		if (info.type != VR::ACTION_FLOAT) return false;
		*output = VRStuff::getActionFloat(info.ovrHandle);
		return true;
	}
	__declspec(dllexport) bool getInputVec2(VR::ActionHandle action, glm::vec2* output)
	{
		if (!VRStuff::HMD) return false;
		if (!action || action > VRStuff::customActions.size()) return false;
		if (!output) return false;
		action -= 1;
		auto& info = (VRStuff::customActions.begin() + action)->second;
		if (!info.ovrHandle) return false;
		if (info.type != VR::ACTION_VEC2) return false;
		*output = VRStuff::getActionVec2(info.ovrHandle);
		return true;
	}
	__declspec(dllexport) bool isControllerConnected(VR::Controller controller)
	{
		if (!VRStuff::HMD) return false;
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return false;
		return VRStuff::HMD->IsTrackedDeviceConnected(controller == VR::CONTROLLER_LEFT ? VRStuff::lCInd : VRStuff::rCInd);
	}
	__declspec(dllexport) void getHead(glm::mat4* result)
	{
		if (!VRStuff::HMD) return;
		if (!result) return;
		*result = ToMat4(VRStuff::orientation3D);
	}
	__declspec(dllexport) void getHead4D(fdm::m4::Mat5* result)
	{
		if (!VRStuff::HMD) return;
		if (!result) return;
		*result = VRStuff::orientation;
	}
	__declspec(dllexport) void getController(VR::Controller controller, glm::mat4* result)
	{
		if (!VRStuff::HMD) return;
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return;
		if (!result) return;
		if (!::isControllerConnected(controller)) return;

		glm::mat4 m = VRStuff::trackedPoses[controller == VR::CONTROLLER_LEFT ? VRStuff::lCInd : VRStuff::rCInd];
		m = glm::rotate(glm::identity<glm::mat4>(), VRStuff::controllerYZAngle, { 1, 0, 0 }) * m;
		glm::mat3 dir = glm::inverse(glm::mat3
			{
				m[0][0],m[1][0],m[2][0],
				m[0][1],m[1][1],m[2][1],
				m[0][2],m[1][2],m[2][2]
			});

		Mat5 mat{ 1 };

		mat[0][0] = -dir[0][0];
		mat[0][1] = dir[1][0];
		mat[0][2] = -dir[2][0];

		mat[1][0] = dir[0][1];
		mat[1][1] = -dir[1][1];
		mat[1][2] = dir[2][1];

		mat[2][0] = -dir[0][2];
		mat[2][1] = dir[1][2];
		mat[2][2] = -dir[2][2];

		mat[4][0] = -m[0][3];
		mat[4][1] = m[1][3];
		mat[4][2] = -m[2][3];

		*result = ToMat4(mat);
	}
	__declspec(dllexport) void getController4D(VR::Controller controller, fdm::m4::Mat5* result)
	{
		if (!VRStuff::HMD) return;
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return;
		if (!result) return;
		if (!::isControllerConnected(controller)) return;

		glm::mat4 m = VRStuff::trackedPoses[controller == VR::CONTROLLER_LEFT ? VRStuff::lCInd : VRStuff::rCInd];
		m = glm::rotate(glm::identity<glm::mat4>(), VRStuff::controllerYZAngle, { 1, 0, 0 }) * m;
		glm::mat3 dir = glm::inverse(glm::mat3
			{
				m[0][0],m[1][0],m[2][0],
				m[0][1],m[1][1],m[2][1],
				m[0][2],m[1][2],m[2][2]
			});

		Mat5 mat{ 1 };

		mat[0][0] = -dir[0][0];
		mat[0][1] = dir[1][0];
		mat[0][2] = -dir[2][0];

		mat[1][0] = dir[0][1];
		mat[1][1] = -dir[1][1];
		mat[1][2] = dir[2][1];

		mat[2][0] = -dir[0][2];
		mat[2][1] = dir[1][2];
		mat[2][2] = -dir[2][2];

		mat[4][0] = -m[0][3];
		mat[4][1] = m[1][3];
		mat[4][2] = -m[2][3];

		mat = lastOrientationMult * mat;
		*result = mat;
	}
	__declspec(dllexport) void getEyePos(VR::Eye eye, glm::vec3* result)
	{
		if (!VRStuff::HMD) return;
		if (eye < 0 || eye >= VR::EYE_COUNT) return;
		if (!result) return;
		switch (eye)
		{
		case VR::EYE_LEFT:
			*result = VRStuff::eyeLP3D;
			break;
		case VR::EYE_RIGHT:
			*result = VRStuff::eyeRP3D;
			break;
		case VR::EYE_DESKTOP:
			*result = VRStuff::hmd3DPos;
			break;
		}
	}
	__declspec(dllexport) void getEyePos4D(VR::Eye eye, glm::vec4* result)
	{
		if (!VRStuff::HMD) return;
		if (eye < 0 || eye >= VR::EYE_COUNT) return;
		if (!result) return;
		switch (eye)
		{
		case VR::EYE_LEFT:
			*result = VRStuff::eyeLP;
			break;
		case VR::EYE_RIGHT:
			*result = VRStuff::eyeRP;
			break;
		case VR::EYE_DESKTOP:
			*result = VRStuff::hmd4DPos;
			break;
		}
	}
	__declspec(dllexport) void getHeadPos(glm::vec3* result)
	{
		if (!VRStuff::HMD) return;
		if (!result) return;

		*result = VRStuff::hmd3DPos;
	}
	__declspec(dllexport) void getHeadPos4D(glm::vec4* result)
	{
		if (!VRStuff::HMD) return;
		if (!result) return;

		*result = VRStuff::hmd4DPos;
	}
	__declspec(dllexport) void getHeadVel(glm::vec3* result)
	{
		if (!VRStuff::HMD) return;
		if (!result) return;

		glm::vec3 vel = VRStuff::trackedVelocities[0];
		*result = glm::vec3(-vel.x, vel.y, -vel.z);
	}
	__declspec(dllexport) void getHeadVel4D(glm::vec4* result)
	{
		if (!VRStuff::HMD) return;
		if (!result) return;

		glm::vec3 vel = VRStuff::trackedVelocities[0];
		*result = lastOrientationMult * glm::vec4(-vel.x, vel.y, -vel.z, 0.0f);
	}
	__declspec(dllexport) void getControllerVel(VR::Controller controller, glm::vec3* result)
	{
		if (!VRStuff::HMD) return;
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return;
		if (!result) return;
		if (!::isControllerConnected(controller)) return;

		glm::vec3 vel = VRStuff::trackedVelocities[controller == VR::CONTROLLER_LEFT ? VRStuff::lCInd : VRStuff::rCInd];
		*result = glm::vec3(-vel.x, vel.y, -vel.z);
	}
	__declspec(dllexport) void getControllerVel4D(VR::Controller controller, glm::vec4* result)
	{
		if (!VRStuff::HMD) return;
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return;
		if (!result) return;
		if (!::isControllerConnected(controller)) return;

		glm::vec3 vel = VRStuff::trackedVelocities[controller == VR::CONTROLLER_LEFT ? VRStuff::lCInd : VRStuff::rCInd];
		*result = lastOrientationMult * glm::vec4(-vel.x, vel.y, -vel.z, 0.0f);
	}
	__declspec(dllexport) void getHeadAngVel(glm::vec3* result)
	{
		if (!VRStuff::HMD) return;
		if (!result) return;

		glm::vec3 angVel = VRStuff::trackedAngularVelocities[0];
		*result = angVel;
	}
	__declspec(dllexport) void getHeadAngVel4D(fdm::m4::BiVector4* result)
	{
		if (!VRStuff::HMD) return;
		if (!result) return;

		glm::vec3 angVel = VRStuff::trackedAngularVelocities[0];
		fdm::m4::BiVector4 b = fdm::m4::BiVector4{ angVel.z, angVel.y, 0, angVel.x, 0, 0 };
		*result = VRStuff::rotateBiVec4(b, lastOrientationMult);
	}
	__declspec(dllexport) void getControllerAngVel(VR::Controller controller, glm::vec3* result)
	{
		if (!VRStuff::HMD) return;
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return;
		if (!result) return;
		if (!::isControllerConnected(controller)) return;

		glm::vec3 angVel = VRStuff::trackedAngularVelocities[controller == VR::CONTROLLER_LEFT ? VRStuff::lCInd : VRStuff::rCInd];
		*result = angVel;
	}
	__declspec(dllexport) void getControllerAngVel4D(VR::Controller controller, fdm::m4::BiVector4* result)
	{
		if (!VRStuff::HMD) return;
		if (controller < 0 || controller >= VR::CONTROLLER_COUNT) return;
		if (!result) return;
		if (!::isControllerConnected(controller)) return;

		glm::vec3 angVel = VRStuff::trackedAngularVelocities[controller == VR::CONTROLLER_LEFT ? VRStuff::lCInd : VRStuff::rCInd];
		fdm::m4::BiVector4 b = fdm::m4::BiVector4{ angVel.z, angVel.y, 0, angVel.x, 0, 0 };
		*result = VRStuff::rotateBiVec4(b, lastOrientationMult);
	}
	__declspec(dllexport) bool inVR()
	{
#ifndef NONVR
		return VRStuff::HMD;
#else
		return false;
#endif
	}
	__declspec(dllexport) float getControllerYZAngleAdjustment()
	{
		if (!VRStuff::HMD) return 0.0f;
		return VRStuff::controllerYZAngle;
	}
	__declspec(dllexport) void getProjectionMatrix(VR::Eye eye, glm::mat4* result)
	{
		if (!VRStuff::HMD) return;
		if (eye < 0 || eye >= VR::EYE_COUNT) return;
		switch (eye)
		{
		case VR::EYE_LEFT:
			*result = VRStuff::prjL;
			break;
		case VR::EYE_RIGHT:
			*result = VRStuff::prjR;
			break;
		case VR::EYE_DESKTOP:
			*result = glm::perspective(
				glm::radians((float)fdm::StateSettings::instanceObj.currentFOV + 30),
				1280.0f / 720.0f,
				Player::Z_NEAR,
				Player::Z_FAR);
			break;
		}
	}
	__declspec(dllexport) void getViewMatrix(VR::Eye eye, glm::mat4* result)
	{
		if (!VRStuff::HMD) return;
		if (eye < 0 || eye >= VR::EYE_COUNT) return;
		bool use3DOrientation = VRStuff::use3DOrientation;
		VRStuff::VREye curEye = VRStuff::curEye;
		VRStuff::use3DOrientation = true;
		VRStuff::curEye = (VRStuff::VREye)eye;
		*result = ToMat4(m4::createCamera({ 0, 0, 0, 0 }, { 0, 0, 1, 0 }, { 0, 1, 0, 0 }, { 1, 0, 0, 0 }, { 0, 0, 0, 1 }));
		VRStuff::use3DOrientation = use3DOrientation;
		VRStuff::curEye = curEye;
	}
	__declspec(dllexport) void getViewMatrix4D(VR::Eye eye, fdm::m4::Mat5* result)
	{
		if (!VRStuff::HMD) return;
		if (eye < 0 || eye >= VR::EYE_COUNT) return;
		bool use3DOrientation = VRStuff::use3DOrientation;
		VRStuff::VREye curEye = VRStuff::curEye;
		VRStuff::use3DOrientation = false;
		VRStuff::curEye = (VRStuff::VREye)eye;
		*result = m4::createCamera({ 0, 0, 0, 0 }, { 0, 0, 1, 0 }, { 0, 1, 0, 0 }, { 1, 0, 0, 0 }, { 0, 0, 0, 1 });
		VRStuff::use3DOrientation = use3DOrientation;
		VRStuff::curEye = curEye;
	}
	__declspec(dllexport) void vrStateRender(fdm::StateManager& s,
		const std::function<void(fdm::StateManager& s)>& vrRender,
		const std::function<void(fdm::StateManager& s)>& uiRender)
	{
		if (!inVR()) return;

		VRStuff::renderingUI = false;

		if (!VRStuff::uiFB.FBO)
			VRStuff::uiFB = Framebuffer3D(WIDTH_UI, HEIGHT_UI, true);

		static const std::function<void(const std::function<void(fdm::StateManager& s)>&, fdm::StateManager&)> vrRenderFull =
			[](const std::function<void(fdm::StateManager& s)>& vrRender, fdm::StateManager& s)
			{
				vrRender(s);
				VRStuff::use3DOrientation = false;
				glDepthMask(GL_TRUE);
				glEnable(GL_DEPTH_TEST);
				VRStuff::renderUIPlane();
				drawHands();
				glDisable(GL_DEPTH_TEST);
				glDepthMask(GL_TRUE);
			};

		VRStuff::updateHMDMatrixPose({ 0,0,0,1 });

		// left eye
		VRStuff::eyeL.use();
		glViewport(0, 0, VRStuff::eyeW, VRStuff::eyeH);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		VRStuff::curEye = VRStuff::VEye_Left;
		VRStuff::updateProjection(VRStuff::eyeW, VRStuff::eyeH);

		vrRenderFull(vrRender, s);

		// right eye
		VRStuff::eyeR.use();
		glViewport(0, 0, VRStuff::eyeW, VRStuff::eyeH);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		VRStuff::curEye = VRStuff::VEye_Right;
		VRStuff::updateProjection(VRStuff::eyeW, VRStuff::eyeH);

		vrRenderFull(vrRender, s);

		// send textures to OpenVR (aka SteamVR)
		VRStuff::eyeL.drawEye(Eye_Left);
		VRStuff::eyeR.drawEye(Eye_Right);

		VRCompositor()->PostPresentHandoff();

		// desktop view
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (VRStuff::desktopView)
		{
			VRStuff::curEye = VRStuff::VEye_PC;
			int width, height;
			glfwGetWindowSize(s.window, &width, &height);
			glViewport(0, 0, width, height);
			VRStuff::updateProjection(width, height);

			vrRenderFull(vrRender, s);
		}

		// render ui into a texture
		VRStuff::renderingUI = true;
		glm::mat4 proj2D = glm::ortho(0.f, (float)WIDTH_UI, (float)HEIGHT_UI, 0.f, -1.f, 1.f);
		VRStuff::updateProjection2D(proj2D);
		VRStuff::uiFB.use();
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, WIDTH_UI, HEIGHT_UI);

		uiRender(s);

		VRStuff::pointCursor();
	}
	__declspec(dllexport) void renderPointer()
	{
		static TexRenderer cursorRenderer{ ResourceManager::get("crosshair.png"), ShaderManager::get("tex2DShader") };
		if (!cursorRenderer.VAO)
		{
			cursorRenderer.init();
		}
		
		double xposa, yposa;
		glfwGetCursorPos(VRStuff::stateManager->window, &xposa, &yposa);

		cursorRenderer.setPos(xposa - 8, yposa - 8, 16, 16);
		glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ZERO); // invert blending
		cursorRenderer.render();
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // reset to normal blending
	}
}
