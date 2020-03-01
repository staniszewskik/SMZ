#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <vector>

#include "constantsRender.h"
#include "helperStructs.h"
#include "Player.h"

enum class ActionState { Nothing, Pressed, Holding, Release }; // Pressed i Holding zeby wszystkie stany mialy tyle samo liter

class Action
{
public:
	Action(int primaryCode, int secondaryCode)
	{
		int codes[2] = { primaryCode, secondaryCode };

		for (int i = 0; i < 2; i++)
		{
			// narazie nie sprawdza czy kod myszkowy
			bindings[i].mouseBinding = false;

			bindings[i].lastDown = 0;

			if (codes[i] != -1)
			{
				bindings[i].keyCode = codes[i];
				bindings[i].usedBinding = true;
			}
			else
				bindings[i].usedBinding = false;
		}
	}

	void checkAction(GLFWwindow *window)
	{
		ActionState bindingStates[2];
		for (int i = 0; i < 2; i++)
		{
			if (!bindings[i].usedBinding)
				bindingStates[i] = ActionState::Nothing;
			else
			{
				int currentDown = glfwGetKey(window, bindings[i].keyCode) == GLFW_PRESS ? 2 : 0;

				switch (bindings[i].lastDown + currentDown)
				{
				case 0: bindingStates[i] = ActionState::Nothing; break; // 0 + 0
				case 1: bindingStates[i] = ActionState::Release; break; // 1 + 0
				case 2: bindingStates[i] = ActionState::Pressed; break; // 0 + 2
				case 3: bindingStates[i] = ActionState::Holding; break; // 1 + 2
				}

				bindings[i].lastDown = currentDown / 2;
			}
		}

		if (bindingStates[0] == ActionState::Holding || bindingStates[1] == ActionState::Holding)
			state = ActionState::Holding;
		else if (bindingStates[0] == ActionState::Pressed && bindingStates[1] == ActionState::Release ||
			bindingStates[0] == ActionState::Release && bindingStates[1] == ActionState::Pressed)
			state = ActionState::Holding;
		else if (bindingStates[0] == ActionState::Pressed || bindingStates[1] == ActionState::Pressed)
			state = ActionState::Pressed;
		else if (bindingStates[0] == ActionState::Release || bindingStates[1] == ActionState::Release)
			state = ActionState::Release;
		else
			state = ActionState::Nothing;
	}

	ActionState state;
private:
	keyBinding bindings[2];
};

class Mechanic
{
public:
	virtual void checkMechanic() = 0;
};

class KeyMan
{
public:
	KeyMan(GLFWwindow *window,
		Player *playerRef,
		unsigned int *width,
		unsigned int *height,
		bool *tookScreenshot,
		float *fovY,
		bool *wireframeDrawing,
		bool *drawCollisions,
		float *sensitivity,
		glm::vec2 *sunRotationPitchYaw,
		bool *pressedRestart)
	{
		this->window = window;
		this->playerRef = playerRef;
		this->width = width;
		this->height = height;
		this->tookScreenshot = tookScreenshot;

		this->fovY = fovY;
		this->wireframeDrawing = wireframeDrawing; wireframeDrawingLastState = false;
		this->drawCollisions = drawCollisions; drawCollisionsLastState = false;
		this->sensitivity = sensitivity;
		this->sunRotationPitchYaw = sunRotationPitchYaw;
		this->pressedRestart = pressedRestart; pressedRestartLastState = false;

		spaceLastState = false;
		printScreenLastState = false;
	}

	~KeyMan()
	{
		for (unsigned int i = 0; i < actions.size(); i++)
			delete actions[i];
		for (unsigned int i = 0; i < mechanics.size(); i++)
			delete mechanics[i];
	}

	void processInput(float deltaTime);
	void setInputMode(unsigned int mode);
	unsigned int getInputMode();

	std::vector<Action *> actions;
	std::vector<Mechanic *> mechanics;
private:
	GLFWwindow *window;
	unsigned int inputMode = 2;
	Player *playerRef;
	unsigned int *width;
	unsigned int *height;
	bool *tookScreenshot;

	float *fovY;
	bool *wireframeDrawing; bool wireframeDrawingLastState;
	bool *drawCollisions; bool drawCollisionsLastState;
	float *sensitivity;
	glm::vec2 *sunRotationPitchYaw;
	bool *pressedRestart; bool pressedRestartLastState;

	bool spaceLastState;
	bool printScreenLastState;

	void processSpaceEsc();
	void processWSAD(float deltaTime);
	void processGraphicToggles();
	void processTest(float deltaTime);
};

class ForceShadowRedraw : public Mechanic
{
public:
	ForceShadowRedraw(Action *action, bool *forceShadowRedraw)
	{
		this->state = &action->state;
		this->forceShadowRedraw = forceShadowRedraw;

		// chyba moznaby przekazywac tu wskaznik do wskaznika do aktualnego poziomu
		// zeby dzialalo jak bedzie zmiana poziomow, bez zadnego dodatkowego kodu w glownej petli,
		// w checkMechanic bylaby po prostu jedna dereferencja wskaznika i ->invalidateCascades();
		// a w glownym pliku zamiast obiektu ChunkMan mamy wskaznik do obiektu tworzonego new przy przelaczaniu poziomow
	}

	void checkMechanic() override
	{
		if (*state == ActionState::Pressed)
			*forceShadowRedraw = true;
	}
private:
	ActionState *state;
	bool *forceShadowRedraw;
};

class ToggleVerticalSync : public Mechanic
{
public:
	ToggleVerticalSync(Action *action)
	{
		this->state = &action->state;

		currentVerticalSyncState = true;
	}

	void checkMechanic() override
	{
		if (*state == ActionState::Pressed)
		{
			if (!currentVerticalSyncState)
			{
				glfwSwapInterval(1);
				cnst::minDrawTime = 1.0f / 60.0f;
				currentVerticalSyncState = true;
			}
			else
			{
				glfwSwapInterval(0);
				cnst::minDrawTime = 1.0f / 300.0f;
				currentVerticalSyncState = false;
			}
		}
	}
private:
	ActionState *state;
	bool currentVerticalSyncState;
};
