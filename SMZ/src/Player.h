#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

class Player
{
public:
	Player(unsigned int progCol);
	~Player();
	void drawCol(glm::mat4 view, glm::mat4 proj);

	glm::vec3 pos = glm::vec3(0);
	glm::vec3 vel = glm::vec3(0);
	glm::vec3 pyr = glm::vec3(0);
	glm::vec3 dir = glm::vec3(0);

	bool onGround;
	bool closeToWall;
private:
	unsigned int progColShader;

	unsigned int colVAO, colVBO, colEBO;
	float *colVert;
	unsigned int *colIndc;
	unsigned int colVertSize, colIndcSize;
};
