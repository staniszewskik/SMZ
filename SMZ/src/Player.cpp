#include "Player.h"

#include "utility.h"
#include "constantsRender.h"
#include "constantsGameplay.h"

Player::Player(unsigned int progCol)
{
	progColShader = progCol;

	onGround = false;
	closeToWall = false;

	util::readObj(cnst::cylinderPath, &colVert, &colVertSize, &colIndc, &colIndcSize);
	util::bindVAO_VN(&colVAO, &colVBO, &colEBO, colVert, colVertSize, colIndc, colIndcSize);
}

Player::~Player()
{
	delete[] colVert;
	delete[] colIndc;
	glDeleteVertexArrays(1, &colVAO);
	glDeleteBuffers(1, &colVBO);
	glDeleteBuffers(1, &colEBO);
}

void Player::drawCol(glm::mat4 view, glm::mat4 proj)
{
	glDisable(GL_CULL_FACE);

	glUseProgram(progColShader);

	glUniform3f(glGetUniformLocation(progColShader, "objectColor"), cnst::playerColor.r, cnst::playerColor.g, cnst::playerColor.b);

	glm::mat4 mdel(1.0f);
	mdel = glm::translate(mdel, pos);
	//mdel = glm::rotate(mdel, pyr.y + glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mdel = glm::scale(mdel, glm::vec3(1.0f, cnst::playerHeight / 2, 1.0f));
	mdel = glm::translate(mdel, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 MVP = proj * view * mdel;
	glUniformMatrix4fv(glGetUniformLocation(progColShader, "mvp"), 1, GL_FALSE, glm::value_ptr(MVP));

	glBindVertexArray(colVAO);
	glDrawElements(GL_TRIANGLES, colIndcSize / sizeof(unsigned int), GL_UNSIGNED_INT, 0);

	glEnable(GL_CULL_FACE);
}
