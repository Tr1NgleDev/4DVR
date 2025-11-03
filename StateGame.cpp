#define _CRT_SECURE_NO_WARNINGS

#include "matConverts.h"

#include "Constants.h"
#include "VR.h"

#include <4dm.h>
using namespace fdm; // 4dm.h

#include <openvr.h>
using namespace vr;

#include "VRTex.h"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/random.hpp>

#include "Player.h"

#include "FontRenderer3D.h"

using namespace VRStuff;

#ifndef NONVR
$hook(void, StateDeathScreen, render, StateManager& s)
{
	original(self, s);
	pointCursor();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
$hook(void, StatePause, render, StateManager& s)
{
	original(self, s);
	if (s.states.size() <= 2)
	{
		pointCursor();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}
/*
$hook(void, StateDeathScreen, updateProjection, const glm::ivec2& size, const glm::ivec2& translate2D)
{
	StateGame::instanceObj->updateProjection(eyeW, eyeH);

	StateGame::instanceObj->updateProjection2D(size, translate2D);
}
$hook(void, StatePause, updateProjection, const glm::ivec2& size, const glm::ivec2& translate2D)
{
	StateGame::instanceObj->updateProjection(eyeW, eyeH);

	StateGame::instanceObj->updateProjection2D(size, translate2D);
}
*/

$hook(void, StateGame, updateProjection, int width, int height)
{
	switch (curEye)
	{
	case VEye_Left:
		self->projection3D = prjL;
		break;
	case VEye_Right:
		self->projection3D = prjR;
		break;
	case VEye_PC:
		self->projection3D = glm::perspective(
			glm::radians(self->FOV),
			(float)width / (float)height,
			Player::Z_NEAR,
			Player::Z_FAR);
		break;
	}

	if (self->world)
		self->world->updateRenderFrustum(self->projection3D);

	self->projection2D = glm::ortho(0.f, (float)WIDTH_UI, (float)HEIGHT_UI, 0.f, -1.f, 1.f);

	glViewport(0, 0, width, height);

	// Update Shaders
	{
		glm::mat4 postProj{ 1 };
		if (curEye == VEye_PC)
			postProj = glm::ortho(0.f, (float)width, (float)height, 0.f, -1.f, 1.f);
		else
			postProj = glm::ortho(0.f, (float)eyeW, (float)eyeH, 0.f, -1.f, 1.f);

		self->renderFramebuffer.getShader()->use();
		glUniformMatrix4fv(glGetUniformLocation(self->renderFramebuffer.getShader()->id(), "P"), 1, GL_FALSE, &postProj[0][0]);

		self->tex2DShader->use();
		glUniformMatrix4fv(glGetUniformLocation(self->tex2DShader->id(), "P"), 1, GL_FALSE, &self->projection2D[0][0]);
		self->qr.shader->use();
		glUniformMatrix4fv(glGetUniformLocation(self->qr.shader->id(), "P"), 1, GL_FALSE, &self->projection2D[0][0]);
		self->font.shader->use();
		glUniformMatrix4fv(glGetUniformLocation(self->font.shader->id(), "P"), 1, GL_FALSE, &self->projection2D[0][0]);
		self->cloudShader->use();
		glUniformMatrix4fv(glGetUniformLocation(self->cloudShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);
		self->moonShader->use();
		glUniformMatrix4fv(glGetUniformLocation(self->moonShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);
		self->blockShader->use();
		glUniformMatrix4fv(glGetUniformLocation(self->blockShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);
		const Shader* skyboxShader = ShaderManager::get("skyboxShader");
		skyboxShader->use();
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* simplexColorShader = ShaderManager::get("tetColorShader");
		simplexColorShader->use();
		glUniformMatrix4fv(glGetUniformLocation(simplexColorShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* playerSkinShader = ShaderManager::get("playerSkinShader");
		playerSkinShader->use();
		glUniformMatrix4fv(glGetUniformLocation(playerSkinShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* targetBlockShader = ShaderManager::get("targetBlockShader");
		targetBlockShader->use();
		glUniformMatrix4fv(glGetUniformLocation(targetBlockShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* simplexSolidColorShader = ShaderManager::get("tetSolidColorShader");
		simplexSolidColorShader->use();
		glUniformMatrix4fv(glGetUniformLocation(simplexSolidColorShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* tetShader = ShaderManager::get("tetShader");
		tetShader->use();
		glUniformMatrix4fv(glGetUniformLocation(tetShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* simplexColorAlphaShader = ShaderManager::get("tetColorAlphaShader");
		simplexColorAlphaShader->use();
		glUniformMatrix4fv(glGetUniformLocation(simplexColorAlphaShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* simplexNormalShader = ShaderManager::get("tetNormalShader");
		simplexNormalShader->use();
		glUniformMatrix4fv(glGetUniformLocation(simplexNormalShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* simplexColorNormalShader = ShaderManager::get("tetColorNormalShader");
		simplexColorNormalShader->use();
		glUniformMatrix4fv(glGetUniformLocation(simplexColorNormalShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* simplexSolidColorNormalShader = ShaderManager::get("tetSolidColorNormalShader");
		simplexSolidColorNormalShader->use();
		glUniformMatrix4fv(glGetUniformLocation(simplexSolidColorNormalShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* tetSolidColorNormalCullShader = ShaderManager::get("tetSolidColorNormalCullShader");
		tetSolidColorNormalCullShader->use();
		glUniformMatrix4fv(glGetUniformLocation(tetSolidColorNormalCullShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* tetMetallicShader = ShaderManager::get("tetMetallicShader");
		tetMetallicShader->use();
		glUniformMatrix4fv(glGetUniformLocation(tetMetallicShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* wireframeShader = ShaderManager::get("wireframeShader");
		wireframeShader->use();
		glUniformMatrix4fv(glGetUniformLocation(wireframeShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* wireframeColorShader = ShaderManager::get("wireframeColorShader");
		wireframeColorShader->use();
		glUniformMatrix4fv(glGetUniformLocation(wireframeColorShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* wireframeGlassesShader = ShaderManager::get("wireframeGlassesShader");
		wireframeGlassesShader->use();
		glUniformMatrix4fv(glGetUniformLocation(wireframeGlassesShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* solidColor3DShader = ShaderManager::get("solidColor3DShader");
		solidColor3DShader->use();
		glUniformMatrix4fv(glGetUniformLocation(solidColor3DShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* blockCrossSectionAreaShader = ShaderManager::get("blockCrossSectionAreaShader");
		blockCrossSectionAreaShader->use();
		glUniformMatrix4fv(glGetUniformLocation(blockCrossSectionAreaShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* blockCrossSectionEdgesShader = ShaderManager::get("blockCrossSectionEdgesShader");
		blockCrossSectionEdgesShader->use();
		glUniformMatrix4fv(glGetUniformLocation(blockCrossSectionEdgesShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* blockNormalShader = ShaderManager::get("blockNormalShader");
		blockNormalShader->use();
		glUniformMatrix4fv(glGetUniformLocation(blockNormalShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* blockNormalCullShader = ShaderManager::get("blockNormalCullShader");
		blockNormalCullShader->use();
		glUniformMatrix4fv(glGetUniformLocation(blockNormalCullShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* butterflyShader = ShaderManager::get("butterflyShader");
		butterflyShader->use();
		glUniformMatrix4fv(glGetUniformLocation(butterflyShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* tex2Din3DShader = ShaderManager::get("4dvr_tex2Din3DShader");
		tex2Din3DShader->use();
		glUniformMatrix4fv(glGetUniformLocation(tex2Din3DShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* vr_textShader = ShaderManager::get("4dvr_textShader");
		vr_textShader->use();
		glUniformMatrix4fv(glGetUniformLocation(vr_textShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);
	}

	original(self, width, height);
}

static gui::Interface qaUI;
static std::vector<gui::Element*> qaEls;
static std::vector<std::pair<std::string, std::string>*> qaPairsToRemove;

#endif
$hook(void, StateGame, update, StateManager& s, double dt)
{
	original(self, s, dt);
	
	if (takeAScreenshot && glfwGetTime() - takeAScreenshotTime > 1.0)
	{
		self->keyInput(s, GLFW_KEY_F2, 0, GLFW_PRESS, 0);

		takeAScreenshot = false;
	}


#ifndef NONVR
	handleEvents();

	static bool prevQuickActionsDown = false;
	bool quickActionsDown = getActionBool(Actions.QuickActions);
	if (quickActionsDown && !prevQuickActionsDown && s.states.back() == &StateGame::instanceObj && !self->chatOpen && !self->player.inventoryManager.isOpen() && !quickActionsOpen)
	{
		self->player.targetDamage = 0.f;
		self->player.keys.rightMouseDown = false;
		self->player.keys.leftMouseDown = false;
		self->world->localPlayerEvent(&self->player, Packet::C_HITTING_STOP, NULL, NULL);
		self->world->localPlayerEvent(&self->player, Packet::C_ACTION_STOP, NULL, NULL);

		quickActionsOpen = true;
	}
	else if (!quickActionsDown && prevQuickActionsDown && quickActionsOpen)
	{
		qaUI.mouseButtonInput(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
		qaUI.mouseButtonInput(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
		quickActionsOpen = false;
	}
	prevQuickActionsDown = quickActionsDown;
#endif
}

#ifndef NONVR

static Framebuffer pcFramebuffer{ };
FontRenderer3D font3D;

inline static constexpr uint32_t FXLIB_FB_MAGIC_NUMBER = 1529352098;

$hook(void, StateGame, render, StateManager& s)
{
	renderingUI = false;
	VRStuff::curEye = VEye_PC;
	int w, h;
	glfwGetFramebufferSize(s.window, &w, &h);

	if (desktopView && (pcFramebuffer.tex2D.width != w || pcFramebuffer.tex2D.height != h))
	{
		pcFramebuffer.texRenderer.texture = &pcFramebuffer.tex2D;
		pcFramebuffer.setShader(ShaderManager::get("postShader"));
		if (fdm::isModLoaded("!tr1ngledev.fxlib"))
			pcFramebuffer.tex2D.target = FXLIB_FB_MAGIC_NUMBER; // fxlib support
		pcFramebuffer.init(w, h, false);
	}
	if (self->renderFramebuffer.tex2D.width != eyeW || self->renderFramebuffer.tex2D.height != eyeH)
	{
		self->renderFramebuffer.texRenderer.texture = &pcFramebuffer.tex2D;
		self->renderFramebuffer.setShader(ShaderManager::get("postShader"));
		if (fdm::isModLoaded("!tr1ngledev.fxlib"))
			self->renderFramebuffer.tex2D.target = FXLIB_FB_MAGIC_NUMBER; // fxlib support
		self->renderFramebuffer.init(w, h, false);
	}

	double time = glfwGetTime();

	if (!self->world)
		return;

	//glfwSetWindowSize(s.window, WIDTH_UI, HEIGHT_UI);
	self->renderFramebuffer.getShader()->use();
	glUniform1f(glGetUniformLocation(self->renderFramebuffer.getShader()->id(), "time"), self->time);

	static bool prevDisplayUIOnHand = false;
	bool displayUIOnHand = !(self->chatOpen || self->player.inventoryManager.isOpen() || s.states.back() != self || quickActionsOpen);

	if (prevDisplayUIOnHand && !displayUIOnHand && planeScale.x != 2.f && planeScale.x != 1.5f)
	{
		glm::vec3 pl, pd;
		glm::vec3 planePos = glm::vec3{ 0.f, 1.25f, 0.f };
		planeScale = glm::vec3{ 2.f, 2.f, 1.f };
		if (!quickActionsOpen || !HMD->IsTrackedDeviceConnected(lCInd))
		{
			pl = -glm::normalize(glm::cross(*(glm::vec3*)orientation3D[2], { 0,1,0 }));
			pd = glm::normalize(glm::cross(pl, { 0,1,0 }));
		}
		else
		{
			glm::mat4 controller = trackedPoses[lCInd];
			controller = glm::rotate(glm::identity<glm::mat4>(), controllerYZAngle, { 1, 0, 0 }) * controller;
			controller = glm::transpose(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-0.01f, 0.01f, 0.04f))) * controller;
			glm::mat3 dir = glm::inverse(glm::mat3
				{
					controller[0][0],controller[1][0],controller[2][0],
					controller[0][1],controller[1][1],controller[2][1],
					controller[0][2],controller[1][2],controller[2][2]
				});
			pl = -glm::normalize(glm::cross(glm::vec3{ dir[0][1],-dir[1][1],dir[2][1] }, {0,1,0}));
			pd = glm::normalize(glm::cross(pl, { 0,1,0 }));

			planePos = glm::vec3{ -controller[0][3], controller[1][3], -controller[2][3] } - pd * 1.25f;
			planeScale = glm::vec3{ 1.5f, 1.5f, 1.f };
		}
		planePosition = pd * 1.75f + planePos + planeOffset();
		planeNormal = -pd;
		planeUp = { 0,1,0 };
	}

	prevDisplayUIOnHand = displayUIOnHand;

	glm::vec4 textPos = glm::vec4{ 3.5f, 10.0f, 3.5f, 3.5f };
	if (HMD)
	{
		keepXZCameraPosition = true;
		//keepXZCameraPosition = !self->player.keepOnEdge;

		// update orientation
		updateHMDMatrixPose({0,0,0,1}, &self->player.orientation);
		self->player.left = *(glm::vec4*)orientation[0];
		self->player.up = *(glm::vec4*)orientation[1];
		self->player.forward = *(glm::vec4*)orientation[2];
		self->player.over = *(glm::vec4*)orientation[3];
		self->player.cameraPos = self->player.pos;

		font3D.centered = false;
		font3D.fontSize = 1;

		// body movement to player movement
		static double lastTime = time - 0.01;
		static bool keepOnEdge = false;
		double dt = time - lastTime;
		{
			glm::vec4 diff = hmd4DPos - lastHMDPos;
			if (!keepOnEdge)
			{
				glm::vec4 vel = self->player.vel;
				self->player.vel = diff / (float)dt;
				self->player.updatePos(self->world.get(), dt);
				self->player.vel = vel;
				lastHMDPos = hmd4DPos;
			}
			else
			{
				self->player.cameraPos = self->player.pos + diff;
			}
		}
		lastTime = time;
		keepOnEdge = self->player.keepOnEdge;
		
		self->player.cameraPos += self->player.over * 0.001f;

		/*
		// body movement to player movement
		static bool lastKeepOnEdge = false;
		if (self->player.keepOnEdge)
		{
			self->player.cameraPos = self->player.pos + (hmd4DPos - lastHMDPos);
		}
		lastKeepOnEdge = self->player.keepOnEdge;
		*/
		glm::vec3 forward3D = *(glm::vec3*)orientation3D[2];
		glm::vec3 up3D = *(glm::vec3*)orientation3D[1];

		glm::mat4 MV = glm::lookAtRH(glm::vec3{ 0,0,0 }, forward3D, up3D);
		const Shader* skyboxShader = ShaderManager::get("skyboxShader");
		skyboxShader->use();
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader->id(), "MV"), 1, GL_FALSE, &MV[0][0]);

		glClearColor(0.f, 0.f, 0.f, 1.f);

		// left eye
		curEye = VEye_Left;
		self->updateProjection(eyeW, eyeH);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		original(self, s);
		glEnable(GL_DEPTH_TEST);
		drawHands(self);
		if (!displayUIOnHand)
			glDisable(GL_DEPTH_TEST);
		else if (HMD->IsTrackedDeviceConnected(lCInd))// set the position only once per frame because why would you do it multiple times its the same bruh
		{
			glm::mat4 controllerL = trackedPoses[lCInd];
			controllerL = glm::rotate(glm::identity<glm::mat4>(), controllerYZAngle, { 1, 0, 0 }) * controllerL;
			glm::mat3 dirL = glm::inverse(glm::mat3
				{
					controllerL[0][0],controllerL[1][0],controllerL[2][0],
					controllerL[0][1],controllerL[1][1],controllerL[2][1],
					controllerL[0][2],controllerL[1][2],controllerL[2][2]
				});
			planeScale = glm::vec3{ 0.4f, 0.4f, 1.f };
			planeUp = { -dirL[0][2], dirL[1][2], -dirL[2][2] };
			planeNormal = { dirL[0][0], -dirL[1][0], dirL[2][0] };

			glm::vec3 planeLeft = glm::cross(planeUp, planeNormal);

			planePosition = glm::vec3{ -controllerL[0][3], controllerL[1][3], -controllerL[2][3] } + planeOffset() +
				-0.012f * planeLeft +
				0.142f * planeUp +
				0.061f * planeNormal;
		}
		renderUIPlane();
		glDisable(GL_DEPTH_TEST);
		eyeL.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		self->renderFramebuffer.getShader()->use();
		glUniform1i(glGetUniformLocation(self->renderFramebuffer.getShader()->id(), "eye"), 0);
		self->renderFramebuffer._render();
		glEnable(GL_DEPTH_TEST);

		// right eye
		curEye = VEye_Right;
		self->updateProjection(eyeW, eyeH);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		original(self, s);
		glEnable(GL_DEPTH_TEST);
		drawHands(self);
		if (!displayUIOnHand)
			glDisable(GL_DEPTH_TEST);
		renderUIPlane();
		glDisable(GL_DEPTH_TEST);
		eyeR.use();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		self->renderFramebuffer.getShader()->use();
		glUniform1i(glGetUniformLocation(self->renderFramebuffer.getShader()->id(), "eye"), 1);
		self->renderFramebuffer._render();
		glEnable(GL_DEPTH_TEST);

		// send textures to OpenVR (aka SteamVR)
		eyeL.drawEye(Eye_Left);
		eyeR.drawEye(Eye_Right);

		VRCompositor()->PostPresentHandoff();
	}

	// desktop view
	if (desktopView)
	{
		Framebuffer oldFB;
		std::memcpy(&oldFB, &self->renderFramebuffer, sizeof(Framebuffer));
		std::memcpy(&self->renderFramebuffer, &pcFramebuffer, sizeof(Framebuffer));

		curEye = VEye_PC;
		self->updateProjection(w, h);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		original(self, s);
		glViewport(0, 0, w, h);
		glEnable(GL_DEPTH_TEST);
		drawHands(self);
		if (!displayUIOnHand)
			glDisable(GL_DEPTH_TEST);
		renderUIPlane();
		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, w, h);
		self->renderFramebuffer.getShader()->use();
		glUniform1i(glGetUniformLocation(self->renderFramebuffer.getShader()->id(), "eye"), -1);
		self->renderFramebuffer._render();
		glEnable(GL_DEPTH_TEST);

		std::memcpy(&self->renderFramebuffer, &oldFB, sizeof(Framebuffer));
		std::memset(&oldFB, 0, sizeof(Framebuffer));
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// render ui into a texture
	renderingUI = true;
	glViewport(0, 0, WIDTH_UI, HEIGHT_UI);
	uiFB.use();
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, WIDTH_UI, HEIGHT_UI);
	self->updateProjection(WIDTH_UI, HEIGHT_UI);

	static bool prevPressDown = false;
	bool pressDown = getActionBool(Actions.Press);
	keepXZCameraPosition = false;

	if (displayUIOnHand)
	{
		bool handleInputs = pointCursor(false, true);
		// health
		{
			TexRenderer& healthRenderer = self->player.healthRenderer;

			int h = (int)(self->player.health / 100.f * 48.f);
			healthRenderer.setClip(0, 48 - h, 32, h);
			healthRenderer.setPos(20, HEIGHT_UI - h * 2 - 48, 64, h * 2);
			healthRenderer.render();

			healthRenderer.setClip(32, 0, 32, 48);
			if (time - self->player.damageTime < Player::DAMAGE_COOLDOWN)
			{
				glm::vec2 a = glm::diskRand(5.f);
				glm::vec2 b = glm::diskRand(5.f);

				healthRenderer.setColor(1, 0, 1, 1);
				healthRenderer.setPos(a.x + 20, a.y + HEIGHT_UI - 96 - 48, 64, 96);
				healthRenderer.render();

				healthRenderer.setColor(0, 1, 1, 1);
				healthRenderer.setPos(b.x + 20, b.y + HEIGHT_UI - 96 - 48, 64, 96);
				healthRenderer.render();

				healthRenderer.setColor(1, 1, 1, 1);
			}

			healthRenderer.setPos(20, HEIGHT_UI - 96 - 48, 64, 96);
			healthRenderer.render();
		}
		// hotbar
		{
			InventoryGrid& hotbar = self->player.hotbar;
			for (int yInd = 0; yInd < hotbar.size.y; yInd++)
			{
				int xPos = 20 + yInd * 54 + 64 + 20;
				int yPos = (yInd % 2 == 0 ? 60 : 0) + HEIGHT_UI - 20 - 128;

				// interactioninging
				double xposa, yposa;
				glfwGetCursorPos(stateManager->window, &xposa, &yposa);
				bool hovered = aabb((int)xposa, (int)yposa, 1, 1, xPos + 5, HEIGHT_UI - 20 - 128, 70 - 10, 72 * 2 + 60);
				bool selected = yInd == hotbar.selectedIndex;

				InventoryGrid::tr.setClip(!hovered || selected ? 0 : 36, 0, 35, 36);
				InventoryGrid::tr.setPos(xPos, yPos, 70, 72);
				InventoryGrid::tr.setColor(1, 1, 1, selected ? 1 : 0.5f);
				InventoryGrid::tr.render();

				auto& slot = hotbar.getSlot(yInd);
				if (slot)
				{
					Item::renderItem(slot, { xPos, yPos });
				}

				if (handleInputs && hovered && pressDown && !prevPressDown)
				{
					self->player.hotbar.selectedIndex = yInd;
					self->world->localPlayerEvent(&self->player, Packet::C_HOTBAR_SLOT_SELECT, self->player.hotbar.selectedIndex, NULL);
				}
			}
			// equipment
			{
				int xPos = 20 + 64 + 20;
				int yPos = -60 + HEIGHT_UI - 20 - 128;

				InventoryGrid::tr.setClip(0, 0, 35, 36);
				InventoryGrid::tr.setPos(xPos, yPos, 70, 72);
				InventoryGrid::tr.setColor(1, 1, 1, 0.5f);
				InventoryGrid::tr.render();

				auto& slot = self->player.equipment.getSlot(0);
				if (slot)
				{
					Item::renderItem(slot, { xPos, yPos });
				}
			}
		}

		// clock lol
		{
			std::time_t currentTime = std::time(nullptr);
			std::tm* localTime = std::localtime(&currentTime);
			int hours = localTime->tm_hour;
			int minutes = localTime->tm_min;
			int seconds = localTime->tm_sec;

			FontRenderer& font = self->font;
			font.centered = false;
			font.color = glm::vec4{ 0, 0, 0, 1 };
			font.fontSize = 2;
			font.pos = glm::ivec2{ 104 + 20 + 70, (-60 + HEIGHT_UI - 20 - 128) + 72 / 2 - 14 / 2};
			font.setText(std::format("{}:{:02}:{:02}", hours, minutes, seconds));
			font.updateModel();
			font.render();

			font.color = glm::vec4{ 1 };
			font.pos -= glm::ivec2{ 2, 2 };
			font.updateModel();
			font.render();
		}
	}

	prevPressDown = pressDown;

	if (self->chatOpen)
	{
		self->chatInput.select();
		self->ui.render();
		pointCursor();
	}
	if (quickActionsOpen)
	{
		qaUI.render();
		pointCursor(true, false, true);
	}
	glViewport(0, 0, WIDTH_UI, HEIGHT_UI);

	if (!self->chatMessages.empty())
	{
		constexpr float visibleTime = 8.0f;
		constexpr float fadeOutTime = 2.0f;
		double d = glfwGetTime() - self->lastChatMessageTime;
		if (d < visibleTime + fadeOutTime)
		{
			std::unique_ptr<gui::Text>& chatMessage = self->chatMessages.back();
			if (chatMessage)
			{
				float a = glm::mix(1.0f, 0.0f, glm::clamp(((float)d - visibleTime) / fadeOutTime, 0.0f, 1.0f));
				int xP, yP, xS, yS;
				//chatMessage->getPos(&self->chatMessageContainer, &xP, &yP);
				chatMessage->getSize(&self->chatMessageContainer, &xS, &yS);
				xP = 12;
				yP = HEIGHT_UI - 78 - 60 * 4;
				self->qr.setQuadRendererMode(GL_TRIANGLES);
				self->qr.setColor(0, 0, 0, a * 0.7f);
				self->qr.setPos(xP - 2, yP - 2, xS + 4, yS + 4);
				self->qr.render();
				self->font.centered = false;
				self->font.fontSize = chatMessage->size;
				self->font.pos = { xP, yP };
				self->font.color = chatMessage->color;
				self->font.color.a = a;
				self->font.setText(chatMessage->text);
				self->font.render();
			}
		}
	}

	if (self->player.inventoryManager.isOpen())
	{
		if (s.states.back() != self)
		{
			self->player.inventoryManager.primary = nullptr;
			self->player.inventoryManager.secondary = nullptr;
			return;
		}
		self->qr.setQuadRendererMode(GL_TRIANGLES);
		self->qr.setColor(0, 0, 0, 0.3f);
		self->qr.setPos(0, 0, WIDTH_UI, HEIGHT_UI);
		self->qr.render();

		double cursorX, cursorY;
		glfwGetCursorPos(s.window, &cursorX, &cursorY);
		glm::ivec2 cursor{ cursorX, cursorY };
		self->player.inventoryManager.ui.render();
		self->player.inventoryManager.secondary->render(cursor);
		self->player.inventoryManager.primary->render(cursor);
		self->player.inventoryManager.cursor.pos = cursor;
		if (self->player.inventoryManager.cursor.item)
		{
			Item::renderItem(self->player.inventoryManager.cursor.item, { cursorX - 20, cursorY - 20 });
		}
		pointCursor();
	}
}
#endif
// add shadow to the item count
$hookStatic(void, Item, renderItem, std::unique_ptr<Item>& item, const glm::ivec2& pos)
{
	item->render(pos);
	if (item->getStackLimit() > 1 || item->count > 1)
	{
		Item::fr.setText(std::to_string(item->count));
		Item::fr.fontSize = 2;
		// shadow
		Item::fr.color = { 0,0,0,1 };
		Item::fr.pos.x = pos.x + 40 + 2;
		Item::fr.pos.y = pos.y + 40 + 2;
		Item::fr.updateModel();
		Item::fr.render();
		// text
		Item::fr.color = { 1,1,1,1 };
		Item::fr.pos.x = pos.x + 40;
		Item::fr.pos.y = pos.y + 40;
		Item::fr.updateModel();
		Item::fr.render();
	}
}
#ifndef NONVR
#include "4DKeyBinds.h"

void addButton(int& elements, int& maxW, gui::ContentBox& qaCB, const stl::string& name, gui::ButtonCallback callback, void* user = nullptr)
{
	gui::Button* btn = new gui::Button();
	btn->setText(name);
	btn->alignX(gui::ALIGN_CENTER_X);
	btn->alignY(gui::ALIGN_CENTER_Y);
	btn->width = std::max((int)btn->text.size() * 20 + 20, 100);
	btn->height = 40;
	if (btn->width > maxW)
		maxW = btn->width;
	btn->user = user;
	btn->callback = callback;
	qaCB.addElement(btn);
	qaEls.push_back(btn);
	elements++;
}

void addText(int& elements, int& maxW, gui::ContentBox& qaCB, const stl::string& text)
{
	while (elements % 10 >= 6)
	{
		gui::Element* empty = new gui::Element();
		qaCB.addElement(empty);
		qaEls.push_back(empty);

		elements++;
	}
	gui::Text* t = new gui::Text();
	t->setText(text);
	t->alignX(gui::ALIGN_CENTER_X);
	t->alignY(gui::ALIGN_CENTER_Y);
	t->shadow = true;
	int w = t->text.size() * 10 + 20;
	if (w > maxW)
		maxW = w;
	qaCB.addElement(t);
	qaEls.push_back(t);
	elements++;
}

$hook(void, StateGame, init, StateManager& s)
{
	eraseAllHaptics();

	updateHMDMatrixPose({0,0,0,1}, &self->player.orientation);
	self->player.left = *(glm::vec4*)orientation[0];
	self->player.up = *(glm::vec4*)orientation[1];
	self->player.forward = *(glm::vec4*)orientation[2];
	self->player.over = *(glm::vec4*)orientation[3];

	//pHeight = hmdPose[1][3];

	/*
	skinScale = pHeight / (ps::HEIGHT - 8) * 0.87f;

	float upperArmLengthL = (ps::meshBounds[ps::UPPER_ARM_L].upperBound.y - ps::meshBounds[ps::UPPER_ARM_L].lowerBound.y) * skinScale;
	float foreArmLengthL = (ps::meshBounds[ps::FOREARM_L].upperBound.y - ps::meshBounds[ps::FOREARM_L].lowerBound.y) * skinScale;
	float upperArmLengthR = (ps::meshBounds[ps::UPPER_ARM_R].upperBound.y - ps::meshBounds[ps::UPPER_ARM_R].lowerBound.y) * skinScale;
	float foreArmLengthR = (ps::meshBounds[ps::FOREARM_R].upperBound.y - ps::meshBounds[ps::FOREARM_R].lowerBound.y) * skinScale;

	armIKL = IKSolver();
	armIKL.addPart({ {upperArmLengthL / -2, pHeight * 0.82f, 0}, {0, 0, 0}, upperArmLengthL });
	armIKL.addPart({ {upperArmLengthL / -2, pHeight * 0.82f + upperArmLengthL, 0}, {0, 0, 0}, foreArmLengthL });

	armIKR = IKSolver();
	armIKR.addPart({ {upperArmLengthR / 2, pHeight * 0.82f, 0}, {0, 0, 0}, upperArmLengthR });
	armIKR.addPart({ {upperArmLengthR / 2, pHeight * 0.82f + upperArmLengthR, 0}, {0, 0, 0}, foreArmLengthR });
	*/
	/*armIKL = IKSolver();
	armIKL.addPart({ {0.5f / 2, 0, 0}, {0, 0, 0}, 0.5f });
	armIKL.addPart({ {0.5f / 2, 0.5f, 0}, {0, 0, 0}, 0.5f });

	armIKR = IKSolver();
	armIKR.addPart({ {0.5f / -2, 0, 0}, {0, 0, 0}, 0.5f });
	armIKR.addPart({ {0.5f / -2, 0.5f, 0}, {0, 0, 0}, 0.5f });*/

	original(self, s);

	self->chatOpen = false;
	activeTextInput = nullptr;

	font3D = FontRenderer3D{ ResourceManager::get("pixelfont.png"), ShaderManager::get("4dvr_textShader") };

	self->renderFramebuffer.init(eyeW, eyeH, false);

	self->crosshairRenderer.setColor(0, 0, 0, 0);

	EntityButterfly::renderInit();

	for (auto& e : qaEls)
		delete e;
	qaEls.clear();

	for (auto& p : qaPairsToRemove)
		delete p;
	qaPairsToRemove.clear();

	qaUI = gui::Interface{ s.window };
	qaUI.viewportCallback = StateGame::viewportCallback;
	qaUI.viewportUser = s.window;
	qaUI.font = &self->font;
	qaUI.qr = &self->qr;

	static gui::ContentBox qaCB{};
	qaCB.clear();
	qaCB.parent = &qaUI;
	qaCB.scrollStep = 50;
	qaCB.width = WIDTH_UI - 20;
	qaCB.height = HEIGHT_UI - 20;
	qaCB.alignX(gui::ALIGN_CENTER_X);
	qaCB.alignY(gui::ALIGN_CENTER_Y);
	qaUI.addElement(&qaCB);

	int elements = 0;
	int maxW = 100;

	// 4dvr default actions
	{
		addText(elements, maxW, qaCB, "4DVR");
		addButton(elements, maxW, qaCB, "Screenshot", [](void* user)
			{
				quickActionsOpen = false;
				takeAScreenshot = true;
				takeAScreenshotTime = glfwGetTime();
			});
		addButton(elements, maxW, qaCB, "Open Chat", [](void* user)
			{
				quickActionsOpen = false;
				StateGame::instanceObj.keyInput(*stateManager, GLFW_KEY_T, 0, GLFW_PRESS, 0);
			});
		addButton(elements, maxW, qaCB, "Drop Item", [](void* user)
			{
				quickActionsOpen = false;
				StateGame::instanceObj.world->localPlayerEvent(&StateGame::instanceObj.player, Packet::C_ITEM_THROW_HOTBAR, 0, 0);
			});
		addButton(elements, maxW, qaCB, "Snap XZ", [](void* user)
			{
				quickActionsOpen = false;

				Player& player = StateGame::instanceObj.player;

				//player.pos.x = (float)player.currentBlock.x + 0.5f;
				//player.pos.z = (float)player.currentBlock.z + 0.5f;
				//player.pos.w = (float)player.currentBlock.w + 0.5f;

				player.left = glm::vec4{ 1, 0, 0, 0 };
				player.up = glm::vec4{ 0, 1, 0, 0 };
				player.forward = glm::vec4{ 0, 0, 1, 0 };
				player.over = glm::vec4{ 0, 0, 0, 1 };
				player.orientation = m4::Mat5{ 1 };
				player.hyperplaneUpdateFlag = true;
			});
		addButton(elements, maxW, qaCB, "Snap XW", [](void* user)
			{
				quickActionsOpen = false;

				Player& player = StateGame::instanceObj.player;

				//player.pos.x = (float)player.currentBlock.x + 0.5f;
				//player.pos.z = (float)player.currentBlock.z + 0.5f;
				//player.pos.w = (float)player.currentBlock.w + 0.5f;

				player.left = glm::vec4{ 1, 0, 0, 0 };
				player.up = glm::vec4{ 0, 1, 0, 0 };
				player.forward = glm::vec4{ 0, 0, 0, 1 };
				player.over = glm::vec4{ 0, 0, -1, 0 };

				player.orientation = m4::Mat5{ 1 };
				player.orientation[2][2] = 0;
				player.orientation[2][3] = 1;
				player.orientation[3][3] = 0;
				player.orientation[3][2] = -1;
				player.hyperplaneUpdateFlag = true;
			});
		addButton(elements, maxW, qaCB, "Snap ZW", [](void* user)
			{
				quickActionsOpen = false;

				Player& player = StateGame::instanceObj.player;

				//player.pos.x = (float)player.currentBlock.x + 0.5f;
				//player.pos.z = (float)player.currentBlock.z + 0.5f;
				//player.pos.w = (float)player.currentBlock.w + 0.5f;

				player.left = glm::vec4{ 0, 0, 0, 1 };
				player.up = glm::vec4{ 0, 1, 0, 0 };
				player.forward = glm::vec4{ 0, 0, 1, 0 };
				player.over = glm::vec4{ -1, 0, 0, 0 };

				player.orientation = m4::Mat5{ 1 };
				player.orientation[0][0] = 0;
				player.orientation[0][3] = 1;
				player.orientation[3][0] = -1;
				player.orientation[3][3] = 0;
				player.hyperplaneUpdateFlag = true;
			});
		addButton(elements, maxW, qaCB, "Toggle W-Depth", [](void* user)
			{
				quickActionsOpen = false;

				wOffset = !wOffset;

				dumpConfig();
			});
	}

	// 4DKeyBinds actions
	if (KeyBinds::isLoaded())
	{
		auto playerBinds = KeyBinds::getBinds(KeyBindsScope::PLAYER);

		std::map<stl::string, std::vector<stl::string>> bindsMap;
		for (auto& bind : playerBinds)
		{
			bindsMap[bind.first].push_back(bind.second);
		}

		// create stuff
		for (auto& ns : bindsMap)
		{
			if (ns.first == "4D Miner") continue;

			addText(elements, maxW, qaCB, ns.first);
			
			for (auto& bind : ns.second)
			{
				qaPairsToRemove.emplace_back(new std::pair<std::string, std::string>(ns.first, bind));
				addButton(elements, maxW, qaCB, bind, [](void* user)
					{
						auto bindData = (std::pair<std::string, std::string>*)user;
						KeyBinds::triggerBind(bindData->first, bindData->second, KeyBindsScope::PLAYER, GLFW_PRESS, 0);
						KeyBinds::triggerBind(bindData->first, bindData->second, KeyBindsScope::PLAYER, GLFW_RELEASE, 0);
						quickActionsOpen = false;
					}, qaPairsToRemove.back());
			}
		}
	}

	// positioning
	qaCB.scrollW = elements / 10 * maxW + maxW * 2 - (int)qaCB.width;
	qaCB.width += std::min(qaCB.scrollW, 0);
	qaCB.scrollW = std::max(qaCB.scrollW, 0);
	for (int i = 0; i < elements; i++)
	{
		int x = i / 10 * maxW;
		int y = (i % 10) * 50;
		gui::Element* e = qaEls[i];
		e->offsetX(x - (elements / 10 * maxW / 2) + qaCB.scrollW / 2);
		e->offsetY(y - (std::min(10, elements) * 50 / 2));
	}
}
$hook(void, StateGame, close, StateManager& s)
{
	original(self, s);

	for (auto& e : qaEls)
		delete e;
	qaEls.clear();

	for (auto& p : qaPairsToRemove)
		delete p;
	qaPairsToRemove.clear();

	qaUI.clear();
}
$hook(void, StateGame, mouseInput, StateManager& s, double xpos, double ypos)
{
	original(self, s, xpos, ypos);
	if (quickActionsOpen)
		qaUI.mouseInput(xpos, ypos);
}
$hook(void, StateGame, scrollInput, StateManager& s, double xoff, double yoff)
{
	original(self, s, xoff, yoff);
	if (quickActionsOpen)
		qaUI.scrollInput(xoff, yoff);
}
$hook(void, StateGame, mouseButtonInput, StateManager& s, int button, int action, int mods)
{
	original(self, s, button, action, mods);
	if (quickActionsOpen)
		qaUI.mouseButtonInput(button, action, mods);
}
$hook(void, StateGame, keyInput, StateManager& s, int key, int scancode, int action, int mods)
{
	original(self, s, key, scancode, action, mods);
	if (quickActionsOpen)
		qaUI.keyInput(key, scancode, action, mods);
}

$hook(bool, EntityChest, action, World* world, Entity* actor, int action)
{
	bool result = original(self, world, actor, action);

	if (result)
	{
		EntityPlayer* entityPlayer = reinterpret_cast<EntityPlayer*>(actor);
		glm::vec4 pos3D = m4::Mat5::inverse(entityPlayer->player->orientation).multiply((self->pos + glm::vec4(0.5f, 1.5f, 0.5f, 0.5f)) - entityPlayer->player->pos, 1);
		glm::vec3 pos3D3D{ pos3D.x, pos3D.y, pos3D.z };
		glm::vec3 head{ -hmdPose[0][3], hmdPose[1][3], -hmdPose[2][3] };
		glm::vec3 dir = head - pos3D3D;
		dir = glm::normalize(dir);

		planeScale = glm::vec3{ 2.f, 2.f, 1.f };
		planePosition = pos3D3D + planeOffset();
		planeNormal = dir;
		planeUp = glm::cross(glm::cross(planeNormal, {0,1,0}), planeNormal);
	}

	return result;
}

$hook(void, StateGame, addChatMessage, Player* player, const stl::string& message, uint32_t color)
{
	if (player != &StateGame::instanceObj.player)
	{
		auto a = new VR::HapticPulse(10, 8, 0.5f, 0.0f, 0.15f);
		auto b = new VR::HapticPulse(8, 10, 0.4f, 0.0f, 0.15f);
		b->startTime += 0.2f;

		VR::addHapticEvent(VR::CONTROLLER_LEFT, a);
		VR::addHapticEvent(VR::CONTROLLER_LEFT, b);
	}
	return original(self, player, message, color);
}

$exec
{
	uint8_t newBytes[0x7e];
	memset(newBytes, 0x90, sizeof(newBytes));
	patchMemory(getFuncAddr((int)Func::StateGame::render) + 0x1B9F, newBytes, sizeof(newBytes)); // remove framebuffer render and glClear(GL_DEPTH_BUFFER_BIT)

	// removing blending changes
	uint8_t newBytes2[0x102 - 0xE9];
	memset(newBytes2, 0x90, sizeof(newBytes2));
	patchMemory(getFuncAddr((int)Func::StateGame::render) + 0xE9, newBytes2, sizeof(newBytes2));

	uint8_t newBytes3[0x219 - 0x20B];
	memset(newBytes3, 0x90, sizeof(newBytes3));
	patchMemory(getFuncAddr((int)Func::StateGame::updateProjection) + 0x20B, newBytes3, sizeof(newBytes3));

	// stop chat from rendering in the world render
	uint8_t newBytes4[0x1CF9 - 0x1CEB];
	memset(newBytes4, 0x90, sizeof(newBytes4));
	patchMemory(getFuncAddr((int)Func::StateGame::render) + 0x1CEB, newBytes4, sizeof(newBytes4));

	uint8_t newBytes5[0x1D08 - 0x1CF9];
	memset(newBytes5, 0x90, sizeof(newBytes5) - 1);
	newBytes5[sizeof(newBytes5) - 1] = 0xEB; // jz -> jmp
	patchMemory(getFuncAddr((int)Func::StateGame::render) + 0x1CF9, newBytes5, sizeof(newBytes5));

	// remove the annoying 4D Glasses matrix stuff
	uint8_t newBytes6[]{0x90,0x90,0x90,0xE9,0xCA,0x04,0x00,0x00,0x90};
	patchMemory(getFuncAddr((int)Func::ItemMaterial::renderEntity) + 0x20FB, newBytes6, sizeof(newBytes6));

	// remove glBindFramebuffer(GL_FRAMEBUFFER, 0); from Framebuffer::render()
	uint8_t newBytes7[]{ 0x90,0x90,0x90,0x90,0x90,0x90 };
	patchMemory(getFuncAddr((int)Func::Framebuffer::render) + 0x19, newBytes7, sizeof(newBytes7));

	// remove the entire StateGame::updateProjection() so that i can call it to have mods working
	uint8_t newBytes8[0x140C - 0x7];
	memset(newBytes8, 0x90, sizeof(newBytes8) - 1);
	newBytes8[sizeof(newBytes8) - 1] = 0xC3;
	patchMemory(getFuncAddr((int)Func::StateGame::updateProjection) + 0x7, newBytes8, sizeof(newBytes8));

	// remove Player::renderHud() call from StateGame::render()
	uint8_t newBytes9[0x1CD9 - 0x1CC5];
	memset(newBytes9, 0x90, sizeof(newBytes9));
	patchMemory(getFuncAddr((int)Func::StateGame::render) + 0x1CC5, newBytes9, sizeof(newBytes9));

}
#endif
