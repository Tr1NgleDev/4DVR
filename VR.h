#pragma once

#include "matConverts.h"
#include "Constants.h"
#include "VRTex.h"

#include <openvr.h>

#include "Framebuffer3D.h"

#include "4DVR.h"

namespace VRStuff
{
	enum VREye
	{
		VEye_Left = 0,
		VEye_Right = 1,
		VEye_PC = 2
	};

	extern vr::IVRSystem* HMD;
	extern vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];

	extern VRTex eyeL;
	extern VRTex eyeR;

	extern unsigned int eyeW, eyeH;

	extern VREye curEye;

	extern glm::mat4 prjL;
	extern glm::mat4 prjR;
	extern glm::mat4 vEyeL;
	extern glm::mat4 vEyeR;
	extern glm::mat4 trackedPoses[vr::k_unMaxTrackedDeviceCount];
	extern glm::mat4 controllerTransforms[2][2]; // 0 - left; 1 - right. 0 - tip; 1 - handgrip
	extern float controllerYZAngle;
	extern glm::vec3 trackedVelocities[vr::k_unMaxTrackedDeviceCount];
	extern glm::vec3 trackedAngularVelocities[vr::k_unMaxTrackedDeviceCount];
	extern glm::mat4 hmdPose;
	extern glm::mat3 hmdDir;
	extern glm::mat3 cDir;

	extern glm::vec4 eyeLP;
	extern glm::vec4 eyeRP;

	extern glm::vec4 eyeLPN;
	extern glm::vec4 eyeRPN;

	extern glm::vec4 eyeLP3D;
	extern glm::vec4 eyeRP3D;

	extern glm::vec4 eyeLPN3D;
	extern glm::vec4 eyeRPN3D;

	extern vr::TrackedDeviceIndex_t lCInd;
	extern vr::TrackedDeviceIndex_t rCInd;
	extern vr::VRInputValueHandle_t cHandles[2];

	extern bool wOffset;
	extern float eyeWGap;
	extern bool desktopView;

	extern Mat5 orientation;
	extern Mat5 orientation3D;
	extern glm::vec3 hmd3DPos;
	extern glm::vec4 hmd4DPos;
	extern bool keepXZCameraPosition;
	extern glm::vec4 lastHMDPos;
	extern bool use3DOrientation;

	extern MeshRenderer handRenderer;
	extern MeshRenderer crosshairRenderer;
	extern const Shader* handShader;
	extern const Shader* crosshairShader;
	extern Mat5 lastCamera;
	extern glm::vec4 lastEye;

	extern Framebuffer3D uiFB;

	extern StateManager* stateManager;

	extern bool takeAScreenshot;
	extern double takeAScreenshotTime;

	extern std::string configPath;
	extern std::string infoPath;
	extern std::unordered_map<std::string, std::string> modeDLLs;
	extern bool showDialogSetting;
	extern float heightRatio;

	extern bool renderingUI;

	extern gui::TextInput* activeTextInput;
	extern bool pointingCursor;
	struct _Actions
	{
		//struct
		//{
			vr::VRActionSetHandle_t MainSet = 0;

			vr::VRActionHandle_t Movement = 0;
			vr::VRActionHandle_t Look4D = 0;
			vr::VRActionHandle_t Use = 0;
			vr::VRActionHandle_t Break = 0;
			vr::VRActionHandle_t Crouch = 0;
			vr::VRActionHandle_t Sprint = 0;
			vr::VRActionHandle_t Jump = 0;
			vr::VRActionHandle_t Inventory = 0;
			vr::VRActionHandle_t Pause = 0;
			vr::VRActionHandle_t ItemUp = 0;
			vr::VRActionHandle_t ItemDown = 0;
			vr::VRActionHandle_t StrafeW = 0;
			vr::VRActionHandle_t StrafeWP = 0;
			vr::VRActionHandle_t QuickActions = 0;

			vr::VRActionHandle_t LeftHaptics = 0;
			vr::VRActionHandle_t RightHaptics = 0;
		//} Main;

		//struct
		//{
			vr::VRActionSetHandle_t MenuSet = 0;

			vr::VRActionHandle_t Press = 0;
			vr::VRActionHandle_t Action = 0;
			vr::VRActionHandle_t Scroll = 0;
		//} Menu;

		//struct
		//{
			vr::VRActionSetHandle_t TutorialSet = 0;

			vr::VRActionHandle_t Move = 0;
			vr::VRActionHandle_t DragRotate = 0;
			vr::VRActionHandle_t ForgMoveLeft = 0;
			vr::VRActionHandle_t ForgMoveRight = 0;
			vr::VRActionHandle_t ForgRotateLeft = 0;
			vr::VRActionHandle_t ForgRotateRight = 0;
			vr::VRActionHandle_t ForgJump = 0;
		//} Tutorial;
	};
	extern _Actions Actions;

	struct ActionInfo
	{
		vr::VRActionHandle_t ovrHandle = 0;
		std::string id = "";
		std::string name = "";
		VR::ActionType type = VR::ACTION_BOOL;
		VR::ActionSet set = VR::ACTION_SET_INGAME;
		VR::ActionRequirement requirement = VR::ACTION_OPTIONAL;
	};
	struct BindingInfo
	{
		VR::ActionSet set = VR::ACTION_SET_INGAME;
		std::string path = "";
		std::string mode = "";
		nlohmann::json parameters;
		nlohmann::json inputs;
	};
	extern nlohmann::ordered_map<std::string, ActionInfo> customActions;
	// set -> path,mode -> info list
	extern std::unordered_map<VR::ActionSet, std::vector<BindingInfo>> customDefaultBindings;

	struct KeyBindActionState
	{
		bool lastState = false;
		bool curState = false;
		std::string ns = "";
		std::string name = "";
	};
	extern std::unordered_map<VR::ActionHandle, KeyBindActionState> keybindsActions;

	std::string GetTrackedDeviceString(TrackedDeviceIndex_t unDevice, TrackedDeviceProperty prop, TrackedPropertyError* peError = nullptr);

	std::string getActionName(vr::VRActionHandle_t handle);
	std::string EVRInputErrorToString(vr::EVRInputError error);

	bool getActionBool(vr::VRActionHandle_t action);
	float getActionFloat(vr::VRActionHandle_t action);
	glm::vec2 getActionVec2(vr::VRActionHandle_t action);
	glm::vec3 getActionVec3(vr::VRActionHandle_t action);
	void shutdownVR();
	bool initVR();
	void initKeyBindsActions();
	glm::mat4 composeProjection(float fLeft, float fRight, float fTop, float fBottom, float zNear, float zFar);
	void processEvent(const vr::VREvent_t& event);
	void handleEvents();
	void updateHMDMatrixPose(const glm::vec4& over, Mat5* orientationMult = nullptr);

	extern glm::vec3 planePosition;
	extern glm::vec3 planeScale;
	extern glm::vec3 planeNormal;
	extern glm::vec3 planeUp;
	inline glm::vec3 planeOffset() { return glm::vec3{ -(float)WIDTH_UI / (float)WIDTH_UI, -(float)HEIGHT_UI / (float)WIDTH_UI, 0.f } * planeScale * 0.5f; }
	inline glm::vec3 planeCenter() { return planePosition - planeOffset(); };
	inline glm::vec3 menuPlanePosition = glm::vec3{ 0.f, 1.25f, 1.75f } + planeOffset();
	void renderUIPlane();
	bool pointCursor(bool renderCursor = true, bool forceFocus = false, bool leftHand = false);

	extern std::array<std::vector<VR::Haptic*>, 2> activeHaptics;
	void eraseAllHaptics();

	template <typename T>
	inline bool aabb(T aX, T aY, T aW, T aH, T bX, T bY, T bW, T bH)
	{
		return aX < bX + bW && aX + aW > bX && aY < bY + bH && aY + aH > bY;
	}

	struct VRRay
	{
		glm::vec3 origin;
		glm::vec3 dir;
	};

	bool rayPlaneIntersection(const VRRay& ray, const glm::vec3& planeNormal, const glm::vec3& planeUp, const glm::vec3& planeCenter, glm::vec2& intersection);

	inline float deltaRatio(float ratio, double dt, double targetDelta = 1.0 / 100.0)
	{
		const double rDelta = dt / (1.0 / (1.0 / targetDelta));
		const double s = 1.0 - ratio;

		return (float)(1.0 - pow(s, rDelta));
	}

	inline float lerp(float a, float b, float ratio, bool clampRatio = true)
	{
		if (clampRatio)
			ratio = glm::clamp(ratio, 0.f, 1.f);
		return a + (b - a) * ratio;
	}
	inline glm::vec3 lerp(const glm::vec3& a, const glm::vec3& b, float ratio, bool clampRatio = true)
	{
		if (clampRatio)
			ratio = glm::clamp(ratio, 0.f, 1.f);
		return a + (b - a) * ratio;
	}
	inline glm::vec4 lerp(const glm::vec4& a, const glm::vec4& b, float ratio, bool clampRatio = true)
	{
		if (clampRatio)
			ratio = glm::clamp(ratio, 0.f, 1.f);
		return a + (b - a) * ratio;
	}

	inline float ilerp(float a, float b, float ratio, float dt, bool clampRatio = true)
	{
		return lerp(a, b, deltaRatio(ratio, dt), clampRatio);
	}
	inline glm::vec3 ilerp(const glm::vec3& a, const glm::vec3& b, float ratio, float dt, bool clampRatio = true)
	{
		return lerp(a, b, deltaRatio(ratio, dt), clampRatio);
	}
	inline glm::vec4 ilerp(const glm::vec4& a, const glm::vec4& b, float ratio, float dt, bool clampRatio = true)
	{
		return lerp(a, b, deltaRatio(ratio, dt), clampRatio);
	}

	void dumpConfig(bool justRestarted = false, const std::string& vrMode = VR_MODE);

	fdm::m4::BiVector4 rotateBiVec4(const fdm::m4::BiVector4& b, const fdm::m4::Mat5& m);

	void updateProjection(const glm::ivec2& size, const glm::ivec2& translate2D);
	void updateProjection(int width, int height);
	void updateProjection2D(const glm::mat4& proj);
}

extern "C"
{
	__declspec(dllexport) VR::ActionHandle addAction(
		const fdm::stl::string& modID,
		const fdm::stl::string& id,
		const fdm::stl::string& name,
		VR::ActionType type,
		VR::ActionSet set,
		VR::ActionRequirement requirement = VR::ACTION_OPTIONAL);
	__declspec(dllexport) bool addDefaultBindButton(
		VR::ActionHandle action,
		VR::Controller controller,
		VR::Button button,
		VR::ButtonInteraction interaction = VR::BTN_CLICK);
	__declspec(dllexport) bool addDefaultBindJoystick(VR::ActionHandle action, VR::Controller controller);
	__declspec(dllexport) bool addDefaultBindDPad(
		VR::ActionHandle actions[VR::DPAD_DIRECTION_COUNT],
		VR::Controller controller,
		float deadzone = 0.15f,
		float overlap = 0.0f);
	__declspec(dllexport) bool addDefaultBindTrigger(VR::ActionHandle action, VR::Controller controller, VR::Trigger trigger = VR::TRIGGER);
	__declspec(dllexport) bool getInputBool(VR::ActionHandle action, bool* output);
	__declspec(dllexport) bool getInputFloat(VR::ActionHandle action, float* output);
	__declspec(dllexport) bool getInputVec2(VR::ActionHandle action, glm::vec2* output);
	__declspec(dllexport) bool isControllerConnected(VR::Controller controller);
	__declspec(dllexport) void getHead(glm::mat4* result);
	__declspec(dllexport) void getHead4D(fdm::m4::Mat5* result);
	__declspec(dllexport) void getController(VR::Controller controller, glm::mat4* result);
	__declspec(dllexport) void getController4D(VR::Controller controller, fdm::m4::Mat5* result);
	__declspec(dllexport) void getEyePos(VR::Eye eye, glm::vec3* result);
	__declspec(dllexport) void getEyePos4D(VR::Eye eye, glm::vec4* result);
	__declspec(dllexport) void getHeadPos(glm::vec3* result);
	__declspec(dllexport) void getHeadPos4D(glm::vec4* result);
	__declspec(dllexport) void getHeadVel(glm::vec3* result);
	__declspec(dllexport) void getHeadVel4D(glm::vec4* result);
	__declspec(dllexport) void getControllerVel(VR::Controller controller, glm::vec3* result);
	__declspec(dllexport) void getControllerVel4D(VR::Controller controller, glm::vec4* result);
	__declspec(dllexport) void getHeadAngVel(glm::vec3* result);
	__declspec(dllexport) void getHeadAngVel4D(fdm::m4::BiVector4* result);
	__declspec(dllexport) void getControllerAngVel(VR::Controller controller, glm::vec3* result);
	__declspec(dllexport) void getControllerAngVel4D(VR::Controller controller, fdm::m4::BiVector4* result);
	__declspec(dllexport) bool inVR();
	__declspec(dllexport) float getControllerYZAngleAdjustment();
	__declspec(dllexport) void getProjectionMatrix(VR::Eye eye, glm::mat4* result);
	__declspec(dllexport) void getViewMatrix(VR::Eye eye, glm::mat4* result);
	__declspec(dllexport) void getViewMatrix4D(VR::Eye eye, fdm::m4::Mat5* result);
	__declspec(dllexport) void vrStateRender(fdm::StateManager& s,
		const std::function<void(fdm::StateManager& s)>& vrRender,
		const std::function<void(fdm::StateManager& s)>& uiRender);
	__declspec(dllexport) void renderPointer();
}
