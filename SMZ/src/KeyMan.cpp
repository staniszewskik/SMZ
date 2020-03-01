#include "KeyMan.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>

#include "constantsGameplay.h"

void KeyMan::processInput(float deltaTime)
{
	for (unsigned int i = 0; i < actions.size(); i++)
		actions[i]->checkAction(window);
	for (unsigned int i = 0; i < mechanics.size(); i++)
		mechanics[i]->checkMechanic();

	if (inputMode == 1 || inputMode == 2)
	{
		processSpaceEsc();
	}

	if (inputMode == 2)
	{
		processTest(deltaTime);
		//processPlatformCreation
	}

	if (inputMode == 0 || inputMode == 2)
	{
		processWSAD(deltaTime);
		processGraphicToggles();

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			if (!spaceLastState && (playerRef->onGround || playerRef->closeToWall))
				playerRef->vel.y = cnst::jumpVel;
			spaceLastState = true;
		}
		else
			spaceLastState = false;

		if (glfwGetKey(window, GLFW_KEY_F12) == GLFW_PRESS)
		{
			if (!printScreenLastState)
			{
				*tookScreenshot = true;
				unsigned char *screenData = new unsigned char[4 * (*width) * (*height)];

				glReadPixels(0, 0, *width, *height, GL_RGBA, GL_UNSIGNED_BYTE, screenData);
				int res = stbi_write_bmp("resources/screenshot.bmp", *width, *height, 4, screenData);
				
				if (res == 0)
					std::cout << "Screenshot fail" << std::endl;
				else
					std::cout << "Screenshot success" << std::endl;

				delete[] screenData;
			}
			printScreenLastState = true;
		}
		else
			printScreenLastState = false;
	}
}

void KeyMan::setInputMode(unsigned int mode)
{
	inputMode = mode;
}

unsigned int KeyMan::getInputMode()
{
	return inputMode;
}

void KeyMan::processSpaceEsc()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void KeyMan::processWSAD(float deltaTime)
{
	// uwaga - zPos zmienia sie z cosinusem, bo yaw == 0 na osi +Z
	// a w dodatku - yaw juz jest w radianach (bo nie zamieniamy PYR na radiany w lookAtPYR)
	float moveSpeed = cnst::baseMoveSpeed * deltaTime;
	bool pressedWSAD = false;
	int pressZ = 0; // kierunek wybrany przez kombinacje WSAD,
	int pressX = 0; // zeby chodzenie na ukos nie bylo sqrt(2) * szybsze niz na wprost
	float pressLookup[3][3] = { {225.0f, 180.0f, 135.0f},
								{270.0f, 360.0f,  90.0f},
								{315.0f,   0.0f,  45.0f}, };

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // os Z rosnie do tylu
	{
		pressedWSAD = true;
		pressZ += 1;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		pressedWSAD = true;
		pressZ -= 1;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		pressedWSAD = true;
		pressX += 1;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		pressedWSAD = true;
		pressX -= 1;
	}

	if (pressedWSAD && (pressZ != 0 || pressX != 0))
	{
		float pressAngle = glm::radians(pressLookup[pressZ + 1][pressX + 1]);
		playerRef->pos.z += moveSpeed * cos(playerRef->pyr.y + pressAngle);
		playerRef->pos.x += moveSpeed * sin(playerRef->pyr.y + pressAngle);
	}
}

void KeyMan::processGraphicToggles()
{
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		if (!wireframeDrawingLastState)
			*wireframeDrawing = !(*wireframeDrawing);
		wireframeDrawingLastState = true;
	}
	else
		wireframeDrawingLastState = false;

	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
	{
		if (!drawCollisionsLastState)
			*drawCollisions = !(*drawCollisions);
		drawCollisionsLastState = true;
	}
	else
		drawCollisionsLastState = false;
}

void KeyMan::processTest(float deltaTime)
{
	float testSpeed1 = cnst::baseTestSpeed1 * deltaTime;
	float testSpeed2 = cnst::baseTestSpeed2 * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) // out - fovy zwieksza sie
		*fovY += testSpeed1;
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) // in - fovy zmniejsza sie
		*fovY -= testSpeed1;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		playerRef->pyr.z += testSpeed2;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		playerRef->pyr.z -= testSpeed2;

	if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
		*sensitivity -= cnst::sensitivitySpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
		*sensitivity += cnst::sensitivitySpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
		sunRotationPitchYaw->x += cnst::sunRotationSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
		sunRotationPitchYaw->x -= cnst::sunRotationSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
		sunRotationPitchYaw->y += cnst::sunRotationSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
		sunRotationPitchYaw->y -= cnst::sunRotationSpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		if (!pressedRestartLastState)
			*pressedRestart = true;
		pressedRestartLastState = true;
	}
	else
		pressedRestartLastState = false;
}
