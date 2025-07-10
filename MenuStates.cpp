#include "matConverts.h"

#include "Constants.h"
#include "VR.h"

#include <4dm.h>
using namespace fdm; // 4dm.h

#include <openvr.h>
using namespace vr;

#include "VRTex.h"

#include <glm/gtx/string_cast.hpp>

#include "Player.h"

using namespace VRStuff;

#include "Framebuffer3D.h"

#ifndef NONVR

$hook(void, StateTitleScreen, updateProjection, int width, int height)
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
		int w, h;
		glfwGetWindowSize(stateManager->window, &w, &h);
		self->projection3D = glm::perspective(
			glm::radians((float)StateSettings::instanceObj.currentFOV + 30),
			(float)w / (float)h,
			Player::Z_NEAR,
			Player::Z_FAR);
		break;
	}
	self->projection2D = glm::ortho(0.f, (float)WIDTH_UI, (float)HEIGHT_UI, 0.f, -1.f, 1.f);

	if (self->bgWorld)
		self->bgWorld->updateRenderFrustum(self->projection3D);

	// Update Shaders
	{
		updateProjection2D(self->projection2D);

		const Shader* blockShader = ShaderManager::get("blockShader");
		blockShader->use();
		glUniformMatrix4fv(glGetUniformLocation(blockShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* simplexColorShader = ShaderManager::get("tetColorShader");
		simplexColorShader->use();
		glUniformMatrix4fv(glGetUniformLocation(simplexColorShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

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

		const Shader* tex2Din3DShader = ShaderManager::get("4dvr_tex2Din3DShader");
		tex2Din3DShader->use();
		glUniformMatrix4fv(glGetUniformLocation(tex2Din3DShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* vr_textShader = ShaderManager::get("4dvr_textShader");
		vr_textShader->use();
		glUniformMatrix4fv(glGetUniformLocation(vr_textShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);

		const Shader* playerSkinShader = ShaderManager::get("playerSkinShader");
		playerSkinShader->use();
		glUniformMatrix4fv(glGetUniformLocation(playerSkinShader->id(), "P"), 1, GL_FALSE, &self->projection3D[0][0]);
	}
}

$hook(void, StateSingleplayer, updateProjection, const glm::ivec2& size, const glm::ivec2& translate2D)
{
	updateProjection(size, translate2D);
}
$hook(void, StateCredits, updateProjection, int width, int height)
{
	updateProjection(width, height);
}
$hook(void, StateDeathScreen, updateProjection, const glm::ivec2& size, const glm::ivec2& translate2D)
{
	updateProjection(size, translate2D);
}
$hook(void, StateErrorScreen, windowResize, int width, int height)
{
	updateProjection(width, height);
}
$hook(void, StateMultiplayer, updateProjection, const glm::ivec2& size, const glm::ivec2& translate2D)
{
	updateProjection(size, translate2D);
}
$hook(void, StateSettings, updateProjection, const glm::ivec2& size, const glm::ivec2& translate2D)
{
	updateProjection(size, translate2D);
}
$hook(void, StateSkinChooser, updateProjection, const glm::ivec2& size, const glm::ivec2& translate2D)
{
	updateProjection(size, translate2D);
}
$hook(void, StateTutorial, updateProjection, const glm::ivec2& size, const glm::ivec2& translate2D)
{
	original(self, size, translate2D);
	glm::mat4 proj2D = glm::ortho(0.f, (float)size.x, (float)size.y, 0.f, -1.f, 1.f);
	proj2D = glm::translate(proj2D, { translate2D.x, translate2D.y, 0 });
	self->projection2D = proj2D;
	updateProjection2D(proj2D);
}
$hook(void, StateTutorialSlideshow, updateProjection, const glm::ivec2& size, const glm::ivec2& translate2D)
{
	updateProjection(size, translate2D);
}

$hook(void, StateCreateWorld, render, StateManager& s)
{
	renderingUI = true;
	original(self, s);
	pointCursor();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
$hook(void, StateCredits, render, StateManager& s)
{
	renderingUI = true;
	original(self, s);
	pointCursor();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
$hook(void, StateErrorScreen, render, StateManager& s)
{
	renderingUI = true;
	original(self, s);
	pointCursor();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
$hook(void, StateErrorScreen, update, StateManager& s, double dt)
{
	original(self, s, dt);
	handleEvents();
}
$hook(void, StateMultiplayer, render, StateManager& s)
{
	renderingUI = true;
	original(self, s);
	pointCursor();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
$hook(void, StateSettings, render, StateManager& s)
{
	renderingUI = true;
	original(self, s);
	pointCursor();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
$hook(void, StateTutorial, render, StateManager& s)
{
	original(self, s);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
$hook(void, StateWorldGen, render, StateManager& s)
{
	renderingUI = true;
	original(self, s);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
$hook(void, StateTitleScreen, update, StateManager& s, double dt)
{
	original(self, s, dt);
	handleEvents();
}

// the fun part but stolen (from myself)
static glm::vec3 skinPos = glm::vec3{ 0.f, 1.1f, 0.8f };
static float skinXZAngle = glm::pi<float>();

$hook(void, StateSkinChooser, init, StateManager& s)
{
	original(self, s);
	skinPos = glm::vec3{ 0.f, 1.1f, 0.8f };
	skinXZAngle = glm::pi<float>();
}

$hook(void, StateSkinChooser, update, StateManager& s, double dt)
{
	original(self, s, dt);

	static bool prevRotating = false, rotating = false;
	rotating = getActionBool(Actions.DragRotate);
	static glm::vec3 rotA, rotB;
	if (rotating && HMD->IsTrackedDeviceConnected(rCInd))
	{
		rotB = rotA;

		glm::mat4 controller = trackedPoses[rCInd];
		glm::vec3 pos{ -controller[0][3], controller[1][3], -controller[2][3] };
		glm::vec3 posXZ{ pos.x, 0.f, pos.z };

		rotA = glm::normalize(pos - skinPos);

		if (!prevRotating)
			rotB = rotA;

		rotA.y = 0;
		rotB.y = 0;

		rotA = glm::normalize(rotA);
		rotB = glm::normalize(rotB);

		float angle = glm::acos(glm::clamp(glm::dot(rotB, rotA), -1.f, 1.f));
		glm::vec3 cross = glm::cross(rotB, rotA);
		if (cross.y < 0.0f)
			angle = -angle;

		skinXZAngle += angle;
	}
	prevRotating = rotating;

	bool moving = getActionBool(Actions.Move) && !rotating;
	if (moving && HMD->IsTrackedDeviceConnected(rCInd))
	{
		glm::vec3 vel = trackedVelocities[rCInd] * (float)dt;
		skinPos += glm::vec3{ -vel.x, vel.y, -vel.z };
	}
}

$hook(void, StateTitleScreen, render, StateManager& s)
{
	eraseAllHaptics();
	renderingUI = false;
	if (!uiFB.FBO)
		uiFB = Framebuffer3D(WIDTH_UI, HEIGHT_UI, true);

	static const glm::vec4 skyColors[4] =
	{ glm::vec4(self->skyColorBottom, 1), glm::vec4(self->skyColorTop, 1), glm::vec4(self->skyColorTop, 1), glm::vec4(self->skyColorBottom, 1) };

	static const std::function<void()> vrRender = [=]
		{
			glDepthMask(GL_TRUE);
			glDisable(GL_DEPTH_TEST); 

			int width, height;
			glfwGetWindowSize(s.window, &width, &height);
			self->qr.setQuadRendererMode(4);
			self->qr.setColor(skyColors);
			self->qr.setPos(0, 0, width, height);
			self->qr.render();

			glEnable(GL_DEPTH_TEST); 

			use3DOrientation = true;
			Mat5 MV = m4::createCamera({ 0,0,0,0 }, { 0,0,1,0 }, { 0,1,0,0 }, { 1,0,0,0 }, { 0,0,0,1 });
			MV.translate(glm::vec4{ 0,0,0,0.01f });
			
			if (self->bgWorld)
			{
				const Shader* blockShader = ShaderManager::get("blockShader");
				const Tex2D* tiles = ResourceManager::get("tiles.png");

				Mat5 worldMV = MV;
				worldMV.translate(-(self->bgPos - glm::vec4{ 0,10,0,0 }));

				blockShader->use();

				glActiveTexture(GL_TEXTURE0);

				glBindTexture(tiles->target, tiles->ID);

				glUniform2ui(glGetUniformLocation(blockShader->id(), "texSize"), 96, 16);

				self->bgWorld->render(worldMV, false, glm::vec3{ 1.0, 0.5, 0.35 });
			}

			if (stateManager->states.back() == &StateSkinChooser::instanceObj)
			{
				MV.translate(glm::vec4(skinPos, 0.f));
				MV *= m4::Mat5(m4::Rotor(m4::wedge({ 1,0,0,0 }, { 0,0,1,0 }), -skinXZAngle));
				MV.scale(glm::vec4(0.3f, 0.3f, 0.3f, 1.f));
				
				StateSkinChooser::instanceObj.skinRenderer.render(MV, { 0,1,0,0 }, nullptr, nullptr, false);
			}

			use3DOrientation = false;

			glEnable(GL_DEPTH_TEST);
			renderUIPlane();
			drawHands();
			glDisable(GL_DEPTH_TEST);

			glDepthMask(GL_TRUE);
		};

	if (self->bgWorld && HMD)
	{
		updateHMDMatrixPose({ 0,0,0,1 });

		// left eye
		eyeL.use();
		glViewport(0, 0, eyeW, eyeH);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		curEye = VEye_Left;
		updateProjection(eyeW, eyeH);
		
		vrRender();

		// right eye
		eyeR.use();
		glViewport(0, 0, eyeW, eyeH);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		curEye = VEye_Right;
		updateProjection(eyeW, eyeH);

		vrRender();

		// send textures to OpenVR (aka SteamVR)
		eyeL.drawEye(Eye_Left);
		eyeR.drawEye(Eye_Right);

		VRCompositor()->PostPresentHandoff();

		// desktop view
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (desktopView)
		{
			curEye = VEye_PC;
			int width, height;
			glfwGetWindowSize(s.window, &width, &height);
			glViewport(0, 0, width, height);
			updateProjection(width, height);

			vrRender();
		}

		// render ui into a texture
		renderingUI = true;
		glm::mat4 proj2D = glm::ortho(0.f, (float)WIDTH_UI, (float)HEIGHT_UI, 0.f, -1.f, 1.f);
		updateProjection2D(proj2D);
		uiFB.use();
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, WIDTH_UI, HEIGHT_UI);
		if (!self->paused)
		{
			self->qr.setQuadRendererMode(GL_TRIANGLES);
			self->qr.setColor(0,0,0,0.25f);
			self->qr.setPos(0, 0, WIDTH_UI, HEIGHT_UI);
			self->qr.render();

			self->ui.render();

			self->logoRenderer.setPos(
				WIDTH_UI / 2 - self->logoRenderer.texture->width * 2,
				75,
				self->logoRenderer.texture->width * 4,
				self->logoRenderer.texture->height * 4);
			self->logoRenderer.render();
		}
		pointCursor();
	}
	else
	{
		original(self, s);
	}
}

$hook(void, StateTitleScreen, init, StateManager& s)
{
	original(self, s);
	VRApplications()->IdentifyApplication(VRApplications()->GetCurrentSceneProcessId(), "tr1ngledev.4dvr");

	planePosition = menuPlanePosition;
	planeScale = glm::vec3{ 2 };
	planeNormal = { 0,0,-1 };
	planeUp = { 0,1,0 };

	self->logoRenderer.texture = ResourceManager::get("assets/textures/logo.png", true);
}

$hook(void, StateSkinChooser, render, StateManager& s)
{
	//original(self, s);
	StateTitleScreen::instanceObj.render(s);

	renderingUI = true;
	self->ui.render();
	pointCursor();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

$hook(void, PlayerSkinViewer, render, gui::Window* w)
{
	glDisable(GL_DEPTH_TEST);

	renderingUI = true;
	uiFB.use();
	glViewport(0, 0, WIDTH_UI, HEIGHT_UI);
}

$hook(void, StatePause, init, StateManager& s)
{
	original(self, s);

	static gui::Button openChatButton{};
	openChatButton.alignX(gui::ALIGN_RIGHT);
	openChatButton.alignY(gui::ALIGN_BOTTOM);
	openChatButton.setText("Open Chat");
	openChatButton.height = 50;
	openChatButton.width = 200;
	openChatButton.offsetX(-60);
	openChatButton.offsetY(-50);
	openChatButton.callback = [](void* user)
		{
			stateManager->popState(); // close the pause menu
			StateGame::instanceObj.keyInput(*stateManager, GLFW_KEY_T, 0, GLFW_PRESS, 0);
		};
	self->ui.addElement(&openChatButton);

	static gui::Button screenshotButton{};
	screenshotButton.alignX(gui::ALIGN_RIGHT);
	screenshotButton.alignY(gui::ALIGN_BOTTOM);
	screenshotButton.setText("Screenshot");
	screenshotButton.height = 50;
	screenshotButton.width = 200;
	screenshotButton.offsetX(-60);
	screenshotButton.offsetY(-120);
	screenshotButton.callback = [](void* user)
		{
			stateManager->popState(); // close the pause menu
			takeAScreenshot = true;
			takeAScreenshotTime = glfwGetTime();
		};
	self->ui.addElement(&screenshotButton);


	static gui::Button calibrateHeightButton{};
	calibrateHeightButton.setText(std::format("Calibrate Height ({}%)", (int)round(heightRatio * 100)));
	calibrateHeightButton.alignX(gui::ALIGN_RIGHT);
	calibrateHeightButton.alignY(gui::ALIGN_BOTTOM);
	calibrateHeightButton.offsetX(-60);
	calibrateHeightButton.offsetY(-190);
	calibrateHeightButton.width = 375;
	calibrateHeightButton.height = 50;
	calibrateHeightButton.user = &calibrateHeightButton;
	calibrateHeightButton.callback = [](void* user)
		{
			heightRatio = hmdPose[1][3] / Player::HEIGHT;

			dumpConfig();

			((gui::Button*)user)->setText(std::format("Calibrate Height ({}%)", (int)round(heightRatio * 100)));
		};
	self->ui.addElement(&calibrateHeightButton);
}

// Tutorial
$hook(void, StateTutorialSlideshow, update, StateManager& s, double dt)
{
	StateTitleScreen::instanceObj.update(s, dt);
	original(self, s, dt);
}
$hook(void, StateTutorialSlideshow, render, StateManager& s)
{
	StateTitleScreen::instanceObj.render(s);

	self->qr.setQuadRendererMode(GL_TRIANGLES);
	self->qr.setColor(0, 0, 0, 0.25f);
	self->qr.setPos(0, 0, WIDTH_UI, HEIGHT_UI);
	self->qr.render();

	self->ui.render();

	double t = glfwGetTime() - self->forgAnimStart;

	int fx = ((int)(t * 2) % 3);

	const Shader* tex2DArrayShader = ShaderManager::get("tex2DArrayShader");
	tex2DArrayShader->use();
	glUniform1i(glGetUniformLocation(tex2DArrayShader->id(), "arrayIndex"), fx);

	pointCursor();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
// the fun part
static glm::vec3 worldPos = glm::vec3{ 0.f, 0.8f, 0.675f };
static float worldXZAngle = 0.f;
static constexpr float world3DScale = 1.f / StateTutorial::demo_size * 0.5f;
static bool rotatingForg = false;
static glm::mat4 lastWorldMat = glm::identity<glm::mat4>();
$hook(void, StateTutorial, init, StateManager& s)
{
	original(self, s);
	worldPos = glm::vec3{ 0.f, 0.8f, 0.675f };
	worldXZAngle = 0.f;
	VRActiveActionSet_t actionSet{ Actions.TutorialSet };
	VRInput()->ShowBindingsForActionSet(&actionSet, sizeof(VRActiveActionSet_t), 1, 0);
}
$hook(void, StateTutorial, update, StateManager& s, double dt)
{
	StateTitleScreen::instanceObj.update(s, dt);

	constexpr float rotationSpeed = 2.f;
	float forgRotation = 
		getActionBool(Actions.ForgRotateLeft) ? rotationSpeed : 0.f + 
		getActionBool(Actions.ForgRotateRight) ? -rotationSpeed : 0.f;
	self->player.angleToRotate += forgRotation * dt;

	self->keys.a = getActionBool(Actions.ForgMoveLeft);
	self->keys.d = getActionBool(Actions.ForgMoveRight);
	self->keys.space = getActionBool(Actions.ForgJump);

	static bool prevWorldRotating = false, worldRotating = false;
	worldRotating = getActionBool(Actions.DragRotate);
	static glm::vec3 worldRotA, worldRotB;
	if (worldRotating && HMD->IsTrackedDeviceConnected(rCInd))
	{
		worldRotB = worldRotA;

		glm::mat4 controller = trackedPoses[rCInd];
		glm::vec3 pos{ -controller[0][3], controller[1][3], -controller[2][3] };
		glm::vec3 posXZ{ pos.x, 0.f, pos.z };

		if (!prevWorldRotating)
		{
			rotatingForg = pos.y > worldPos.y + 7.f * world3DScale;
		}

		if (!rotatingForg)
			worldRotA = glm::normalize(pos - worldPos);
		else
			worldRotA = glm::normalize(pos - glm::vec3(lastWorldMat * glm::vec4(self->player.pos - glm::vec3{StateTutorial::demo_size * 0.5f} * world3DScale, 1.f)));

		if (!prevWorldRotating)
			worldRotB = worldRotA;

		worldRotA.y = 0;
		worldRotB.y = 0;

		worldRotA = glm::normalize(worldRotA);
		worldRotB = glm::normalize(worldRotB);

		float angle = glm::acos(glm::clamp(glm::dot(worldRotB, worldRotA), -1.f, 1.f));
		glm::vec3 cross = glm::cross(worldRotB, worldRotA);
		if (cross.y < 0.0f)
			angle = -angle;

		if (!rotatingForg)
			worldXZAngle += angle;
		else
		{
			self->player.orientation = glm::rotate(self->player.orientation, angle, { 0,1,0 });
			self->updatePlayerComponentVectors();
		}
	}
	prevWorldRotating = worldRotating;

	bool worldMoving = getActionBool(Actions.Move);
	if (worldMoving && HMD->IsTrackedDeviceConnected(rCInd))
	{
		glm::vec3 vel = trackedVelocities[rCInd] * (float)dt;
		worldPos += glm::vec3{ -vel.x, vel.y, -vel.z };
	}

	original(self, s, dt);
}
$hook(void, StateTutorial, render, StateManager& s)
{
	renderingUI = false;
	static const glm::vec4 skyColors[4] =
	{ glm::vec4(StateTitleScreen::instanceObj.skyColorBottom, 1), glm::vec4(StateTitleScreen::instanceObj.skyColorTop, 1), 
		glm::vec4(StateTitleScreen::instanceObj.skyColorTop, 1), glm::vec4(StateTitleScreen::instanceObj.skyColorBottom, 1) };

	static const std::function<void()> vrRender = [=]
		{
			constexpr float demo_size = StateTutorial::demo_size;
			glDepthMask(GL_TRUE);
			glDisable(GL_DEPTH_TEST);

			self->qr.setQuadRendererMode(4);
			self->qr.setColor(skyColors);
			self->qr.setPos(0, 0, eyeW, eyeH);
			self->qr.render();

			glEnable(GL_DEPTH_TEST);

			Mat5 MV = m4::createCamera({ 0,0,0,0 }, { 0,0,1,0 }, { 0,1,0,0 }, { 1,0,0,0 }, { 0,0,0,1 });
			MV.translate(glm::vec4{ 0,0,0,0.01f });

			if (StateTitleScreen::instanceObj.bgWorld)
			{
				const Shader* blockShader = ShaderManager::get("blockShader");
				const Tex2D* tiles = ResourceManager::get("tiles.png");

				Mat5 worldMV = MV;
				worldMV.translate(-(StateTitleScreen::instanceObj.bgPos - glm::vec4{ 0,10,0,0 }));

				blockShader->use();

				glActiveTexture(GL_TEXTURE0);

				glBindTexture(tiles->target, tiles->ID);

				glUniform2ui(glGetUniformLocation(blockShader->id(), "texSize"), 96, 16);

				StateTitleScreen::instanceObj.bgWorld->render(worldMV, false, glm::vec3{ 1.0, 0.5, 0.35 });
			}

			renderUIPlane();
			drawHands();

			// render the 3d forg world
			{
				// the world itself
				glm::mat4 MVm4 = ToMat4(MV);

				lastWorldMat = glm::identity<glm::mat4>();
				lastWorldMat = glm::scale(lastWorldMat, glm::vec3{ world3DScale });
				lastWorldMat = glm::translate(lastWorldMat, worldPos * demo_size * 2.f);
				lastWorldMat = glm::rotate(lastWorldMat, worldXZAngle, { 0,1,0 });
				lastWorldMat = glm::translate(lastWorldMat, { -demo_size * 0.5f, -demo_size * 0.5f, -demo_size * 0.5f });
				MVm4 *= lastWorldMat;

				glm::mat4 MVP = StateTitleScreen::instanceObj.projection3D * MVm4;

				const Shader* blockShader = ShaderManager::get("blockTutorial3DShader");
				blockShader->use();

				const Tex2D* tiles = ResourceManager::get("tiles.png");
				glBindTexture(tiles->target, tiles->ID);

				glUniform2ui(glGetUniformLocation(blockShader->id(), "texSize"), 96, 16);
				glUniformMatrix4fv(glGetUniformLocation(blockShader->id(), "MVP"), 1, GL_FALSE, &MVP[0][0]);

				self->mr3d.render();

				// the cross-section
				glm::mat4 viewCS = glm::lookAtRH(self->player.pos, self->player.pos + self->player.forward, self->player.up);
				
				glm::mat4 modelViewCSA = glm::scale(viewCS, glm::vec3{ demo_size });
				glm::mat4 MVCSA = glm::scale(MVm4, glm::vec3{ demo_size });

				const Shader* crossAreaShader = ShaderManager::get("blockCrossSectionAreaShader");
				crossAreaShader->use();

				glUniformMatrix4fv(glGetUniformLocation(crossAreaShader->id(), "MV"), 1, GL_FALSE, &MVCSA[0][0]);
				glUniformMatrix4fv(glGetUniformLocation(crossAreaShader->id(), "MV2D"), 1, GL_FALSE, &modelViewCSA[0][0]);
				glUniformMatrix4fv(glGetUniformLocation(crossAreaShader->id(), "P"), 1, GL_FALSE, &StateTitleScreen::instanceObj.projection3D[0][0]);
				glUniform4f(glGetUniformLocation(crossAreaShader->id(), "inColor"), 1.f, 1.f, 1.f, 0.2f);

				self->worldCubeRenderer.render();

				// 3D forg
				const Shader* forg3DShader = ShaderManager::get("forg3DShader");
				forg3DShader->use();

				glm::mat4 MVforg = MVm4;
				MVforg = glm::translate(MVforg, glm::vec3(MVm4[0][2], MVm4[1][2], MVm4[2][2]) * 0.02f);
				MVforg = glm::translate(MVforg, { 0.f, -1.f / 16.f, 0.f });
				MVforg = glm::translate(MVforg, self->player.pos + self->player.forward * -0.5f + glm::vec3{ 0, 1.f / 16.f, 0 });
				MVforg *= self->player.orientation;

				MVP = StateTitleScreen::instanceObj.projection3D * MVforg;

				const Tex2D* forgTex = ResourceManager::get("forg.png");
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(forgTex->target, forgTex->ID);

				glUniformMatrix4fv(glGetUniformLocation(forg3DShader->id(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
				glUniform2ui(glGetUniformLocation(forg3DShader->id(), "texSize"), 8, 6);

				self->forgRenderer3D.render();

				// 2D stuff
				glDisable(GL_DEPTH_TEST);

				// cross-section edges
				const Shader* crossEdgesShader = ShaderManager::get("blockCrossSectionEdgesShader");
				crossEdgesShader->use();

				glUniformMatrix4fv(glGetUniformLocation(crossEdgesShader->id(), "MV2D"), 1, GL_FALSE, &viewCS[0][0]);
				glUniformMatrix4fv(glGetUniformLocation(crossEdgesShader->id(), "MV"), 1, GL_FALSE, &MVm4[0][0]);
				glUniformMatrix4fv(glGetUniformLocation(crossEdgesShader->id(), "P"), 1, GL_FALSE, &StateTitleScreen::instanceObj.projection3D[0][0]);
				glUniform4f(glGetUniformLocation(crossEdgesShader->id(), "inColor"), 1, 1, 1, 1);

				self->mr3d.render();

				// forg outline
				forg3DShader->use();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(forgTex->target, forgTex->ID);
				glUniformMatrix4fv(glGetUniformLocation(forg3DShader->id(), "MVP"), 1, GL_FALSE, &MVP[0][0]);
				self->forgOutlineRenderer3D.render();
			}

			glEnable(GL_DEPTH_TEST);
			renderUIPlane();
			drawHands();
			glDisable(GL_DEPTH_TEST);

			glDepthMask(GL_TRUE);
		};

	if (HMD)
	{
		// update forg anim
		{
			double t = glfwGetTime() - self->player.stateStartTime;

			int x = 0, y = 0;

			switch (self->player.currentState)
			{
			case StateTutorial::PLAYER_IDLE:
			{
				x = ((int)(t * 2) % 3);
				y = 0;
			} break;
			case StateTutorial::PLAYER_WALKING:
			{
				x = ((int)(t * 8) % 3);
				y = 1;
			} break;
			case StateTutorial::PLAYER_FALLING:
			{
				x = 3;
				y = 0;
			} break;
			}

			x += self->player.currentDirection == StateTutorial::PLAYER_DIRECTION_LEFT ? 4 : 0;

			self->setForgSprite(x, y);
		}

		updateHMDMatrixPose({ 0,0,0,1 });

		// left eye
		eyeL.use();
		glViewport(0, 0, eyeW, eyeH);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		curEye = VEye_Left;
		updateProjection(eyeW, eyeH);

		vrRender();

		// right eye
		eyeR.use();
		glViewport(0, 0, eyeW, eyeH);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		curEye = VEye_Right;
		updateProjection(eyeW, eyeH);

		vrRender();

		// send textures to OpenVR (aka SteamVR)
		eyeL.drawEye(Eye_Left);
		eyeR.drawEye(Eye_Right);

		VRCompositor()->PostPresentHandoff();

		// desktop view
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (desktopView)
		{
			curEye = VEye_PC;
			int width, height;
			glfwGetWindowSize(s.window, &width, &height);
			glViewport(0, 0, width, height);
			updateProjection(width, height);

			vrRender();
		}

		// render ui into a texture
		renderingUI = true;
		updateProjection(WIDTH_UI, HEIGHT_UI);
		uiFB.use();
		updateProjection(WIDTH_UI, HEIGHT_UI);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		{
			self->qr.setQuadRendererMode(GL_TRIANGLES);
			self->qr.setColor(0, 0, 0, 0.25f);
			self->qr.setPos(0, 0, WIDTH_UI, HEIGHT_UI);
			self->qr.render();

			// render the 2d forg view
			{
				self->updateProjection({ WIDTH_UI - 40, HEIGHT_UI - 40 }, { 20, 20 });
				glViewport(20, 20, WIDTH_UI - 40, HEIGHT_UI - 40);
				
				glm::mat4 MV2D = glm::inverse(self->player.orientation);
				MV2D[0] *= -1;
				MV2D[2] *= -1;
				MV2D = glm::translate(MV2D, -self->player.pos);

				const float scale = 9.f;
				int w = ((float)HEIGHT_UI - 40.f) / scale;
				// the world
				{
					const Shader* blockShader = ShaderManager::get("blockTutorial2DShader");
					blockShader->use();

					const Tex2D* tiles = ResourceManager::get("tiles.png");
					glBindTexture(tiles->target, tiles->ID);

					glUniform2ui(glGetUniformLocation(blockShader->id(), "texSize"), 96, 16);
					glUniformMatrix4fv(glGetUniformLocation(blockShader->id(), "MV2D"), 1, false, &MV2D[0][0]);
					glm::mat4 erm = glm::ortho(
						-scale * 0.5f * ((float)WIDTH_UI / (float)HEIGHT_UI) - 0.2f * 1.5f,
						scale * 0.5f * ((float)WIDTH_UI / (float)HEIGHT_UI) - 0.2f * 1.5f,
						-scale * 0.5f + 0.2f * 3.85f,
						scale * 0.5f + 0.2f * 3.85f,
						-1.f, 1.f);
					//erm = glm::translate(erm, glm::vec3{ h / -2, h / -2, 0.f });
					glUniformMatrix4fv(glGetUniformLocation(blockShader->id(), "MVP"), 1, false, &erm[0][0]);

					self->mr2d.render();
				}
				// forg
				{
					self->forgRenderer.setPos(
						(WIDTH_UI - 40) / 2 - w / 2,
						(HEIGHT_UI - 40) / 2 - w / 2,
						w, w);
					self->forgRenderer.render();
				}
			}

			self->updateProjection({ WIDTH_UI, HEIGHT_UI }, { 0, 0 });
			glViewport(0, 0, WIDTH_UI, HEIGHT_UI);

			self->ui.render();
			pointCursor();
		}
	}
}

$hook(void, StateTutorial, mouseInput, StateManager& s, double xpos, double ypos) { self->ui.mouseInput(xpos, ypos); }
$hook(void, StateTutorial, mouseButtonInput, StateManager& s, int button, int action, int mods) { self->ui.mouseButtonInput(button, action, mods); }

// credits
$hook(void, StateCredits, update, StateManager& s, double dt)
{
	StateTitleScreen::instanceObj.update(s, dt);
	original(self, s, dt);
	bool exit = getActionBool(Actions.Press);
	if (exit)
	{
		s.popState();
	}
}
$hook(void, StateCredits, render, StateManager& s)
{
	renderingUI = false;

	if (!uiFB.FBO)
		uiFB = Framebuffer3D(WIDTH_UI, HEIGHT_UI, true);

	static const std::function<void()> vrRender = [=]
		{
			glDepthMask(GL_TRUE);
			glDisable(GL_DEPTH_TEST);

			int width, height;
			glfwGetWindowSize(s.window, &width, &height);
			self->qr.setQuadRendererMode(4);
			self->qr.setColor(0, 0, 0, 1);
			self->qr.setPos(0, 0, width, height);
			self->qr.render();

			use3DOrientation = false;

			glEnable(GL_DEPTH_TEST);
			renderUIPlane();
			drawHands();
			glDisable(GL_DEPTH_TEST);

			glDepthMask(GL_TRUE);
		};

	if (HMD)
	{
		updateHMDMatrixPose({ 0,0,0,1 });

		// left eye
		eyeL.use();
		glViewport(0, 0, eyeW, eyeH);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		curEye = VEye_Left;
		updateProjection(eyeW, eyeH);

		vrRender();

		// right eye
		eyeR.use();
		glViewport(0, 0, eyeW, eyeH);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		curEye = VEye_Right;
		updateProjection(eyeW, eyeH);

		vrRender();

		// send textures to OpenVR (aka SteamVR)
		eyeL.drawEye(Eye_Left);
		eyeR.drawEye(Eye_Right);

		VRCompositor()->PostPresentHandoff();

		// desktop view
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (desktopView)
		{
			curEye = VEye_PC;
			int width, height;
			glfwGetWindowSize(s.window, &width, &height);
			glViewport(0, 0, width, height);
			updateProjection(width, height);

			vrRender();
		}

		// render ui into a texture
		renderingUI = true;
		glm::mat4 proj2D = glm::ortho(0.f, (float)WIDTH_UI, (float)HEIGHT_UI, 0.f, -1.f, 1.f);
		updateProjection2D(proj2D);
		uiFB.use();
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, WIDTH_UI, HEIGHT_UI);

		original(self, s);

		pointCursor();
	}
	else
	{
		original(self, s);
	}
}

#endif
