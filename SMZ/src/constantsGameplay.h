#pragma once

#include <glm/glm.hpp>

namespace cnst
{
	const float maxUpdateTime = 1.0f / 30.0f;
	const float minUpdateTime = 1.0f / 120.0f;

	const unsigned int seedLength = 32;

	const float gravity = 6.0f;
	const float wallDistance = 0.1f;
	const float jumpVel = 19.0f; // bylo 9.0f
	const float barrierHeight = -250.0f;

	const float baseMoveSpeed = 17.0f; // bylo 7.0f
	//const float maxMoveSpeed = 24.0f;
	//const float walljumpThreshold = 12.0f;
	//const float walljumpPenalty = 4.1f;
	//const float acceleration = 1.0f;
	const float airControl = 1.0f;
	const float playerHeight = 3.2f;
	const float forehead = 0.2f;

	const float baseTestSpeed1 = 20.0f;
	const float baseTestSpeed2 = 2.0f;
	const float sensitivitySpeed = 0.001f;
	const float sunRotationSpeed = 0.1f;
}
