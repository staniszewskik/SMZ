#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>

#include "constantsRender.h"
#include "constantsGameplay.h"
#include "utility.h"
#include "KeyMan.h"
#include "ResMan.h"
#include "ChunkMan.h"
#include "Player.h"
#include "Text.h"

bool firstMouse = true;
Player *playerRef;
float lastX = 0, lastY = 0;
float sensitivity = 0.001f;
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; // reversed since y-coordinates range from bottom to top
	lastX = (float)xpos;
	lastY = (float)ypos;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	playerRef->pyr.y -= xoffset;
	playerRef->pyr.x += yoffset;

	if (playerRef->pyr.x > glm::radians(90.0f))
		playerRef->pyr.x = glm::radians(90.0f);
	else if (playerRef->pyr.x < -glm::radians(90.0f))
		playerRef->pyr.x = -glm::radians(90.0f);
}

//bool throwHeld = false;
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		//if (action == GLFW_PRESS)
		//	throwHeld = true;
		//else
		//	throwHeld = false;
	}
}

int main(void)
{
	GLFWwindow *window = nullptr;
	unsigned int width, height;
	if (util::initWindow("SMZ", &window, &width, &height) == -1)
		return -1;

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSwapInterval(1); // vsync - spory input delay jesli limit fps (w constantsRender) nie jest ustawiony blisko odswiezania
	// (moze nadmiarowe klatki zapychaja bufor czy cos), ale ogolnie dziala ladnie

	unsigned int progPlatformShader = util::getShaderProgramFromName("platform");
	unsigned int progInterfaceShader = util::getShaderProgramFromName("interface");
	unsigned int progShadowShader = util::getShaderProgramFromName("shadow");
	unsigned int progCubeShader = util::getShaderProgramFromName("skybox");
	unsigned int progColShader = util::getShaderProgramFromName("col");

	ResMan resMan;
	Player player(progColShader);
	playerRef = &player;
	Text textDrawer(progInterfaceShader);

	unsigned int currentLevelIndex = 0;
	ChunkMan currentLevel(resMan.levelNames[0], progPlatformShader, progShadowShader, progCubeShader);
	glm::vec3 loadedSunDir = currentLevel.sun.dir;
	glm::vec2 sunRotationPitchYaw(0.0f, 0.0f);

	player.pyr.y = currentLevel.initialPlayerYaw;
	player.pos = currentLevel.initialPlayerPos;

	float fovY = 45.0f;
	bool wireframeDrawing = false;
	bool drawCollisions = false;
	bool tookScreenshot = false;
	bool pressedRestart = false;
	KeyMan keyMan(window,
		&player,
		&width,
		&height,
		&tookScreenshot,
		&fovY,
		&wireframeDrawing,
		&drawCollisions,
		&sensitivity,
		&sunRotationPitchYaw,
		&pressedRestart);
	keyMan.setInputMode(2); // 0 - rozgrywka, 1 - tylko spacja i esc, 2 - dev (testowe, tworzenie poziomow)

	bool forceShadowRedraw = false;
	Action *forceShadowRedrawAction = new Action(GLFW_KEY_F, -1);
	keyMan.actions.push_back(forceShadowRedrawAction);
	keyMan.mechanics.push_back(new ForceShadowRedraw(forceShadowRedrawAction, &forceShadowRedraw));

	Action *toggleVerticalSyncAction = new Action(GLFW_KEY_V, -1);
	keyMan.actions.push_back(toggleVerticalSyncAction);
	keyMan.mechanics.push_back(new ToggleVerticalSync(toggleVerticalSyncAction));

	char textFPS[] = "000FPS";

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearColor(cnst::clearColor.r, cnst::clearColor.g, cnst::clearColor.b, 0.0F);
	int frameCount = 0;
	float currentTime, updateDeltaTime, drawDeltaTime, drawCumulTime = 0.0f;
	float lastFrame, lastTick;
	lastFrame = (float)glfwGetTime();
	lastTick = lastFrame;
	bool dropFrame = false, levelCleared = false, depthMapHelpersValid = false;
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window) && !levelCleared)
	{
		if (drawCumulTime >= 1.0f)
		{
			std::cout << frameCount << std::endl;
			if (frameCount <= 999)
			{
				textFPS[2] = frameCount % 10 + '0';
				frameCount /= 10;
				textFPS[1] = frameCount % 10 + '0';
				frameCount /= 10;
				textFPS[0] = frameCount % 10 + '0';
				frameCount = 0;
			}
			else
			{
				textFPS[2] = '9';
				textFPS[1] = '9';
				textFPS[0] = '9';
				frameCount = 0;
			}
			drawCumulTime -= 1.0f;
		}

		// aktualizujemy dane
		currentTime = (float)glfwGetTime();
		updateDeltaTime = currentTime - lastTick;
		if (updateDeltaTime >= cnst::minUpdateTime)
		{
			if (updateDeltaTime > cnst::maxUpdateTime)
			{
				updateDeltaTime = cnst::maxUpdateTime;
				lastTick += cnst::maxUpdateTime;
				dropFrame = true;
			}
			else
			{
				lastTick = currentTime;
				dropFrame = false;
			}

			// klawiatura
			keyMan.processInput(updateDeltaTime);

			// chunki
			currentLevel.update(player.pos);

			/* Poll for and process events */
			glfwPollEvents();

			// fizyka
			currentLevel.collidePlayer(&player, updateDeltaTime);

			// czy gracz spadl z planszy
			if (player.pos.y <= cnst::barrierHeight || pressedRestart)
			{
				pressedRestart = false;

				player.pos = currentLevel.initialPlayerPos;
				player.pyr = glm::vec3(0.0f, currentLevel.initialPlayerYaw, 0.0f);
				player.vel = glm::vec3(0.0f);

				currentLevel.restartLevel();
			}

			if (forceShadowRedraw)
			{
				forceShadowRedraw = false;
				currentLevel.invalidateCascades();
			}
		}

		// rysujemy glowny widok
		currentTime = (float)glfwGetTime();
		drawDeltaTime = currentTime - lastFrame;
		if (!dropFrame && drawDeltaTime >= cnst::minDrawTime)
		{
			lastFrame = currentTime;
			frameCount++;
			drawCumulTime += drawDeltaTime;

			if (wireframeDrawing)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glm::vec3 camPos = glm::vec3(player.pos.x, player.pos.y + cnst::playerHeight - cnst::forehead, player.pos.z);

			glm::mat4 mulRPY(1.0f);
			mulRPY = glm::rotate(mulRPY, -(sunRotationPitchYaw.y + loadedSunDir.y), glm::vec3(0.0f, 1.0f, 0.0f));
			mulRPY = glm::rotate(mulRPY, -(sunRotationPitchYaw.x + loadedSunDir.x), glm::vec3(1.0f, 0.0f, 0.0f));
			currentLevel.sun.dir = mulRPY * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

			float fovRadianY = glm::radians(fovY);
			float aspectRatio = (float)width / height;
			if (!depthMapHelpersValid)
			{
				depthMapHelpersValid = true;
				currentLevel.calcDepthMapHelpers(fovRadianY, fovRadianY * aspectRatio);
			}
			currentLevel.drawDepthMap(camPos, player.pyr);

			glViewport(0, 0, width, height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glm::mat4 proj = glm::perspective(fovRadianY, aspectRatio, cnst::near, cnst::far);
			glm::mat4 view = util::lookAtPYR(camPos, player.pyr);

			if (drawCollisions)
			{
				if (!wireframeDrawing)
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				player.drawCol(view, proj);
				if (!wireframeDrawing)
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			currentLevel.draw(view, proj, camPos, player.pyr);

			textDrawer.drawText(textFPS, width - 16 * 6, height - 16, width, height, wireframeDrawing);

			/* Swap front and back buffers */
			glfwSwapBuffers(window);
		}
	}

	glfwTerminate();
	return 0;
}
