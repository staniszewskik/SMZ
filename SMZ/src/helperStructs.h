#pragma once

#include <glm/glm.hpp>

struct sunLight
{
	glm::vec3 dir;
	glm::vec3 color;
};

struct platform
{
	glm::vec3 pos;
	glm::vec3 dim;
};

struct intPlatform
{
	glm::ivec3 min;
	glm::ivec3 max;
};

struct manualAdd
{
	glm::ivec2 chunkPos;
	platform addition;
	bool type;
};

struct manualDel
{
	glm::ivec2 chunkPos;
	glm::vec3 deletion;
};

struct keyBinding
{
	int keyCode;
	bool usedBinding;
	bool mouseBinding;
	int lastDown;
};
