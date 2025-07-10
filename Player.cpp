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

#include <deque>

#include "FontRenderer3D.h"

#include "4DKeyBinds.h"

using namespace VRStuff;

#ifndef NONVR
bool prevItemDown = false;
bool prevItemUp = false;

bool prevLMDown = false;
bool prevRMDown = false;

bool prevPauseDown = false;
bool prevInvDown = false;

bool prevLook4D = false;
glm::mat3 startCDirInv{ 1 };
float prevCYaw = 0;
float prevCRoll = 0;
float totalCYaw = 0;
float totalCRoll = 0;
float totalRotated = 0;

bool punchingL = false;
bool punchingR = false;
glm::vec4 attackDirL, attackDirR;
std::deque<glm::vec3> lVelocities;
std::deque<glm::vec3> rVelocities;
bool afterPunch = false;
float afterPunchTargetDamage = 0;

bool ::quickActionsOpen = false;

inline static void removeYZXY(Mat5& o)
{
	glm::vec4 left = *(glm::vec4*)o[0];
	glm::vec4 up = *(glm::vec4*)o[1];
	glm::vec4 forward = *(glm::vec4*)o[2];
	glm::vec4 over = *(glm::vec4*)o[3];

	glm::vec4 pl = -glm::normalize(m4::cross(forward, { 0,1,0,0 }, over));
	glm::vec4 pd = glm::normalize(m4::cross(pl, { 0,1,0,0 }, over));

	*(glm::vec4*)o[0] = pl;
	*(glm::vec4*)o[1] = { 0,1,0,0 };
	*(glm::vec4*)o[2] = pd;
}
inline static void removeYZ(Mat5& o)
{
	glm::vec4 left = *(glm::vec4*)o[0];
	glm::vec4 up = *(glm::vec4*)o[1];
	glm::vec4 forward = *(glm::vec4*)o[2];
	glm::vec4 over = *(glm::vec4*)o[3];

	glm::vec4 pd = glm::normalize(m4::cross(left, { 0,1,0,0 }, over));

	*(glm::vec4*)o[0] = left;
	*(glm::vec4*)o[1] = glm::normalize(m4::cross(pd, left, over));
	*(glm::vec4*)o[2] = pd;
}
inline static void removeXY(Mat5& o)
{
	glm::vec4 left = *(glm::vec4*)o[0];
	glm::vec4 up = *(glm::vec4*)o[1];
	glm::vec4 forward = *(glm::vec4*)o[2];
	glm::vec4 over = *(glm::vec4*)o[3];

	glm::vec4 pl = -glm::normalize(m4::cross(forward, { 0,1,0,0 }, over));

	*(glm::vec4*)o[0] = pl;
	*(glm::vec4*)o[1] = glm::normalize(m4::cross(forward, pl, over));
	*(glm::vec4*)o[2] = forward;
}


$hook(void, Player, mouseInput, GLFWwindow* window, World* world, double xpos, double ypos)
{
	//original(self, window, world, xpos, ypos);
}

$hook(void, Player, renderHud, GLFWwindow* window)
{
	//if (drawHud)
		//original(self, window);
}

$hook(bool, Player, headIsInBlock)
{
	return false;
}

$hook(void, ItemBlock, postAction, World* world, Player* player, int action)
{
	if (player == &StateGame::instanceObj.player)
	{
		static double lastHapticTime = -1;
		if (glfwGetTime() - lastHapticTime > 0.2)
		{
			VR::addHapticEvent(VR::CONTROLLER_RIGHT,
				new VR::HapticPulse(6, 4, 0.2f, 0.0f, 0.08f));
			lastHapticTime = glfwGetTime();
		}
	}

	return original(self, world, player, action);
}

$hook(void, Player, hitTargetBlock, World* world)
{
	int targetDamageOld = (int)glm::floor(self->targetDamage * 4.0f);
	original(self, world);

	if (self == &StateGame::instanceObj.player)
	{
		if ((int)glm::floor(self->targetDamage * 4.0f) > targetDamageOld)
		{
			VR::addHapticEvent(afterPunch ? (punchingL ? VR::CONTROLLER_LEFT : VR::CONTROLLER_RIGHT) : VR::CONTROLLER_RIGHT,
				new VR::HapticPulse(1, 0, 0.9f, 0.0f, 0.08f));
		}
		else if (afterPunch)
		{
			VR::addHapticEvent(punchingL ? VR::CONTROLLER_LEFT : VR::CONTROLLER_RIGHT,
				new VR::HapticPulse(1, 0, 0.2f, 0.0f, 0.08f));
		}
	}
}

$hook(void, Player, update, World* world, double dt, EntityPlayer* entityPlayer)
{
	// check if `self` is the local player. if not then dont do anything
	if (self != &StateGame::instanceObj.player) return original(self, world, dt, entityPlayer);;
	//handleEvents();

	double time = glfwGetTime();

	static double oldHP = self->maxHealth;
	if (self->health < oldHP && time - self->damageTime <= Player::DAMAGE_COOLDOWN)
	{
		auto a = new VR::HapticPulse(40, 10, 1.0f, 0.0f, 0.12f);
		auto b = new VR::HapticPulse(30, 10, 0.9f, 0.0f, 0.13f);
		b->startTime += 0.1f;
		auto c = new VR::HapticPulse(20, 10, 0.8f, 0.0f, 0.14f);
		c->startTime += 0.1f;
		auto d = new VR::HapticPulse(10, 0, 0.7f, 0.0f, 0.15f);
		d->startTime += 0.1f;

		VR::addHapticEvent(VR::CONTROLLER_LEFT, a);
		VR::addHapticEvent(VR::CONTROLLER_RIGHT, b);
		VR::addHapticEvent(VR::CONTROLLER_LEFT, c);
		VR::addHapticEvent(VR::CONTROLLER_RIGHT, d);
	}
	oldHP = self->health;

	self->angleToRotate = 0;

	// head orientation sync
	{
		static double lastHeadSyncTime = 0;

		if (time - lastHeadSyncTime > 0.45)
		{
			world->localPlayerEvent(self, Packet::C_LOOK, 0, &orientation);

			lastHeadSyncTime = time;
		}
	}

	bool pauseDown = getActionBool(Actions.Pause);
	if (!self->inventoryManager.isOpen() && pauseDown && !prevPauseDown && !quickActionsOpen)
	{
		if (stateManager->states.back() == &StateGame::instanceObj)
		{
			self->targetDamage = 0.f;
			self->keys.rightMouseDown = false;
			self->keys.leftMouseDown = false;
			world->localPlayerEvent(self, Packet::C_HITTING_STOP, NULL, NULL);
			world->localPlayerEvent(self, Packet::C_ACTION_STOP, NULL, NULL);
			stateManager->pushState(&StatePause::instanceObj);
		}
		else
			stateManager->popState();
	}
	prevPauseDown = pauseDown;

	if (stateManager->states.back() != &StateGame::instanceObj) return;

	bool invDown = getActionBool(Actions.Inventory);
	if (invDown && !prevInvDown && !quickActionsOpen)
	{
		if (self->inventoryManager.isOpen())
		{
			self->inventoryManager.primary = nullptr;
			self->inventoryManager.secondary = nullptr;
		}
		else
		{
			self->targetDamage = 0.f;
			self->keys.rightMouseDown = false;
			self->keys.leftMouseDown = false;
			world->localPlayerEvent(self, Packet::C_HITTING_STOP, NULL, NULL);
			world->localPlayerEvent(self, Packet::C_ACTION_STOP, NULL, NULL);
			self->inventoryManager.primary = &self->hotbar;
			self->inventoryManager.secondary = &self->inventoryAndEquipment;
			if (self->inventoryManager.isOpen())
			{
				self->inventoryManager.craftingMenu.updateAvailableRecipes();
				self->inventoryManager.updateCraftingMenuBox();
			}
			world->localPlayerEvent(self, Packet::C_INVENTORY_OPEN, 0, nullptr);
		}
	}
	prevInvDown = invDown;

	if (self->inventoryManager.isOpen()) return;

	bool rightMouseDown = getActionBool(Actions.Use);
	if (prevRMDown != rightMouseDown)
	{
		self->keys.rightMouseDown = !prevRMDown && rightMouseDown;
		if (prevRMDown && !rightMouseDown)
		{
			if (!self->inventoryManager.isOpen())
				world->localPlayerEvent(self, Packet::C_ACTION_STOP, NULL, NULL);
			self->alreadyPlaced = false;
		}
		else
		{
			self->mouseDownTime = glfwGetTime();

			if (!self->inventoryManager.isOpen())
			{
				self->rightClickActionTime = 0.0f;
				world->localPlayerEvent(self, Packet::C_ACTION_START, NULL, NULL);

				VR::addHapticEvent(VR::CONTROLLER_RIGHT,
					new VR::HapticPulse(6, 4, 0.2f, 0.0f, 0.08f));
			}
		}
		prevRMDown = rightMouseDown;
	}

	bool leftMouseDown = getActionBool(Actions.Break);
	printf("%i;%i\n", prevLMDown, leftMouseDown);
	if (leftMouseDown)
	{
		if (afterPunch)
		{
			world->localPlayerEvent(self, Packet::C_BLOCK_BREAK_CANCEL, NULL, NULL);
			afterPunch = false;
			afterPunchTargetDamage = 0;
		}
	}
	if (prevLMDown != leftMouseDown)
	{
		self->keys.leftMouseDown = !prevLMDown && leftMouseDown;
		if (!prevLMDown && leftMouseDown) // PRESS
		{
			self->mouseDownTime = glfwGetTime();

			if (!self->inventoryManager.isOpen())
			{
				world->localPlayerEvent(self, Packet::C_HITTING_START, NULL, NULL);
			}
		}
		else if (prevLMDown && !leftMouseDown) // RELEASE
		{
			world->localPlayerEvent(self, Packet::C_HITTING_STOP, NULL, NULL);
		}
		prevLMDown = leftMouseDown;
	}

	bool crouchDown = getActionBool(Actions.Crouch);
	bool sprintDown = getActionBool(Actions.Sprint);
	bool jumpDown = getActionBool(Actions.Jump);

	float strafeW = (getActionBool(Actions.StrafeW) && !quickActionsOpen ? -1.f : 0.f) + (getActionBool(Actions.StrafeWP) && !quickActionsOpen ? 1.f : 0.f);

	bool itemDown = getActionBool(Actions.ItemDown) && !quickActionsOpen;
	bool itemUp = getActionBool(Actions.ItemUp) && !quickActionsOpen;

	if (itemUp && !prevItemUp)
	{
		self->hotbar.selectedIndex--;
		if (self->hotbar.selectedIndex < 0) self->hotbar.selectedIndex = self->hotbar.size.y - 1;
		world->localPlayerEvent(self, Packet::C_HOTBAR_SLOT_SELECT, self->hotbar.selectedIndex, NULL);
	}
	prevItemUp = itemUp;
	if (itemDown && !prevItemDown)
	{
		self->hotbar.selectedIndex++;
		if (self->hotbar.selectedIndex >= self->hotbar.size.y) self->hotbar.selectedIndex = 0;
		world->localPlayerEvent(self, Packet::C_HOTBAR_SLOT_SELECT, self->hotbar.selectedIndex, NULL);
	}
	prevItemDown = itemDown;

	glm::vec4 pd = glm::normalize(m4::cross(self->left, { 0, 1, 0, 0 }, self->over));
	glm::vec4 left = glm::normalize(m4::cross(pd, { 0, 1, 0, 0 }, self->over));

	bool look4D = getActionBool(Actions.Look4D);
	if (look4D && HMD->IsTrackedDeviceConnected(rCInd))
	{
		glm::mat3 controller = trackedPoses[rCInd];
		glm::vec3 cForward = controller * glm::vec3{ 0,0,-1 };
		glm::vec3 cUp = controller * glm::vec3{ 0,1,0 };
		//glm::vec3 cRight = glm::vec3(cDir[0][0], cDir[1][0], cDir[2][0]);

		if (!prevLook4D) // if just started rotating
		{
			startCDirInv = glm::inverse(controller);
			prevCYaw = 0;
			prevCRoll = 0;
			totalCYaw = 0;
			totalCRoll = 0;
			totalRotated = 0;
		}

		cForward = startCDirInv * cForward;
		cUp = startCDirInv * cUp;
		//cRight = startCDirInv * cRight;

		glm::vec3 angVel = trackedAngularVelocities[rCInd];

		//float pitch = asin(cForward.y);
		//float yaw = asin(cRight.z);
		//float roll = asin(cRight.y);
		float yaw = dot(angVel, cUp);
		float roll = dot(angVel, cForward);

		constexpr float angleMod = 1.f;
		constexpr float startThreshold2 = glm::radians(6.f);
		constexpr float startThreshold = glm::radians(20.f);
		constexpr float lowSpeed = glm::radians(20.f);
		constexpr float highSpeed = glm::radians(30.f);

		float xwAngle = 0;
		float zwAngle = 0;

		if (fabsf(totalCYaw) > startThreshold)
		{
			float inter = std::lerp(prevCYaw, totalCYaw, 0.2f);
			xwAngle = glm::min((fabsf(inter) - startThreshold + lowSpeed), highSpeed) * glm::sign(inter) * dt;

			//self->hyperplaneUpdateFlag = true;
		}
		else if (fabsf(totalCYaw) >= startThreshold2)
		{
			xwAngle = yaw * dt;

			//self->hyperplaneUpdateFlag = true;
		}

		if (fabsf(totalCRoll) > startThreshold)
		{
			float inter = std::lerp(prevCRoll, totalCRoll, 0.2f);
			zwAngle = glm::min((fabsf(inter) - startThreshold + lowSpeed), highSpeed) * glm::sign(inter) * dt;

			//self->hyperplaneUpdateFlag = true;
		}
		else if (fabsf(totalCRoll) >= startThreshold2)
		{
			zwAngle = roll * dt;

			//self->hyperplaneUpdateFlag = true;
		}

		m4::Mat5 rotMat
		{
			m4::Rotor(m4::wedge(left,self->over), zwAngle * angleMod) *
			m4::Rotor(m4::wedge(self->over,-pd), xwAngle * angleMod)
		};

		totalRotated += zwAngle + xwAngle;
		if (totalRotated > glm::radians(1.0f))
		{
			self->hyperplaneUpdateFlag = true;
			totalRotated = 0;
		}
		self->orientation = rotMat * self->orientation;

		orientation = self->orientation * orientation3D;

		self->left = *(glm::vec4*)orientation[0];
		self->left = glm::normalize(self->left);
		self->over = *(glm::vec4*)orientation[3];
		self->over = glm::normalize(self->over);
		pd = glm::normalize(m4::cross(self->left, { 0, 1, 0, 0 }, self->over));
		left = glm::normalize(m4::cross(pd, { 0, 1, 0, 0 }, self->over));

		//prevCPitch = pitch;
		prevCYaw = totalCYaw;
		prevCRoll = prevCRoll;

		totalCYaw += yaw * dt;
		totalCRoll += roll * dt;
	}
	if (prevLook4D && !look4D)
	{
		self->hyperplaneUpdateFlag = true;
	}
	prevLook4D = look4D;

	// update compass
	CompassRenderer::left = left;
	CompassRenderer::over = self->over;
	CompassRenderer::pd = pd;

	self->keys.leftMouseDown = leftMouseDown;
	self->keys.rightMouseDown = rightMouseDown;
	self->keys.space = getActionBool(Actions.Jump);
	//self->crouching = self->keys.shift = GetActionBool(Actions.Crouch);
	//self->sprinting = self->keys.ctrl = GetActionBool(Actions.Sprint);

	self->crouching = self->keys.shift = /*touchingGround && */crouchDown;
	self->keepOnEdge = self->crouching;
	self->sprinting = !self->crouching && sprintDown;

	glm::vec2 movementInput = getActionVec2(Actions.Movement);

	// movement

	// friction and acceleration
	float currentFric = Player::FRIC * dt;
	float currentAcc = Player::ACC * dt;
	float maxSpeed = Player::MAX_WALK_SPEED;
	if (self->crouching)
	{
		currentAcc *= 0.7f;
		maxSpeed = Player::MAX_CROUCH_SPEED;
	}
	else if (self->sprinting)
	{
		currentAcc *= 2;
		maxSpeed = Player::MAX_SPRINT_SPEED;
	}

	if (self->groundBlock == BlockInfo::ICE || self->groundBlock == BlockInfo::GLASS)
	{
		currentFric *= 0.2f;
	}

	// handle movement

	glm::vec4 horizMovementDir{ 0 };
	if (!self->inventoryManager.isOpen())
	{
		// vector pointing from the left side of the player
		movementInput = glm::clamp(movementInput, -1.0f, 1.0f);

		horizMovementDir = movementInput.x * left + movementInput.y * pd + strafeW * self->over;

		if (strafeW != 0)
			self->hyperplaneUpdateFlag = true;

		/*if (jumpDown)
		{
			if (self->touchingGround)
			{
				self->vel.y = Player::JUMP_VEL;
				self->deltaVel.y = 0;
				world->localPlayerEvent(self, Packet::C_JUMP, 0, nullptr);
			}
		}*/

		// horizontal player movement
		if (glm::length(horizMovementDir) != 0)
		{
			// movement acceleration
			glm::vec4 movementAcc = glm::normalize(horizMovementDir) * glm::clamp(glm::length(movementInput) + glm::abs(strafeW), 0.f, 1.f) * currentAcc;
			float movementAccLen = glm::length(movementAcc);

			// horizontal velocity after deltaVel is applied
			glm::vec4 horizVel = glm::vec4{ self->vel.x, 0, self->vel.z, self->vel.w } + glm::vec4{ self->deltaVel.x, 0, self->deltaVel.z, self->deltaVel.w };
			float horizVelLen = glm::length(horizVel);

			glm::vec4 totalHorizVel = horizVel + movementAcc;
			float totalHorizVelLen = glm::length(totalHorizVel);

			// make sure totalVel + movementAcc doesn't exceed the maximum walking speed
			if (totalHorizVelLen < maxSpeed || totalHorizVelLen < horizVelLen)
			{
				self->deltaVel += movementAcc;
			}
			// this shit breaks movement completely
			/*else if (horizVelLen < maxSpeed)
			{
				// calculate how much of movementAcc can be added to horizVel without exceeding maxWalkSpeed
				glm::vec4 intersection = m4::adjustToMaxHorizSpeed(horizVel, movementAcc, maxSpeed);

				// glm::length(intersection) should be equal to radius
				//std::cout << glm::length(intersection) << std::endl;

				// adjust deltaVel so that vel + deltaVel = intersection
				self->deltaVel += intersection - horizVel;
			}*/
		}
	}

	// detecting punching/swinging
	
	// punching
	{
		if (HMD->IsTrackedDeviceConnected(rCInd))
		{
			glm::vec3 vel = trackedVelocities[rCInd];

			rVelocities.emplace_back(vel);

			if (rVelocities.size() > punchFrames)
			{
				rVelocities.pop_front();

				glm::vec3 totalAcc{ 0.f };
				for (int i = 1; i < punchFrames; i++)
				{
					glm::vec3 acc = (rVelocities[i] - rVelocities[i - 1]) / (float)dt;

					totalAcc += acc;
				}

				glm::vec3 avAcc = totalAcc / (float)punchFrames;
				float accMag = glm::length(avAcc);

				if (accMag > punchAccelerationThreshold)
				{
					// punch detected
					static int falsePositives = 0;

					falsePositives++;
					static double lastPunchTime = 0.f;
					static double lastFalsePositiveTime = time;

					if (time - lastFalsePositiveTime > punchTimeout * 2)
						falsePositives = 0;

					lastFalsePositiveTime = time;
					if (falsePositives > punchFalsePositivesMax && time - lastPunchTime > punchTimeout)
					{
						lastPunchTime = time;
						falsePositives = 0;
						glm::vec3 dir = avAcc / accMag;
						punchingR = true;
						attackDirR = self->orientation * glm::vec4(-dir.x, dir.y, -dir.z, 0.f) * 0.75f;
					}
				}
			}
		}
		if (HMD->IsTrackedDeviceConnected(lCInd))
		{
			glm::vec3 vel = trackedVelocities[lCInd];

			lVelocities.emplace_back(vel);

			if (lVelocities.size() > punchFrames)
			{
				lVelocities.pop_front();

				glm::vec3 totalAcc{ 0.f };
				for (int i = 1; i < punchFrames; i++)
				{
					glm::vec3 acc = (lVelocities[i] - lVelocities[i - 1]) / (float)dt;

					totalAcc += acc;
				}

				glm::vec3 avAcc = totalAcc / (float)punchFrames;
				float accMag = glm::length(avAcc);

				if (accMag > punchAccelerationThreshold)
				{
					// punch detected
					static int falsePositivesl = 0;

					falsePositivesl++;
					static double lastPunchTimel = 0.f;
					static double lastFalsePositiveTimel = time;

					if (time - lastFalsePositiveTimel > punchTimeout * 2)
						falsePositivesl = 0;

					lastFalsePositiveTimel = time;
					if (falsePositivesl > punchFalsePositivesMax && time - lastPunchTimel > punchTimeout)
					{
						lastPunchTimel = time;
						falsePositivesl = 0;
						glm::vec3 dir = avAcc / accMag;
						punchingL = true;
						attackDirL = self->orientation * glm::vec4(-dir.x, dir.y, -dir.z, 0.f) * 0.75f;
					}
				}
			}
		}
	}

	// mod compat stuff
	if (HMD->IsTrackedDeviceConnected(rCInd))
	{
		glm::mat4 controller = trackedPoses[rCInd];
		controller = glm::rotate(glm::identity<glm::mat4>(), controllerYZAngle, { 1, 0, 0 }) * controller;
		controller = glm::transpose(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.01f, 0.f, 0.07f))) * controller;
		glm::mat3 dir = glm::inverse(glm::mat3
			{
				controller[0][0],controller[1][0],controller[2][0],
				controller[0][1],controller[1][1],controller[2][1],
				controller[0][2],controller[1][2],controller[2][2]
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
		mat[4][0] = -controller[0][3];
		mat[4][1] = controller[1][3];
		mat[4][2] = -controller[2][3];
		mat = self->orientation * mat;
		glm::vec4 controllerPos = *(glm::vec4*)mat[4];
		self->cameraPos = self->pos + controllerPos - hmd4DPos;
		self->forward = *(glm::vec4*)mat[1]; // technically thats the up direction but thats actually what you want there
	}

	// 4dkeybinds actions
	for (auto& a : keybindsActions)
	{
		a.second.lastState = a.second.curState;
		getInputBool(a.first, &a.second.curState);

		if (a.second.curState && !a.second.lastState)
		{
			KeyBinds::triggerBind(a.second.ns, a.second.name, KeyBindsScope::PLAYER, GLFW_PRESS, 0);
		}
		else if (!a.second.curState && a.second.lastState)
		{
			KeyBinds::triggerBind(a.second.ns, a.second.name, KeyBindsScope::PLAYER, GLFW_RELEASE, 0);
		}
	}

	original(self, world, dt, entityPlayer);
}

$hook(bool, Player, keyInput, GLFWwindow* window, World* world, int key, int scancode, int action, int mods)
{
	glm::vec4 oldPos = self->pos;

	bool result = original(self, window, world, key, scancode, action, mods);
	
	if (oldPos != self->pos)
	{
		removeYZXY(self->orientation);
		lastHMDPos = { 0,0,0,0 };
	}

	return result;
}

$hook(void, Player, updateTargetBlock, World* world, float maxDist)
{
	glm::vec4 cameraPos = self->cameraPos;
	glm::vec4 forward = self->forward;
	glm::vec4 pos = self->pos;

	TrackedDeviceIndex_t dev = punchingL ? lCInd : rCInd;
	if (HMD->IsTrackedDeviceConnected(dev))
	{
		glm::mat4 controller = trackedPoses[dev];
		controller = glm::rotate(glm::identity<glm::mat4>(), controllerYZAngle, { 1, 0, 0 }) * controller;
		controller = glm::transpose(glm::translate(glm::identity<glm::mat4>(), glm::vec3(dev == lCInd ? -0.01f : 0.01f, 0.f, 0.07f))) * controller;
		glm::mat3 dir = glm::inverse(glm::mat3
			{
				controller[0][0],controller[1][0],controller[2][0],
				controller[0][1],controller[1][1],controller[2][1],
				controller[0][2],controller[1][2],controller[2][2]
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

		mat[4][0] = -controller[0][3];
		mat[4][1] = controller[1][3];
		mat[4][2] = -controller[2][3];

		mat = self->orientation * mat;

		glm::vec4 controllerPos = *(glm::vec4*)mat[4];
		self->cameraPos = self->pos + controllerPos - hmd4DPos;
		self->pos = self->cameraPos;

		if (punchingR)
		{
			float l = glm::length(attackDirR);
			float lI = 1.f / l;
			self->forward = attackDirR * lI;
			self->cameraPos -= self->forward * 0.15f;
			maxDist = l + 0.15f;
		}
		else if (punchingL)
		{
			float l = glm::length(attackDirL);
			float lI = 1.f / l;
			self->forward = attackDirL * lI;
			self->cameraPos -= self->forward * 0.15f;
			maxDist = l + 0.15f;
		}
		else if (!afterPunch)
		{
			self->forward = *(glm::vec4*)mat[1]; // technically thats the up direction but thats actually what you want there
		}

		/*printf("controller pos: %f; %f; %f; %f;\ncontroller pos multiplied by mR: %f; %f; %f; %f;\ncontroller dir: %f; %f; %f; %f;\n",
			controllerPos.x, controllerPos.y, controllerPos.z, controllerPos.w,
			controllerPosMR.x, controllerPosMR.y, controllerPosMR.z, controllerPosMR.w,
			self->forward.x, self->forward.y, self->forward.z, self->forward.w);*/
	}

	bool punchedAir = false;
	if (!afterPunch || punchingR || punchingL)
	{
		glm::ivec4 targetBlockOld = self->targetBlock;
		bool targetingBlockOld = self->targetingBlock;
		uint8_t blockOld = world->getBlock(self->targetBlock);
		original(self, world, maxDist);
		uint8_t blockNew = world->getBlock(self->targetBlock);
		if (afterPunch && !self->targetingBlock)
		{
			self->targetDamage = afterPunchTargetDamage;
			self->targetBlock = targetBlockOld;
			self->targetingBlock = true;
			punchedAir = true;
		}
		else if (afterPunch && self->targetingBlock && targetBlockOld != self->targetBlock)
		{
			self->targetDamage = 0;
			afterPunchTargetDamage = 0;
		}
	}

	if (punchingR || punchingL)
	{
		world->localPlayerEvent(self, Packet::C_HITTING_START, NULL, NULL);
		glm::vec4 endPoint = self->cameraPos + self->forward * maxDist * 2.5f;
		Entity* ent = world->getEntityIntersection(self->cameraPos, endPoint, self->EntityPlayerID);
		if (ent)
		{
			//Item* item = nullptr;
			//item = self->hotbar.getSlot(self->hotbar.selectedIndex)->get();
			//if (item)
			//	item->entityAction(world, self, ent, 0);
			world->localPlayerEvent(self, Packet::C_ITEM_ACTION_ENTITY, 0, ent);

			VR::addHapticEvent(punchingL ? VR::CONTROLLER_LEFT : VR::CONTROLLER_RIGHT,
				new VR::HapticPulse(1, 0, 0.9f, 0.0f, 0.08f));
			goto nope;
		}
		float targetDamageOld = self->targetDamage;
		if (!punchedAir)
		{
			bool oldAfterPunch = afterPunch;
			afterPunch = true;
			self->hitTargetBlock(world);
			afterPunch = oldAfterPunch;
		}
		self->targetDamage = targetDamageOld + (self->targetDamage - targetDamageOld) * 15.f;
		afterPunchTargetDamage = self->targetDamage;
		if (targetDamageOld == 0.f && self->targetDamage > 0.f)
		{
			afterPunch = true;
			world->localPlayerEvent(self, Packet::C_BLOCK_BREAK_START, NULL, NULL);
		}
		if (self->targetDamage >= 1.f)
		{
			world->localPlayerEvent(self, Packet::C_BLOCK_BREAK_FINISH, NULL, NULL);
			afterPunch = false;
			afterPunchTargetDamage = 0;
		}

		punchingR = false;
		punchingL = false;
		world->localPlayerEvent(self, Packet::C_HITTING_STOP, NULL, NULL);
	}
	nope:
	self->cameraPos = cameraPos;
	self->pos = pos;
	self->forward = forward;
}

#endif

class HapticSolenoidCollector : public VR::Haptic
{
public:
	bool on = true;
	HapticSolenoidCollector()
	{
		duration = INFINITY;
	}
	float frequency(double time) override
	{
		if (!on)
			return 0.0f;
		return glm::mix(2.0f, 10.0f, glm::sin(time * 5.0f) * 0.5f + 0.5f);
	}
	float amplitude(double time) override
	{
		if (!on)
			return 0.0f;
		return glm::mix(0.4f, 0.3f, glm::sin(time * 7.5f) * 0.5f + 0.5f);
	}
};

void adjustItemMat(Mat5& MV, Item* item, uint8_t hand = 0, bool onlinePlayer = false)
{
	const bool leftHand = hand == 0;

	glm::vec4 translation = glm::vec4(0.0075f * (leftHand ? -1 : 1), onlinePlayer ? 0.f : -0.07f, 0.0025f, 0.f);
	glm::vec4 scale = glm::vec4(ITEM_SCALE);

	if (onlinePlayer)
		MV.scale({ 1,1,-1,1 });

	if (item->getName() == "Solenoid Collector")
	{
		if (leftHand)
			MV *= rotorZ22p5;
		else
			MV *= rotorZ22p5N;
		MV *= rotorY;
		if(leftHand)
			MV *= rotorY180;

		translation += glm::vec4(-0.0175f * (leftHand ? -1 : 1), 0.13f, -0.045f, 0.f);
#ifndef NONVR
		if (!onlinePlayer && !leftHand)
		{
			static HapticSolenoidCollector haptic{};
			haptic.destroy = false;
			ItemTool::collectorAnimation = prevRMDown;

			if (!VR::isHapticActive(VR::CONTROLLER_RIGHT, &haptic))
			{
				VR::addHapticEvent(VR::CONTROLLER_RIGHT, &haptic);
			}
			
			haptic.on = ItemTool::collectorAnimation;
		}
#endif
	}
	else if (item->getName() == "4D Glasses")
	{
		MV *= rotorX45N;
		translation += glm::vec4(-0.19f * (leftHand ? -1 : 1), -0.05f, -0.08f, 0.f);
	}
	else if (item->getName() == "Ultrahammer")
	{
		translation += glm::vec4(0.f, 0.07f, 0.f, 0.f);
	}
	else if (item->getName() == "Hypersilk")
	{
		translation += glm::vec4(0.f, -0.01f, -0.06f, -0.22f);
		scale *= 0.4f;
	}
	else if (item->getName() == "Klein Bottle" || item->getName() == "Health Potion" || item->getName() == "Jellyfish Juice")
	{
		translation += glm::vec4(0.f, 0.1f, -0.05f, -0.05f);
		MV *= rotorY180;
		scale *= 0.55f;
	}
	else if (((std::string)item->getName()).ends_with("Bars"))
	{
		translation += glm::vec4(-0.1f, 0.f, 0.f, 0.f);
		MV *= rotorZ;
		scale *= 0.9f;
	}
	else if (item->getName() == "Rock")
	{
		translation += glm::vec4(0.f, 0.f, -0.07f, 0.f);
		scale *= 0.45f;
	}
	else if (0 == strcmp(typeid(*item).name(), "class ItemBlock"))
	{
		translation += glm::vec4(0.f, -0.01f, -0.06f, 0.f);
		scale *= 0.75f;
	}
	else if (item->getName() == "Alidade")
	{
		translation += glm::vec4(0.f, -0.125f, -0.03f, 0.f);
		scale *= 0.5f;
	}
	else if (((std::string)item->getName()).ends_with(" Lens"))
	{
		translation += glm::vec4(0.f, 0.02f, -0.06f, 0.f);
		scale *= 0.45f;
	}

	MV *= rotorXN;

	if (0 == strcmp(typeid(*item).name(), "class ItemTool"))
		MV *= Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), glm::radians(-40.f) });

	if (onlinePlayer)
		MV *= rotorX180;

	MV.translate(translation);

	MV.scale(scale);
}

#ifndef NONVR
void drawHands(StateGame* gameState)
{
	/*
	glm::vec3 hUp{ hmdPose[0][1], hmdPose[1][1], hmdPose[2][1] };
	glm::vec3 hForward{ hmdPose[0][2], hmdPose[1][2], hmdPose[2][2] };
	glm::vec3 hPos{ hmdPose[0][3], hmdPose[1][3], hmdPose[2][3] };
	glm::vec3 shoulderOffset = glm::cross({ 0, 1, 0 }, hForward) * 0.4f;
	glm::vec3 shoulderForward = glm::cross(shoulderOffset / 0.4f, {0, 1, 0}) * 0.1f;
	float shoulderHeight = 0.25f;
	*/

	//PlayerSkin& skin = StateSkinChooser::instanceObj->skin;
	glm::mat4& eyeMat = curEye == VEye_Left ? vEyeL : vEyeR;
	glm::vec4& eyePos = curEye == VEye_Left ? eyeLP : eyeRP;
	/*Mat5 view = fdmHooks::m4::createCameraH::original(
		{ eyeMat[0][3], eyeMat[1][3], eyeMat[2][3], 0.f },
		{ hmdPose[0][2], hmdPose[1][2], hmdPose[2][2], 0.f },
		{ hmdPose[0][1], hmdPose[1][1], hmdPose[2][1], 0.f },
		glm::vec4{ hmdPose[0][0], hmdPose[1][0], hmdPose[2][0], 0.f },
		{ 0.f, 0.f, 0.f, 1.f });*/

		/*const Shader* skinShader = ShaderManager::get("playerSkinShader");
		skinShader->use();
		glUniform4f(glGetUniformLocation(skinShader->id(), "lightDir"), 0.f, 1.f, 0.f, 0.f);
		glUniform3ui(glGetUniformLocation(skinShader->id(), "texSize"), ps::WIDTH, ps::HEIGHT, ps::DEPTH);
		// can be set to different colors for different effects, e.g. taking damage
		glUniform3f(glGetUniformLocation(skinShader->id(), "inColor"), 1.0f, 1.0f, 1.0f);*/

		//Mat5 skinMV = view;
		//skinMV.scale(glm::vec4{ -1, 1, -1, 1 } * skinScale);

	if (HMD->IsTrackedDeviceConnected(lCInd))
	{
		Item* item = nullptr;

		if (gameState)
			item = gameState->player.equipment.getSlot(0).get();

		Mat5 MV{ 1 };
		glm::mat4 controller = trackedPoses[lCInd];
		controller = glm::rotate(glm::identity<glm::mat4>(), controllerYZAngle, { 1, 0, 0 }) * controller;
		Mat5 mat5Controller = ToMat5(controller * glm::inverse(eyeMat), true);
		mat5Controller[4][3] = 0.001f;

		/*
		armIKL.target = { trackedPoses[lCInd][0][3], trackedPoses[lCInd][1][3], trackedPoses[lCInd][2][3] };
		armIKL.basePos = hPos - (hUp * shoulderHeight) - shoulderOffset - shoulderForward;

		glm::vec3 wristForward = { trackedPoses[lCInd][0][2], trackedPoses[lCInd][1][2], trackedPoses[lCInd][2][2] };
		glm::vec3 wristUp = { trackedPoses[lCInd][0][1], trackedPoses[lCInd][1][1], trackedPoses[lCInd][2][1] };
		glm::vec3 wristLeft = { trackedPoses[lCInd][0][0], trackedPoses[lCInd][1][0], trackedPoses[lCInd][2][0] };

		armIKL.controlTarget = approxElbow(hPos, armIKL.target, wristForward, wristLeft, wristUp, armIKL.parts[0].length, armIKL.parts[1].length, armIKL.basePos, shoulderForward / 0.1f) + glm::vec3{0.05f, -0.1f, 0.f};

		armIKL.update();
		*/

		//MV.translate({ 0.f, -0.01f, 0.f, 0.f });

		if (item != nullptr)
		{
			MV *= mat5Controller;
			adjustItemMat(MV, item, 0);
		}

		Mat5 MVhand{ 1 };

		MVhand *= mat5Controller;

		MVhand.translate(glm::vec4(-0.01f, 0.1f, 0.08f, 0.f));

		MVhand.scale(glm::vec4(0.25f, 0.55f, 0.25f, 0.25f) * 0.2f * 2.f);

		MVhand.translate(glm::vec4{ -0.5f });

		//printf("\nMV:\n");
		//m4::printMat5(MV);

		//if (!drawHud)
		{
			// render hand
			handShader->use();

			glUniform1fv(glGetUniformLocation(handShader->id(), "MV"), sizeof(MVhand) / sizeof(float), &MVhand[0][0]);
			glUniform4f(glGetUniformLocation(handShader->id(), "lightDir"), 0.f, 1.f, 0.f, 0.f);
			glUniform4f(glGetUniformLocation(handShader->id(), "inColor"), 1.f, 1.f, 1.f, 1.f);

			handRenderer.render();

			// render the item in hand
			if (item != nullptr)
			{
				item->renderEntity(MV, true, { 0, 1, 0, 0 });
			}

			/*

			{
				Mat5 upperArmL = skinMV;

				Mat5 rotY = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 0, 1, 0 }, glm::vec4{ 0, 1, 0, 0 }), (armIKL.parts[0].eulerAngle.y) });
				Mat5 rotX = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 0, 1, 0 }, glm::vec4{ 1, 0, 0, 0 }), (armIKL.parts[0].eulerAngle.x) });
				Mat5 rotZ = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), (armIKL.parts[0].eulerAngle.z) });
				Mat5 pos = Mat5(1);

				pos.scale(glm::vec4{ -1, 1, -1, 1 } / skinScale);

				pos.translate({ armIKL.parts[0].pos.x ,armIKL.parts[0].pos.y ,armIKL.parts[0].pos.z ,0 });

				pos.scale(glm::vec4{ -1, 1, -1, 1 } * skinScale);


				upperArmL = pos * upperArmL;
				upperArmL = rotX * upperArmL;
				upperArmL = rotY * upperArmL;
				upperArmL = rotZ * upperArmL;

				upperArmL.translate(glm::vec4{ -(float)ps::meshBounds[ps::UPPER_ARM_L].origin.x - 3,-(float)ps::meshBounds[ps::UPPER_ARM_L].origin.y + 10,-(float)ps::meshBounds[ps::UPPER_ARM_L].origin.z, 0 });

				skinShader->use();
				glUniform1fv(glGetUniformLocation(skinShader->id(), "MV"), sizeof(upperArmL) / sizeof(float), &upperArmL[0][0]);
				glBindTexture(GL_TEXTURE_2D, skin.tex.ID);
				skin.meshComps[ps::UPPER_ARM_L].render();
			}

			{
				Mat5 foreArmL = skinMV;

				Mat5 rotY = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 0, 1, 0 }, glm::vec4{ 0, 1, 0, 0 }), (armIKL.parts[1].eulerAngle.y) });
				Mat5 rotX = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 0, 1, 0 }, glm::vec4{ 1, 0, 0, 0 }), -(armIKL.parts[1].eulerAngle.x) });
				Mat5 rotZ = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), -(armIKL.parts[1].eulerAngle.z) });
				Mat5 pos = Mat5(1);

				pos.scale(glm::vec4{ -1, 1, -1, 1 } / skinScale);

				pos.translate({ armIKL.parts[1].pos.x ,armIKL.parts[1].pos.y ,armIKL.parts[1].pos.z ,0 });

				pos.scale(glm::vec4{ -1, 1, -1, 1 } * skinScale);


				foreArmL = pos * foreArmL;
				foreArmL = rotZ * foreArmL;
				foreArmL = rotY * foreArmL;
				foreArmL = rotX * foreArmL;

				foreArmL.translate(glm::vec4{ -(float)ps::meshBounds[ps::FOREARM_L].origin.x,-(float)ps::meshBounds[ps::FOREARM_L].origin.y + 2,-(float)ps::meshBounds[ps::FOREARM_L].origin.z, 0 });

				skinShader->use();
				glUniform1fv(glGetUniformLocation(skinShader->id(), "MV"), sizeof(foreArmL) / sizeof(float), &foreArmL[0][0]);
				glBindTexture(GL_TEXTURE_2D, skin.tex.ID);
				skin.meshComps[ps::FOREARM_L].render();
			}

			for (auto& pos : armIKL.positions)
			{
				Mat5 MVa = view;

				MVa.translate(glm::vec4{ pos.x, pos.y, pos.z, 0.0f });

				MVa.scale(glm::vec4{ 0.05f });

				MVa.translate(glm::vec4{ -0.5f });

				handShader->use();

				glUniform1fv(glGetUniformLocation(handShader->id(), "MV"), sizeof(MVa) / sizeof(float), &MVa[0][0]);
				glUniform4f(glGetUniformLocation(handShader->id(), "inColor"), 0.1f, 0.9f, 0.1f, 1.f);
				glUniform4f(glGetUniformLocation(handShader->id(), "lightDir"), 0.f, 1.f, 0.f, 0.f);

				handRenderer.render();
			}
			*/
		}
	}
	if (HMD->IsTrackedDeviceConnected(rCInd))
	{
		Item* item = nullptr;

		if (gameState)
			item = gameState->player.hotbar.getSlot(gameState->player.hotbar.selectedIndex).get();

		Mat5 MV{ 1 };
		glm::mat4 controller = trackedPoses[rCInd];
		controller = glm::rotate(glm::identity<glm::mat4>(), controllerYZAngle, { 1, 0, 0 }) * controller;
		Mat5 mat5Controller = ToMat5(controller * glm::inverse(eyeMat), true);
		mat5Controller[4][3] = 0.001f;

		/*armIKR.target = {trackedPoses[rCInd][0][3], trackedPoses[rCInd][1][3], trackedPoses[rCInd][2][3]};
		armIKR.basePos = hPos - (hUp * shoulderHeight) + shoulderOffset - shoulderForward;

		glm::vec3 wristForward = { trackedPoses[rCInd][0][2], trackedPoses[rCInd][1][2], trackedPoses[rCInd][2][2] };
		glm::vec3 wristUp = { trackedPoses[rCInd][0][1], trackedPoses[rCInd][1][1], trackedPoses[rCInd][2][1] };
		glm::vec3 wristLeft = { trackedPoses[rCInd][0][0], trackedPoses[rCInd][1][0], trackedPoses[rCInd][2][0] };

		armIKR.controlTarget = approxElbow(hPos, armIKR.target, wristForward, wristLeft, wristUp, armIKR.parts[0].length, armIKR.parts[1].length, armIKR.basePos, shoulderForward / 0.1f) + glm::vec3{ 0.05f, -0.1f, 0.f };

		armIKR.update();
		*/

		//MV.translate({ 0.f, -0.01f, 0.f, 0.f });

		if (item != nullptr)
		{
			MV *= mat5Controller;
			adjustItemMat(MV, item, 1);
		}

		Mat5 MVhand{ 1 };

		MVhand *= mat5Controller;

		MVhand.translate(glm::vec4(0.01f, 0.1f, 0.08f, 0.f));

		MVhand.scale(glm::vec4(0.25f, 0.55f, 0.25f, 0.25f) * 0.2f * 2.f);

		MVhand.translate(glm::vec4{ -0.5f });

		//printf("\nMV:\n");
		//m4::printMat5(MV);

		//if (!drawHud)
		{
			// render hand
			handShader->use();

			glUniform1fv(glGetUniformLocation(handShader->id(), "MV"), sizeof(MVhand) / sizeof(float), &MVhand[0][0]);
			glUniform4f(glGetUniformLocation(handShader->id(), "lightDir"), 0.f, 1.f, 0.f, 0.f);
			glUniform4f(glGetUniformLocation(handShader->id(), "inColor"), 1.f, 1.f, 1.f, 1.f);

			handRenderer.render();

			// render the item in hand
			if (item != nullptr)
			{
				item->renderEntity(MV, true, { 0, 1, 0, 0 });
				ItemTool::collectorAnimation = false;
			}
			/*
			{
				Mat5 upperArmR = skinMV;

				Mat5 rotY = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 0, 1, 0 }, glm::vec4{ 0, 1, 0, 0 }), (armIKR.parts[0].eulerAngle.y) });
				Mat5 rotX = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 0, 1, 0 }, glm::vec4{ 1, 0, 0, 0 }), (armIKR.parts[0].eulerAngle.x) });
				Mat5 rotZ = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), (armIKR.parts[0].eulerAngle.z) });
				Mat5 pos = Mat5(1);

				pos.scale(glm::vec4{ -1, 1, -1, 1 } / skinScale);

				pos.translate({ armIKR.parts[0].pos.x ,armIKR.parts[0].pos.y ,armIKR.parts[0].pos.z ,0 });

				pos.scale(glm::vec4{ -1, 1, -1, 1 } *skinScale);


				upperArmR = pos * upperArmR;
				upperArmR = rotX * upperArmR;
				upperArmR = rotY * upperArmR;
				upperArmR = rotZ * upperArmR;

				upperArmR.translate(glm::vec4{ -(float)ps::meshBounds[ps::UPPER_ARM_R].origin.x + 2,-(float)ps::meshBounds[ps::UPPER_ARM_R].origin.y + 10,-(float)ps::meshBounds[ps::UPPER_ARM_R].origin.z, 0 });

				skinShader->use();
				glUniform1fv(glGetUniformLocation(skinShader->id(), "MV"), sizeof(upperArmR) / sizeof(float), &upperArmR[0][0]);
				glBindTexture(GL_TEXTURE_2D, skin.tex.ID);
				skin.meshComps[ps::UPPER_ARM_R].render();
			}

			{
				Mat5 foreArmR = skinMV;

				Mat5 rotY = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 0, 1, 0 }, glm::vec4{ 0, 1, 0, 0 }), (armIKR.parts[1].eulerAngle.y) });
				Mat5 rotX = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 0, 1, 0 }, glm::vec4{ 1, 0, 0, 0 }), (armIKR.parts[1].eulerAngle.x) });
				Mat5 rotZ = Mat5(m4::Rotor{ m4::wedge(glm::vec4{ 0, 1, 0, 0 }, glm::vec4{ 0, 0, 1, 0 }), (armIKR.parts[1].eulerAngle.z) });
				Mat5 pos = Mat5(1);

				pos.scale(glm::vec4{ -1, 1, -1, 1 } / skinScale);

				pos.translate({ armIKR.parts[1].pos.x ,armIKR.parts[1].pos.y ,armIKR.parts[1].pos.z ,0 });

				pos.scale(glm::vec4{ -1, 1, -1, 1 } *skinScale);


				foreArmR = pos * foreArmR;
				foreArmR = rotX * foreArmR;
				foreArmR = rotY * foreArmR;
				foreArmR = rotZ * foreArmR;

				foreArmR.translate(glm::vec4{ -(float)ps::meshBounds[ps::FOREARM_R].origin.x,-(float)ps::meshBounds[ps::FOREARM_R].origin.y + 2,-(float)ps::meshBounds[ps::FOREARM_R].origin.z, 0 });

				skinShader->use();
				glUniform1fv(glGetUniformLocation(skinShader->id(), "MV"), sizeof(foreArmR) / sizeof(float), &foreArmR[0][0]);
				glBindTexture(GL_TEXTURE_2D, skin.tex.ID);
				skin.meshComps[ps::FOREARM_R].render();
			}

			for (auto& pos : armIKR.positions)
			{
				Mat5 MVa = view;

				MVa.translate(glm::vec4{ pos.x, pos.y, pos.z, 0.0f });

				MVa.scale(glm::vec4{ 0.05f });

				MVa.translate(glm::vec4{ -0.5f });

				handShader->use();

				glUniform1fv(glGetUniformLocation(handShader->id(), "MV"), sizeof(MVa) / sizeof(float), &MVa[0][0]);
				glUniform4f(glGetUniformLocation(handShader->id(), "inColor"), 0.1f, 0.9f, 0.1f, 1.f);
				glUniform4f(glGetUniformLocation(handShader->id(), "lightDir"), 0.f, 1.f, 0.f, 0.f);

				handRenderer.render();
			}*/
		}
	}

	// crosshair renderrerrerrrerrerrrerrerrererrrerrerrerrrerrrerrer
	if (gameState && !afterPunch)
	{
		const glm::vec4 offset = -gameState->player.pos + hmd4DPos;
		glm::vec4 reachEndpoint = m4::Mat5::inverse(gameState->player.orientation) * (gameState->player.reachEndpoint + offset);
		glm::vec4 targetBlock = glm::vec4(gameState->player.targetBlock) + offset;
		glm::vec4 targetPlaceBlock = glm::vec4(gameState->player.targetPlaceBlock) + offset;
		glm::vec4 normal = m4::Mat5::inverse(gameState->player.orientation) * (targetPlaceBlock - targetBlock);
		if (!gameState->player.targetingBlock)
		{
			normal = glm::normalize(-reachEndpoint);
		}
		
		normal.x *= -1;
		normal.z *= -1;

		Mat5 eyeMat5 = ToMat5(glm::inverse(eyeMat), true);
		Mat5 crosshairMat = eyeMat5;
		crosshairMat.translate(glm::vec4{ -reachEndpoint.x + normal.x * 0.02f, reachEndpoint.y + normal.y * 0.02f, -reachEndpoint.z + normal.z * 0.02f, -0.01f });
		Mat5 rot = m4::Mat5(m4::Rotor({ 0,1,0,0 }, glm::vec4(glm::vec3(normal), 0.f)));
		crosshairMat *= rot;
		crosshairMat.scale(glm::vec4{ 0.075f, 0.00001f, 0.075f, 1.f });
		crosshairShader->use();

		glUniform1fv(glGetUniformLocation(crosshairShader->id(), "MV"), sizeof(crosshairMat) / sizeof(float), &crosshairMat[0][0]);
		glUniform1i(glGetUniformLocation(crosshairShader->id(), "texSampler"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gameState->crosshairRenderer.texture->ID);

		//glDisable(GL_DEPTH_TEST);

		glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ZERO); // invert blending
		crosshairRenderer.render();
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // reset to normal blending

		//glEnable(GL_DEPTH_TEST);
	}
}

$hook(void, EntityAlidade, postAction, World* world, Entity* actor, int action)
{
	if (action == GLFW_PRESS)
	{
		EntityPlayer* entityPlayer = reinterpret_cast<EntityPlayer*>(actor);
		Player* player = entityPlayer->player;

		entityPlayer->setPos(self->pos);
		player->currentBlock = self->currentBlock;
		
		player->orientation = self->orientation;

		player->angleToRotate = 0;
		
		player->rightClickActionTime = glfwGetTime();

		if (player == &StateGame::instanceObj.player)
		{
			removeYZXY(player->orientation);
			lastHMDPos = { 0,0,0,0 };
		}
	}
}

$hook(void, Player, init, const glm::ivec4& pos)
{
	original(self, pos);
	if (self == &StateGame::instanceObj.player)
	{
		removeYZXY(self->orientation);
		lastHMDPos = { 0,0,0,0 };
	}
}

$hook(void, Player, load, const nlohmann::json& j)
{
	original(self, j);
	if (self == &StateGame::instanceObj.player)
	{
		removeYZXY(self->orientation);
		*(glm::vec4*)self->orientation[0] = glm::normalize(-m4::cross({ 0,0,1,0 }, { 0,1,0,0 }, *(glm::vec4*)self->orientation[3]));
		*(glm::vec4*)self->orientation[1] = { 0,1,0,0 };
		*(glm::vec4*)self->orientation[2] = glm::normalize(m4::cross(*(glm::vec4*)self->orientation[0], { 0,1,0,0 }, *(glm::vec4*)self->orientation[3]));
		lastHMDPos = { 0,0,0,0 };
	}
}

$hook(void, Player, loadClientData, const nlohmann::json& j)
{
	original(self, j);
	if (self == &StateGame::instanceObj.player)
	{
		removeYZXY(self->orientation);
		*(glm::vec4*)self->orientation[0] = glm::normalize(-m4::cross({ 0,0,1,0 }, { 0,1,0,0 }, *(glm::vec4*)self->orientation[3]));
		*(glm::vec4*)self->orientation[1] = { 0,1,0,0 };
		*(glm::vec4*)self->orientation[2] = glm::normalize(m4::cross(*(glm::vec4*)self->orientation[0], { 0,1,0,0 }, *(glm::vec4*)self->orientation[3]));
		lastHMDPos = { 0,0,0,0 };
	}
}

$hook(void, Player, respawn, World* world, const glm::ivec4& pos)
{
	original(self, world, pos);
	if (self == &StateGame::instanceObj.player)
	{
		removeYZXY(self->orientation);
		lastHMDPos = { 0,0,0,0 };
	}
}

$hook(void, ItemMaterial, action, World* world, Player* player, int action)
{
	Mat5 orr4D = player->orientation;

	Mat5 orr = orientation;
	removeXY(orr);
	player->orientation = orr;
	
	original(self, world, player, action);

	player->orientation = orr4D;
}

// fix alidade shit
$hook(nlohmann::json&, Player, getMovementUpdate, nlohmann::json* result)
{
	original(self, result);

	Mat5 orr = orientation;
	removeXY(orr);
	(*result)["orientation"] = orr.toJson();
	(*result)["orientation4D"] = self->orientation.toJson();

	return *result;
}
$hook(void, EntityPlayer, applyMovementUpdate, const nlohmann::json& j)
{
	nlohmann::json jc = j;

	if (self->player == &StateGame::instanceObj.player)
	{
		Mat5 orr = Mat5(j["orientation4D"]);

		jc["orientation"] = orr.toJson();
	}

	original(self, jc);
}

// vr sync in multiplayer

$hook(nlohmann::json&, Player, getMovementUpdate, nlohmann::json* result)
{
	original(self, result);

	Mat5 matL{ 1 };
	Mat5 matR{ 1 };

	// left hand
	{
		glm::mat4 controllerL = trackedPoses[lCInd];
		controllerL = glm::rotate(glm::identity<glm::mat4>(), controllerYZAngle, { 1, 0, 0 }) * controllerL;
		controllerL = glm::transpose(glm::translate(glm::identity<glm::mat4>(), glm::vec3(-0.01f, 0.f, 0.08f))) * controllerL;
		glm::mat3 dirL = glm::inverse(glm::mat3
			{
				controllerL[0][0],controllerL[1][0],controllerL[2][0],
				controllerL[0][1],controllerL[1][1],controllerL[2][1],
				controllerL[0][2],controllerL[1][2],controllerL[2][2]
			});


		matL[0][0] = -dirL[0][0];
		matL[0][1] = dirL[1][0];
		matL[0][2] = -dirL[2][0];

		matL[1][0] = dirL[0][1];
		matL[1][1] = -dirL[1][1];
		matL[1][2] = dirL[2][1];

		matL[2][0] = -dirL[0][2];
		matL[2][1] = dirL[1][2];
		matL[2][2] = -dirL[2][2];

		matL[4][0] = -controllerL[0][3];
		matL[4][1] = controllerL[1][3];
		matL[4][2] = -controllerL[2][3];

		matL = self->orientation * matL;
	}
	
	// right hand
	{
		glm::mat4 controllerR = trackedPoses[rCInd];
		controllerR = glm::rotate(glm::identity<glm::mat4>(), controllerYZAngle, { 1, 0, 0 }) * controllerR;
		controllerR = glm::transpose(glm::translate(glm::identity<glm::mat4>(), glm::vec3(0.01f, 0.f, 0.08f))) * controllerR;
		glm::mat3 dirR = glm::inverse(glm::mat3
			{
				controllerR[0][0],controllerR[1][0],controllerR[2][0],
				controllerR[0][1],controllerR[1][1],controllerR[2][1],
				controllerR[0][2],controllerR[1][2],controllerR[2][2]
			});


		matR[0][0] = -dirR[0][0];
		matR[0][1] = dirR[1][0];
		matR[0][2] = -dirR[2][0];

		matR[1][0] = dirR[0][1];
		matR[1][1] = -dirR[1][1];
		matR[1][2] = dirR[2][1];

		matR[2][0] = -dirR[0][2];
		matR[2][1] = dirR[1][2];
		matR[2][2] = -dirR[2][2];

		matR[4][0] = -controllerR[0][3];
		matR[4][1] = controllerR[1][3];
		matR[4][2] = -controllerR[2][3];

		matR = self->orientation * matR;

	}

	(*result)["4dvr_data"] = nlohmann::json
	{
		{
			"head",
			orientation.toJson()
		},
		{
			"handL",
			matL.toJson()
		},
		{
			"handR",
			matR.toJson()
		},
		{
			"heightRatio",
			heightRatio
		},
		{
			"headPos",
			m4::vec4ToJson(hmd4DPos)
		}
	};

	return *result;
}

bool localPlayerEvent(World* world, Player* player, Packet::ClientPacket eventType, int64_t eventValue, void* data)
{
	if (!afterPunch)
	{
		if (eventType == Packet::C_BLOCK_BREAK_FINISH)
		{
			static double lastHapticTime = -1;
			if (glfwGetTime() - lastHapticTime > 0.2)
			{
				VR::addHapticEvent(VR::CONTROLLER_RIGHT,
					new VR::HapticPulse(6, 4, 0.4f, 0.0f, 0.08f));
				lastHapticTime = glfwGetTime();
			}
		}
	}
	else
	{
		if (eventType == Packet::C_BLOCK_BREAK_CANCEL)
		{
			player->targetDamage = afterPunchTargetDamage;
			//return false;
		}
	}
	return true;
}

$hook(void, WorldSingleplayer, localPlayerEvent, Player* player, Packet::ClientPacket eventType, int64_t eventValue, void* data)
{
	if (localPlayerEvent(self, player, eventType, eventValue, data))
		return original(self, player, eventType, eventValue, data);
}
$hook(void, WorldClient, localPlayerEvent, Player* player, Packet::ClientPacket eventType, int64_t eventValue, void* data)
{
	if (localPlayerEvent(self, player, eventType, eventValue, data))
		return original(self, player, eventType, eventValue, data);
}

#else
void drawHands(StateGame* gameState)
{
	// nothing
}
#endif

struct EntityPlayerData
{
	Mat5 head;
	Mat5 handL;
	Mat5 handR;
	float heightRatio;
	glm::vec4 headPos;

	Mat5 lastHead;
	Mat5 lastHandL;
	Mat5 lastHandR;
	glm::vec4 lastHeadPos;

	double time=0,lastTime=0;
};
inline static std::unordered_map<EntityPlayer*, EntityPlayerData> entityPlayerData;

$hook(void, EntityPlayer, applyMovementUpdate, const nlohmann::json& j)
{
	if (!j.contains("4dvr_data"))
	{
		original(self, j);
		return;
	}

	const nlohmann::json& vrData = j["4dvr_data"];

	if (vrData.is_null())
	{
		original(self, j);
		return;
	}

	if (!entityPlayerData.contains(self))
		entityPlayerData.insert({ self, { Mat5{1}, Mat5{1}, Mat5{1}, 1.7f / Player::HEIGHT, {0,0,0,0} } });

	entityPlayerData[self].lastHead = entityPlayerData[self].head;
	entityPlayerData[self].lastHandL = entityPlayerData[self].handL;
	entityPlayerData[self].lastHandR = entityPlayerData[self].handR;
	entityPlayerData[self].lastHeadPos = entityPlayerData[self].headPos;

	entityPlayerData[self].head = Mat5(vrData["head"]);
	entityPlayerData[self].handL = Mat5(vrData["handL"]);
	entityPlayerData[self].handR = Mat5(vrData["handR"]);
	entityPlayerData[self].heightRatio = vrData["heightRatio"];
	entityPlayerData[self].headPos = m4::vec4FromJson<float>(vrData["headPos"]);

	entityPlayerData[self].lastTime = entityPlayerData[self].time;
	entityPlayerData[self].time = glfwGetTime();
	
	original(self, j);
}

inline static glm::vec4 matScale{ 1.f };
inline static glm::vec4 matTranslate{ 0.f };
inline static Mat5* lastEPMV;
inline static EntityPlayer* lastEP;
using ps = PlayerSkin;
$hook(void, EntityPlayer, render, const World* world, const m4::Mat5& MV, bool glasses)
{
	if (self->player != &StateGame::instanceObj.player && StateSettings::instanceObj.nametags)
	{
		static FontRenderer3D font3D{ ResourceManager::get("pixelfont.png"), ShaderManager::get("4dvr_textShader") };
		font3D.fontSize = 1;
		font3D.scale = 1.0f / 64.0f;

		glDisable(GL_DEPTH_TEST);
		font3D.setText(self->displayName);
		m4::Mat5 invOrientation = m4::Mat5::inverse(StateGame::instanceObj.player.orientation);
		use3DOrientation = true;
		m4::Mat5 MV3D = m4::createCamera({ 0,0,0,0 }, { 0,0,1,0 }, { 0,1,0,0 }, { 1,0,0,0 }, { 0,0,0,1 });
		use3DOrientation = false;
		glm::vec4 pos3D = invOrientation.multiply((self->getPos() + glm::vec4{0, 2.0f + 1.0f / 2.0f, 0, 0}) - StateGame::instanceObj.player.cameraPos + hmd4DPos, 1.0f);
		glm::vec3 pos3D3D{ pos3D.x, pos3D.y, pos3D.z };
		glm::vec3 head{ -hmdPose[0][3], hmdPose[1][3], -hmdPose[2][3] };
		glm::vec3 dir = glm::normalize(head - pos3D3D);
		glm::vec3 up = glm::normalize(glm::cross(glm::cross(dir, { 0,1,0 }), dir));
		glm::vec3 left = glm::normalize(glm::cross(up, dir));
		glm::mat4 dirMat{ 1 };
		dirMat[0] = glm::vec4{ left, 0.0f };
		dirMat[1] = glm::vec4{ up, 0.0f };
		dirMat[2] = glm::vec4{ dir, 0.0f };
		glm::mat4 model{ 1 };
		model = glm::translate(model, pos3D3D);
		model = model * dirMat;
		model = glm::translate(model, glm::vec3{ font3D.getSize() * -0.5f, 0.0f });
		font3D.setModel(model);
		font3D.render(ToMat4(MV3D));
		glEnable(GL_DEPTH_TEST);
	}

	if (!entityPlayerData.contains(self))
	{
		original(self, world, MV, glasses);
		return;
	}

	matScale = glm::vec4(entityPlayerData[self].heightRatio, entityPlayerData[self].heightRatio, entityPlayerData[self].heightRatio, 1.f);
	matTranslate = glm::vec4(0.f, -(1.f - matScale.y * (1.8f * 0.5f)), 0.f, 0.f);

	double time = glfwGetTime();
	static double time2 = (0.5f + 0.025f) * 0.5f;
	time2 = lerp(time2, entityPlayerData[self].time - entityPlayerData[self].lastTime, 0.5f);
	double diff = (time - entityPlayerData[self].time) / time2;
	double delta = 1.0 - diff;
	//double ratio = diff < 1.0 ? 1.0 - (delta * delta) : diff;
	double ratio = glm::clamp(diff, -1.0, 3.0);
	glm::vec4 headPos = lerp(
		entityPlayerData[self].lastHeadPos,
		entityPlayerData[self].headPos,
		ratio, false);
	Mat5 handR = entityPlayerData[self].handR;
	glm::vec4 handRPos = *(glm::vec4*)handR[4];
	glm::vec4 lastHandRPos = *(glm::vec4*)entityPlayerData[self].lastHandR[4];
	*(glm::vec4*)handR[4] = lerp(
		lastHandRPos,
		handRPos,
		ratio, false);
	Mat5 handL = entityPlayerData[self].handL;
	glm::vec4 handLPos = *(glm::vec4*)handL[4];
	glm::vec4 lastHandLPos = *(glm::vec4*)entityPlayerData[self].lastHandL[4];
	*(glm::vec4*)handL[4] = lerp(
		lastHandLPos,
		handLPos,
		ratio, false);

	// scale based on the height ratio (also in normalizeMat5)
	self->skinRenderer.orientationHead = entityPlayerData[self].head;
	self->skinRenderer.orientationHead.scale(matScale);

	// adjust the position because of the scaling
	Mat5 MV2 = MV;
	Mat5 MV3 = MV;
	MV2.translate(matTranslate);
	self->skinRenderer.orientationHead.translate(matTranslate);
	lastEPMV = &MV2;
	lastEP = self;

	// fuck the arms
	self->skinRenderer.transformations[ps::FOREARM_R] = { 0 };
	self->skinRenderer.transformations[ps::UPPER_ARM_R] = { 0 };
	self->skinRenderer.transformations[ps::FOREARM_L] = { 0 };
	self->skinRenderer.transformations[ps::UPPER_ARM_L] = { 0 };

	// render the skin
	original(self, world, MV2, glasses);
	
	// render the hands
	{
		MV3.translate(glm::vec4{ self->player->pos.x,self->player->pos.y,self->player->pos.z,self->player->pos.w } - headPos);

		const Shader* skinShader = ShaderManager::get("playerSkinShader");
		skinShader->use();
		glBindTexture(GL_TEXTURE_2D, self->skin.tex.ID);

		const float skinScale = Player::HEIGHT / (PlayerSkin::HEIGHT - 7);

		// right forearm/hand
		{
			Mat5 MVskin = MV3;
			MVskin = MVskin * handR;
			MVskin.scale(glm::vec4{ -skinScale,skinScale,skinScale,skinScale } * matScale);
			MVskin.translate({ -(float)ps::meshBounds[ps::FOREARM_R].origin.x, -(float)ps::meshBounds[ps::FOREARM_R].origin.y - 10, -(float)ps::meshBounds[ps::FOREARM_R].origin.z, 0 });

			glUniform1fv(glGetUniformLocation(skinShader->id(), "MV"), sizeof(Mat5) / sizeof(float), &MVskin[0][0]);
			self->skin.meshComps[ps::FOREARM_R].render();
		}
		// left forearm/hand
		{
			Mat5 MVskin2 = MV3;
			MVskin2 = MVskin2 * handL;
			MVskin2.scale(glm::vec4{ -skinScale,skinScale,skinScale,skinScale } * matScale);
			MVskin2.translate({ -(float)ps::meshBounds[ps::FOREARM_L].origin.x, -(float)ps::meshBounds[ps::FOREARM_L].origin.y - 10, -(float)ps::meshBounds[ps::FOREARM_L].origin.z, 0 });

			glUniform1fv(glGetUniformLocation(skinShader->id(), "MV"), sizeof(Mat5) / sizeof(float), &MVskin2[0][0]);
			self->skin.meshComps[ps::FOREARM_L].render();
		}
		/*
		// right upper arm
		{
			Mat5 MVskin = MV3;
			MVskin.scale(glm::vec4{ -skinScale,skinScale,skinScale,skinScale } * matScale);

			MVskin.translate(
				glm::vec4(
				(float)(-ps::meshBounds[ps::UPPER_ARM_R].origin.x),
				(float)(-ps::meshBounds[ps::UPPER_ARM_R].origin.y),
				(float)(-ps::meshBounds[ps::UPPER_ARM_R].origin.z + ps::meshBounds[ps::UPPER_ARM_R].zOffset), 0));

			glUniform1fv(glGetUniformLocation(skinShader->id(), "MV"), sizeof(Mat5) / sizeof(float), &MVskin[0][0]);
			glDisable(GL_DEPTH_TEST);
			self->skin.meshComps[ps::UPPER_ARM_R].render();
			glEnable(GL_DEPTH_TEST);
		}
		*/
		// right item
		{
			Item* itemR = self->player->hotbar.getSlot(self->player->hotbar.selectedIndex).get();

			if (itemR)
			{
				Mat5 MVItemR = MV3;
				MVItemR = MVItemR * handR;
				adjustItemMat(MVItemR, itemR, 1, true);

				itemR->renderEntity(MVItemR, itemR->getName() != "Compass" && itemR->getName() != "4D Glasses", { 0, 1, 0, 0 });
			}
		}
		// left item
		{
			Item* itemL = self->player->equipment.getSlot(0).get();

			if (itemL)
			{
				Mat5 MVItemL = MV3;
				MVItemL = MVItemL * handL;
				adjustItemMat(MVItemL, itemL, 0, true);

				itemL->renderEntity(MVItemL, itemL->getName() != "Compass" && itemL->getName() != "4D Glasses", {0, 1, 0, 0});
			}
		}
	}
}

$hook(void, Player, renderTargetBlock, const m4::Mat5& MV, bool renderBG)
{
	if (lastEPMV && lastEPMV == &MV)
	{
		Mat5 MV3 = MV;
		MV3.translate(-matTranslate);
		original(self, MV3, renderBG);
		lastEPMV = nullptr;
		return;
	}
	original(self, MV, renderBG);
}

$hookStatic(void, main_cpp, normalizeMat5, Mat5& mat)
{
	original(mat);
	if (lastEP)
	{
		mat.scale(matScale);
	}
}

$exec
{
	// remove nametag rendering

	uint8_t newBytes[0x89F - 0x50D];
	memset(newBytes, 0x90, sizeof(newBytes));
	patchMemory(getFuncAddr((int)Func::EntityPlayer::render) + 0x50D, newBytes, sizeof(newBytes));
}

extern "C" __declspec(dllexport) bool isEntityPlayerInVR(fdm::EntityPlayer* entity)
{
	if (!entity) return false;
	if (entityPlayerData.contains(entity)) return true;
	return false;
}
