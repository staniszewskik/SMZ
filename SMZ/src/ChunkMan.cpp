#include "ChunkMan.h"

#include <fstream>
#include <sstream>
#include <ctime>

#include "utility.h"

ChunkMan::ChunkMan(std::string name, unsigned int progPlatform, unsigned int progShadow, unsigned int progCube)
{
	for (int x = 0; x < 13; x++)
	{
		for (int y = 0; y < 13; y++)
		{
			chunkMatrix[x][y] = nullptr;
			chunkValid[x][y] = false;
		}
	}

	for (int i = 0; i < spiralCount; i++)
		chunkMatrix[spiralOrder[i].x + 6][spiralOrder[i].y + 6] = new Chunk();

	readLevelData(name);

	progPlatformShader = progPlatform;
	progShadowShader = progShadow;
	progCubeShader = progCube;

	cubeTextureID = util::createSkyboxGradient();
	sunLutID = util::createLutTexture(cnst::diffuseLutSampleCount, cnst::sunLutTexture);

	platformTextureID = util::createBasicTexture(("resources/textures/" + name + "_0.png").c_str());
	platformSpecularID = util::createBasicTexture(("resources/textures/" + name + "_s.png").c_str());
	platformNormalID = util::createBasicTexture(("resources/textures/" + name + "_n.png").c_str());
	diffuseLutID = util::createLutTexture(cnst::diffuseLutSampleCount, cnst::diffuseLutTexture);

	util::bindVAO_Cubemap(&cubeVAO, &cubeVBO);

	genDepthMaps();
	genJitterTable();
	initDraw();
}

ChunkMan::~ChunkMan()
{
	glDeleteTextures(1, &platformTextureID);
	glDeleteTextures(1, &platformSpecularID);
	glDeleteTextures(1, &platformNormalID);
	glDeleteTextures(1, &diffuseLutID);
	glDeleteTextures(1, &cubeTextureID);
	glDeleteTextures(1, &sunLutID);
	glDeleteTextures(cnst::cascadeCount, depthMapID);

	for (int i = 0; i < spiralCount; i++)
	{
		glm::ivec2 matrixCoords = spiralOrder[i] + glm::ivec2(6, 6);

		if (chunkMatrix[matrixCoords.x][matrixCoords.y] != nullptr)
			delete chunkMatrix[matrixCoords.x][matrixCoords.y];
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);

	glDeleteFramebuffers(cnst::cascadeCount, depthMapFBO);
}

void ChunkMan::update(glm::vec3 playerPos)
{
	if (playerPos.x < currentChunk.x * 100.0f - 50.0f - 20.0f)
	{
		currentChunk.x--;
		rightShift(1);
	}
	else if (playerPos.x > currentChunk.x * 100.0f + 50.0f + 20.0f)
	{
		currentChunk.x++;
		leftShift(1);
	}
	else if (playerPos.z < currentChunk.y * 100.0f - 50.0f - 20.0f)
	{
		currentChunk.y--;
		upShift(1);
	}
	else if (playerPos.z > currentChunk.y * 100.0f + 50.0f + 20.0f)
	{
		currentChunk.y++;
		downShift(1);
	}

	if (validCount == spiralCount)
		return;

	for (unsigned int i = 0; i < newChunkCoords.size(); i++)
	{
		glm::ivec2 newCoords = newChunkCoords[i] - currentChunk;

		if (newCoords.x < 0 || newCoords.x >= 13 || newCoords.y < 0 || newCoords.y >= 13 ||
			chunkMatrix[newCoords.x][newCoords.y] == nullptr)
		{
			if (i != newChunkCoords.size() - 1)
			{
				glm::ivec2 lastTemp = newChunkCoords.back();
				glm::ivec2 indexTemp = newChunkCoords[i];
				newChunkCoords.back() = indexTemp;
				newChunkCoords[i] = lastTemp;
			}
			newChunkCoords.pop_back();
			i--;
		}
	}

	for (int i = lastGenIndex; i < spiralCount; i++)
	{
		glm::ivec2 matrixCoords = spiralOrder[i] + glm::ivec2(6, 6);

		if (!chunkValid[matrixCoords.x][matrixCoords.y])
		{
			chunkValid[matrixCoords.x][matrixCoords.y] = true;
			validCount++;
			lastGenIndex = i;

			genPlatforms(spiralOrder[i]);
			applyManuals(spiralOrder[i]);
			chunkMatrix[matrixCoords.x][matrixCoords.y]->genChunkGeo();

			newChunkCoords.push_back(matrixCoords + currentChunk);

			return;
		}
	}

	lastGenIndex = 0;
}

void ChunkMan::draw(glm::mat4 view, glm::mat4 proj, glm::vec3 viewPos, glm::vec3 playerPyr)
{
	// platformy
	glUseProgram(progPlatformShader);

	glUniform3f(sunDirLoc, sun.dir.x, sun.dir.y, sun.dir.z);
	glUniform3f(viewPosLoc, viewPos.x, viewPos.y, viewPos.z);

	glm::mat4 playerViewProj = proj * view;
	glUniformMatrix4fv(VPLoc, 1, GL_FALSE, glm::value_ptr(playerViewProj));
	for (int i = 0; i < cnst::cascadeCount; i++)
		glUniformMatrix4fv(lightSpaceMatrixLoc + i, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix[i]));

	for (int i = 0; i < cnst::cascadeCount; i++)
		glUniform1f(cascadeEndClipSpaceLoc + i, (proj * glm::vec4(0.0f, 0.0f, -cascadeZBounds[i + 1], 1.0f)).z);

	for (int i = 0; i < spiralCount; i++)
	{
		if (!chunkValid[spiralOrder[i].x + 6][spiralOrder[i].y + 6])
			continue;

		Chunk *c = chunkMatrix[spiralOrder[i].x + 6][spiralOrder[i].y + 6];
		for (int p = 0; p < 2; p++)
		{
			if (c->platforms[p].size() == 0)
				continue;

			if(p == 0)
				glUniform3f(accentLoc, 1.0f, 1.0f, 1.0f);
			else
				glUniform3f(accentLoc, accent.r, accent.g, accent.b);

			glBindVertexArray(c->platformVAO[p]);
			glDrawArrays(GL_TRIANGLES, 0, c->platformVertSize[p] / (sizeof(float) * 12));
		}
	}

	// skybox
	glUseProgram(progCubeShader);

	glUniform3f(glGetUniformLocation(progCubeShader, "sun.dir"), sun.dir.x, sun.dir.y, sun.dir.z);

	glm::mat4 nonmovingVP = glm::mat4(glm::mat3(view));
	nonmovingVP = proj * nonmovingVP;
	glUniformMatrix4fv(glGetUniformLocation(progCubeShader, "VP"), 1, GL_FALSE, glm::value_ptr(nonmovingVP));

	glBindVertexArray(cubeVAO);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthFunc(GL_LESS);
}

void ChunkMan::drawDepthMap(glm::vec3 camPos, glm::vec3 pyr)
{
	glViewport(0, 0, cnst::defaultShadowWidth, cnst::defaultShadowHeight);

	glm::mat4 zeroLightView = glm::lookAt(glm::vec3(0.0f), sun.dir, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec4 zeroLightPos = zeroLightView * glm::vec4(camPos, 1.0f);
	glm::mat4 lightView = glm::lookAt(camPos, camPos + sun.dir, glm::vec3(0.0f, 1.0f, 0.0f));

	bool updateCascade[cnst::cascadeCount];
	platform candidateBoundingBoxes[cnst::cascadeCount];
	for (int i = cnst::cascadeCount - 2; i < cnst::cascadeCount; i++)
	{
		updateCascade[i] = false;
		candidateBoundingBoxes[i] = calcCascadeProjection(camPos, pyr, i, lightView);

		if (!(candidateBoundingBoxes[i].pos.x >= cascadeBoundingBox[i].pos.x &&
			candidateBoundingBoxes[i].dim.x <= cascadeBoundingBox[i].dim.x &&
			candidateBoundingBoxes[i].pos.y >= cascadeBoundingBox[i].pos.y &&
			candidateBoundingBoxes[i].dim.y <= cascadeBoundingBox[i].dim.y &&
			candidateBoundingBoxes[i].pos.z >= cascadeBoundingBox[i].pos.z &&
			candidateBoundingBoxes[i].dim.z <= cascadeBoundingBox[i].dim.z))
			updateCascade[i] = true;
	}

	for (int i = 0; i < cnst::cascadeCount; i++)
	{
		if (!cascadeValid[i] || (i >= cnst::cascadeCount - 2 && updateCascade[i]) ||
			glm::abs(zeroLightPos.x - cascadeCenter[i].x) >= 0.5f * cascadeRadius[i] ||
			glm::abs(zeroLightPos.y - cascadeCenter[i].y) >= 0.5f * cascadeRadius[i] ||
			glm::abs(zeroLightPos.z - cascadeCenter[i].z) >= 0.5f * cascadeRadius[i])
		{
			cascadeCenter[i] = zeroLightPos;
			cascadeValid[i] = true;
		}
		else if (newChunkCoords.size() > 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);

			glUseProgram(progShadowShader);
			glUniformMatrix4fv(glGetUniformLocation(progShadowShader, "lightSpaceMatrix"), 1, GL_FALSE,
				glm::value_ptr(lightSpaceMatrix[i]));

			for (unsigned int i = 0; i < newChunkCoords.size(); i++)
			{
				glm::ivec2 newCoords = newChunkCoords[i] - currentChunk;
				drawChunkPlatformsDepth(chunkMatrix[newCoords.x][newCoords.y]);
			}

			continue;
		}
		else
			continue;

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
		glClear(GL_DEPTH_BUFFER_BIT);

		glm::mat4 lightProjection;
		if (i >= cnst::cascadeCount - 2)
		{
			cascadeBoundingBox[i].pos.x = (candidateBoundingBoxes[i].pos.x + candidateBoundingBoxes[i].dim.x) / 2.0f;
			candidateBoundingBoxes[i].pos.x = (candidateBoundingBoxes[i].dim.x - candidateBoundingBoxes[i].pos.x) * 0.75f;
			cascadeBoundingBox[i].dim.x = cascadeBoundingBox[i].pos.x + candidateBoundingBoxes[i].pos.x;
			cascadeBoundingBox[i].pos.x -= candidateBoundingBoxes[i].pos.x;

			cascadeBoundingBox[i].pos.y = (candidateBoundingBoxes[i].pos.y + candidateBoundingBoxes[i].dim.y) / 2.0f;
			candidateBoundingBoxes[i].pos.y = (candidateBoundingBoxes[i].dim.y - candidateBoundingBoxes[i].pos.y) * 0.75f;
			cascadeBoundingBox[i].dim.y = cascadeBoundingBox[i].pos.y + candidateBoundingBoxes[i].pos.y;
			cascadeBoundingBox[i].pos.y -= candidateBoundingBoxes[i].pos.y;

			cascadeBoundingBox[i].pos.z = candidateBoundingBoxes[i].pos.z - 1.5f * cnst::far;
			cascadeBoundingBox[i].dim.z = candidateBoundingBoxes[i].dim.z + 1.5f * cnst::far;

			lightProjection = glm::ortho(cascadeBoundingBox[i].pos.x, cascadeBoundingBox[i].dim.x,
				cascadeBoundingBox[i].pos.y, cascadeBoundingBox[i].dim.y, 
				cascadeBoundingBox[i].pos.z, cascadeBoundingBox[i].dim.z);
		}
		else
			lightProjection = glm::ortho(-1.5f * cascadeRadius[i], 1.5f * cascadeRadius[i],
				-1.5f * cascadeRadius[i], 1.5f * cascadeRadius[i],
				-1.5f * cnst::far, 1.5f * cnst::far);

		lightSpaceMatrix[i] = lightProjection * lightView;

		glUseProgram(progShadowShader);
		glUniformMatrix4fv(glGetUniformLocation(progShadowShader, "lightSpaceMatrix"), 1, GL_FALSE,
			glm::value_ptr(lightSpaceMatrix[i]));

		for (int j = 0; j < spiralCount; j++)
		{
			if (!chunkValid[spiralOrder[j].x + 6][spiralOrder[j].y + 6])
				continue;

			drawChunkPlatformsDepth(chunkMatrix[spiralOrder[j].x + 6][spiralOrder[j].y + 6]);
		}
	}

	newChunkCoords.clear();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ChunkMan::drawChunkPlatformsDepth(Chunk *c)
{
	for (int p = 0; p < 2; p++)
	{
		if (c->platforms[p].size() == 0)
			continue;

		glBindVertexArray(c->platformVAO[p]);
		glDrawArrays(GL_TRIANGLES, 0, c->platformVertSize[p] / (sizeof(float) * 12));
	}
}

void ChunkMan::calcDepthMapHelpers(float fovY, float fovX)
{
	cascadeZBounds[0] = cnst::near;
	for (int i = 0; i < cnst::cascadeCount - 1; i++)
		cascadeZBounds[i + 1] = cnst::cascadePortions[i] * cnst::far;
	cascadeZBounds[cnst::cascadeCount] = cnst::far;

	ratioY = glm::tan(fovY / 2.0f);
	ratioX = glm::tan(fovX / 2.0f);

	for (int i = 0; i < cnst::cascadeCount; i++)
	{
		float cascadeFar = cascadeZBounds[i + 1];
		cascadeRadius[i] = glm::length(glm::vec3(cascadeFar * ratioX, cascadeFar * ratioY, cascadeFar));
	}
}

void ChunkMan::invalidateCascades()
{
	for (int i = 0; i < cnst::cascadeCount; i++)
		cascadeValid[i] = false;
}

//unsigned int Level::addPlatform(glm::vec3 pos, unsigned int type)
//{
	// dodaje do odpowiedniego wektora platform nowa platforme, o domyslnym rozmiarze i na podanej pozycji
	//platform toAdd;
	//toAdd.pos = pos;
	//toAdd.dim = glm::vec3(3.0f, 3.0f, 3.0f);
	//platforms[type].push_back(toAdd);

	// zwalnia VAO, VBO i takie tam i tworzy na nowo

	//return 0;
//}

//void Level::modifyPlatform(platform newVals, unsigned int type, unsigned int i)
//{
	// modyfikuje odpowiednia platforme w odpowiednim wektorze
	// zwalnia VAO, VBO i takie tam i tworzy na nowo
//}

//void Level::removePlatform(unsigned int type, unsigned int i)
//{
	// usuwa odpowiednia platforme w odpowiednim wektorze
	// zwalnia VAO, VBO i takie tam i tworzy na nowo
//}

void ChunkMan::restartLevel()
{
	glm::ivec2 chunkDiff = currentChunk - initialPlayerChunk;

	if (chunkDiff.x < 0)
		leftShift(-chunkDiff.x);
	else
		rightShift(chunkDiff.x);

	if (chunkDiff.y < 0)
		downShift(-chunkDiff.y);
	else
		upShift(chunkDiff.y);

	currentChunk = initialPlayerChunk;

	restartCount++;
}
void ChunkMan::collidePlayer(Player* p, float deltaTime)
{
	p->vel.y -= cnst::gravity * deltaTime;
	p->pos += p->vel * deltaTime;

	if (!p->onGround)
		p->pos += p->dir * cnst::airControl;

	p->onGround = false;
	p->closeToWall = false;

	glm::vec3 cor;
	for (int i = 0; i < 9; i++)
	{
		if (!chunkValid[spiralOrder[i].x + 6][spiralOrder[i].y + 6])
			continue;

		Chunk *c = chunkMatrix[spiralOrder[i].x + 6][spiralOrder[i].y + 6];
		for (int m = 0; m < 2; m++)
		{
			for (unsigned int n = 0; n < c->platforms[m].size(); n++)
			{
				bool closeTest = false;
				if (collideSingle(p->pos, 1.0f, cnst::playerHeight, c->platforms[m][n], &cor, &closeTest))
				{
					if (cor.y > 0.0f && p->vel.y < 0.0f || cor.y < 0.0f && p->vel.y > 0.0f)
						p->vel.y = 0.0f;

					if (cor.y > 0.0f)
						p->onGround = true;

					p->pos += cor;
				}
				if (closeTest && cor.y == 0.0f)
					p->closeToWall = true;
			}
		}
	}
}

bool ChunkMan::collideSingle(glm::vec3 pos, float r, float h, platform plt, glm::vec3 *cor, bool *closeToWall)
{
	*cor = glm::vec3(0.0f);
	bool foundCol = false;
	if (!(pos.y < plt.pos.y + plt.dim.y && pos.y + h > plt.pos.y))
		return foundCol;

	bool inXZBands = false;

	if (pos.x >= plt.pos.x && pos.x <= plt.pos.x + plt.dim.x)
	{
		inXZBands = true;

		// czy jestesmy blisko sciany?
		if (pos.z + r + cnst::wallDistance > plt.pos.z&& pos.z - r - cnst::wallDistance < plt.pos.z + plt.dim.z)
			*closeToWall = true;

		if (pos.z + r > plt.pos.z&& pos.z - r < plt.pos.z + plt.dim.z)
		{
			foundCol = true;
			cor->z = plt.pos.z - (pos.z + r);
			glm::vec3 other(0.0f, 0.0f, plt.pos.z + plt.dim.z - (pos.z - r));
			if (glm::length(other) < glm::length(*cor))
				*cor = other;
		}
	}

	if (pos.z >= plt.pos.z && pos.z <= plt.pos.z + plt.dim.z)
	{
		inXZBands = true;

		// czy jestesmy blisko sciany?
		if (pos.x + r + cnst::wallDistance > plt.pos.x&& pos.x - r - cnst::wallDistance < plt.pos.x + plt.dim.x)
			*closeToWall = true;

		if (pos.x + r > plt.pos.x&& pos.x - r < plt.pos.x + plt.dim.x)
		{
			glm::vec3 first(plt.pos.x - (pos.x + r), 0.0f, 0.0f);
			if (!foundCol || (foundCol && glm::length(first) < glm::length(*cor)))
			{
				foundCol = true;
				*cor = first;
			}
			glm::vec3 other(plt.pos.x + plt.dim.x - (pos.x - r), 0.0f, 0.0f);
			if (!foundCol || (foundCol && glm::length(other) < glm::length(*cor)))
			{
				foundCol = true;
				*cor = other;
			}
		}
	}

	if (!inXZBands &&
		pos.x + r >= plt.pos.x && pos.x - r <= plt.pos.x + plt.dim.x &&
		pos.z + r >= plt.pos.z && pos.z - r <= plt.pos.z + plt.dim.z)
	{
		glm::vec3 corners[4] = { glm::vec3(plt.pos.x, 0.0f, plt.pos.z),
								 glm::vec3(plt.pos.x + plt.dim.x, 0.0f, plt.pos.z),
								 glm::vec3(plt.pos.x, 0.0f, plt.pos.z + plt.dim.z),
								 glm::vec3(plt.pos.x + plt.dim.x, 0.0f, plt.pos.z + plt.dim.z), };

		// dla kazdego z 4 wierzcholkow
		for (int i = 0; i < 4; i++)
		{
			glm::vec3 diff(pos.x - corners[i].x, 0.0f, pos.z - corners[i].z);
			float diffLength = glm::length(diff);

			// czy jestesmy blisko sciany?
			if (diffLength < r + cnst::wallDistance)
				*closeToWall = true;

			if (diffLength < r)
			{
				glm::vec3 first = glm::normalize(diff) * (r - diffLength);
				if (!foundCol || (foundCol && glm::length(first) < glm::length(*cor)))
				{
					foundCol = true;
					*cor = first;
				}
			}
		}
	}

	// znalezc tez poprawke pionowa jesli jest kolizja
	if (foundCol)
	{
		glm::vec3 first(0.0f, plt.pos.y - (pos.y + h), 0.0f);
		if (foundCol && glm::length(first) < glm::length(*cor))
			*cor = first;
		glm::vec3 other(0.0f, plt.pos.y + plt.dim.y - pos.y, 0.0f);
		if (foundCol && glm::length(other) < glm::length(*cor))
			*cor = other;
	}

	return foundCol;
}

platform ChunkMan::calcCascadeProjection(glm::vec3 pos, glm::vec3 pyr, int cascadeIndex, glm::mat4 lightView)
{
	mulRPY = glm::mat4(1.0f);
	mulRPY = glm::rotate(mulRPY, pyr.y - glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mulRPY = glm::rotate(mulRPY, -pyr.x, glm::vec3(1.0f, 0.0f, 0.0f));
	mulRPY = glm::rotate(mulRPY, -pyr.z, glm::vec3(0.0f, 0.0f, 1.0f));

	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			for (int z = 0; z < 2; z++)
			{
				float dist = z == 0 ? cascadeZBounds[cascadeIndex] : cascadeZBounds[cascadeIndex + 1];
				float xDir = x == 0 ? -1.0f : 1.0f;
				float yDir = y == 0 ? -1.0f : 1.0f;
				subfrustum[x * 4 + y * 2 + z] = glm::vec4(xDir * dist * ratioX, yDir * dist * ratioY, dist, 0.0f);
			}
		}
	}

	mulRPY = glm::translate(mulRPY, pos);
	mulRPY = lightView * mulRPY;

	for (int i = 0; i < 8; i++)
		subfrustum[i] = mulRPY * subfrustum[i];

	glm::vec4 minBounds = subfrustum[0];
	glm::vec4 maxBounds = subfrustum[0];
	for (int i = 1; i < 8; i++)
	{
		if (subfrustum[i].x < minBounds.x)
			minBounds.x = subfrustum[i].x;
		if (subfrustum[i].y < minBounds.y)
			minBounds.y = subfrustum[i].y;
		if (subfrustum[i].z < minBounds.z)
			minBounds.z = subfrustum[i].z;

		if (subfrustum[i].x > maxBounds.x)
			maxBounds.x = subfrustum[i].x;
		if (subfrustum[i].y > maxBounds.y)
			maxBounds.y = subfrustum[i].y;
		if (subfrustum[i].z > maxBounds.z)
			maxBounds.z = subfrustum[i].z;
	}

	platform cascadeBoundingBox;
	cascadeBoundingBox.pos = minBounds;
	cascadeBoundingBox.dim = maxBounds;

	return cascadeBoundingBox;
}

void ChunkMan::initDraw()
{
	// platformy
	glUseProgram(progPlatformShader);

	sunDirLoc = glGetUniformLocation(progPlatformShader, "sun.dir");
	viewPosLoc = glGetUniformLocation(progPlatformShader, "viewPos");
	VPLoc = glGetUniformLocation(progPlatformShader, "VP");
	lightSpaceMatrixLoc = glGetUniformLocation(progPlatformShader, "lightSpaceMatrix[0]");
	cascadeEndClipSpaceLoc = glGetUniformLocation(progPlatformShader, "cascadeEndClipSpace[0]");
	accentLoc = glGetUniformLocation(progPlatformShader, "accent");

	glUniform1i(glGetUniformLocation(progPlatformShader, "ourTexture"), cnst::baseTextureSlot);
	glUniform1i(glGetUniformLocation(progPlatformShader, "specularMap"), cnst::specularMapSlot);
	glUniform1i(glGetUniformLocation(progPlatformShader, "normalMap"), cnst::normalMapSlot);
	glUniform1i(glGetUniformLocation(progPlatformShader, "diffuseLut"), cnst::diffuseLutSlot);

	glUniform3f(glGetUniformLocation(progPlatformShader, "sun.color"), sun.color.r, sun.color.g, sun.color.b);
	glUniform3f(glGetUniformLocation(progPlatformShader, "ambient"), ambient.r, ambient.g, ambient.b);

	for (int i = 0; i < cnst::cascadeCount; i++)
	{
		std::string depthMapName = "depthMap[" + std::to_string(i) + "]";
		glUniform1i(glGetUniformLocation(progPlatformShader, depthMapName.c_str()), cnst::depthMapSlotStart + i);
	}

	glm::mat4 unitMdel(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(progPlatformShader, "mdel"), 1, GL_FALSE, glm::value_ptr(unitMdel));
	glm::mat3 unitNorm(1.0f);
	glUniformMatrix3fv(glGetUniformLocation(progPlatformShader, "norm"), 1, GL_FALSE, glm::value_ptr(unitNorm));

	for (int i = 0; i < cnst::cascadeCount; i++)
	{
		glActiveTexture(GL_TEXTURE0 + cnst::depthMapSlotStart + i);
		glBindTexture(GL_TEXTURE_2D, depthMapID[i]);
	}

	for (int i = 0; i < cnst::jitterSampleCount; i++)
	{
		std::string jitterName = "jitterTable[" + std::to_string(i) + "]";
		glUniform2f(glGetUniformLocation(progPlatformShader, jitterName.c_str()), jitterTable[i].x, jitterTable[i].y);
	}

	glActiveTexture(GL_TEXTURE0 + cnst::baseTextureSlot);
	glBindTexture(GL_TEXTURE_2D, platformTextureID);
	glActiveTexture(GL_TEXTURE0 + cnst::specularMapSlot);
	glBindTexture(GL_TEXTURE_2D, platformSpecularID);
	glActiveTexture(GL_TEXTURE0 + cnst::normalMapSlot);
	glBindTexture(GL_TEXTURE_2D, platformNormalID);
	glActiveTexture(GL_TEXTURE0 + cnst::diffuseLutSlot);
	glBindTexture(GL_TEXTURE_1D, diffuseLutID);

	// skybox
	glActiveTexture(GL_TEXTURE0 + cnst::skyboxGradientSlot);
	glBindTexture(GL_TEXTURE_1D, cubeTextureID);

	glActiveTexture(GL_TEXTURE0 + cnst::sunLutSlot);
	glBindTexture(GL_TEXTURE_1D, sunLutID);

	glUseProgram(progCubeShader);
	glUniform1i(glGetUniformLocation(progCubeShader, "skyboxGradient"), cnst::skyboxGradientSlot);
	glUniform1i(glGetUniformLocation(progCubeShader, "sunLut"), cnst::sunLutSlot);

	glUniform3f(glGetUniformLocation(progCubeShader, "sun.color"), sun.color.r, sun.color.g, sun.color.b);
}

void ChunkMan::genDepthMaps()
{
	glGenFramebuffers(cnst::cascadeCount, depthMapFBO);
	glGenTextures(cnst::cascadeCount, depthMapID);
	for (int i = 0; i < cnst::cascadeCount; i++)
	{
		glBindTexture(GL_TEXTURE_2D, depthMapID[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			cnst::defaultShadowWidth, cnst::defaultShadowHeight,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapID[i], 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ChunkMan::genJitterTable()
{
	std::seed_seq jitterSeeds = { (unsigned int)trueRandom(), (unsigned int)time(NULL) };
	randomEngine.seed(jitterSeeds);

	std::uniform_real_distribution<float> randomDistribution;
	randomDistribution = std::uniform_real_distribution<float>(-0.5F, std::nextafter(0.5F, 1.0F));

	float jitterStart = -1.0f * cnst::jitterSampleDim / 2.0f + 0.5f;
	float jitterBound = -1.0f * jitterStart + 0.5f;
	for (int x = 0; x < cnst::jitterSampleDim; x++)
	{
		for (int y = 0; y < cnst::jitterSampleDim; y++)
			jitterTable[x * cnst::jitterSampleDim + y] = glm::vec2(jitterStart + 1.0f * x, jitterStart + 1.0f * y);
	}
	for (int i = 0; i < cnst::jitterSampleCount; i++)
	{
		jitterTable[i] += glm::vec2(randomDistribution(randomEngine), randomDistribution(randomEngine));
		jitterTable[i] += glm::vec2(jitterBound);
		jitterTable[i] /= 2.0f * jitterBound;
		float trigArg = 2.0f * cnst::pi * jitterTable[i].x;
		float areaScale = glm::sqrt(jitterTable[i].y);
		jitterTable[i] = glm::vec2(areaScale * glm::cos(trigArg), areaScale * glm::sin(trigArg));
		jitterTable[i] *= jitterBound;
	}
}

void ChunkMan::leftShift(int n)
{
	if (n == 0)
		return;

	if (n >= 13)
	{
		cleanChunks();
		return;
	}

	for (int x = -n; x < 13; x++)
	{
		for (int y = 0; y < 13; y++)
		{
			if (x < 0)
			{
				if (chunkMatrix[x + n][y] != nullptr)
				{
					if (chunkValid[x + n][y])
						validCount--;

					delete chunkMatrix[x + n][y];
				}
			}
			else if (chunkMatrix[x][y] == nullptr)
			{
				if (x + n < 13 && chunkMatrix[x + n][y] != nullptr)
				{
					if (chunkValid[x + n][y])
						validCount--;

					delete chunkMatrix[x + n][y];
				}
			}
			else
			{
				if (x + n < 13 && chunkMatrix[x + n][y] != nullptr)
				{
					chunkMatrix[x][y] = chunkMatrix[x + n][y];
					chunkValid[x][y] = chunkValid[x + n][y];
				}
				else
				{
					chunkMatrix[x][y] = new Chunk();
					chunkValid[x][y] = false;
				}
			}
		}
	}
}

void ChunkMan::rightShift(int n)
{
	if (n == 0)
		return;

	if (n >= 13)
	{
		cleanChunks();
		return;
	}

	for (int x = 12 + n; x >= 0; x--)
	{
		for (int y = 0; y < 13; y++)
		{
			if (x >= 13)
			{
				if (chunkMatrix[x - n][y] != nullptr)
				{
					if (chunkValid[x - n][y])
						validCount--;

					delete chunkMatrix[x - n][y];
				}
			}
			else if (chunkMatrix[x][y] == nullptr)
			{
				if (x - n >= 0 && chunkMatrix[x - n][y] != nullptr)
				{
					if (chunkValid[x - n][y])
						validCount--;

					delete chunkMatrix[x - n][y];
				}
			}
			else
			{
				if (x - n >= 0 && chunkMatrix[x - n][y] != nullptr)
				{
					chunkMatrix[x][y] = chunkMatrix[x - n][y];
					chunkValid[x][y] = chunkValid[x - n][y];
				}
				else
				{
					chunkMatrix[x][y] = new Chunk();
					chunkValid[x][y] = false;
				}
			}
		}
	}
}

void ChunkMan::downShift(int n)
{
	if (n == 0)
		return;

	if (n >= 13)
	{
		cleanChunks();
		return;
	}

	for (int y = -n; y < 13; y++)
	{
		for (int x = 0; x < 13; x++)
		{
			if (y < 0)
			{
				if (chunkMatrix[x][y + n] != nullptr)
				{
					if (chunkValid[x][y + n])
						validCount--;

					delete chunkMatrix[x][y + n];
				}
			}
			else if (chunkMatrix[x][y] == nullptr)
			{
				if (y + n < 13 && chunkMatrix[x][y + n] != nullptr)
				{
					if (chunkValid[x][y + n])
						validCount--;

					delete chunkMatrix[x][y + n];
				}
			}
			else
			{
				if (y + n < 13 && chunkMatrix[x][y + n] != nullptr)
				{
					chunkMatrix[x][y] = chunkMatrix[x][y + n];
					chunkValid[x][y] = chunkValid[x][y + n];
				}
				else
				{
					chunkMatrix[x][y] = new Chunk();
					chunkValid[x][y] = false;
				}
			}
		}
	}
}

void ChunkMan::upShift(int n)
{
	if (n == 0)
		return;

	if (n >= 13)
	{
		cleanChunks();
		return;
	}

	for (int y = 12 + n; y >= 0; y--)
	{
		for (int x = 0; x < 13; x++)
		{
			if (y >= 13)
			{
				if (chunkMatrix[x][y - n] != nullptr)
				{
					if (chunkValid[x][y - n])
						validCount--;

					delete chunkMatrix[x][y - n];
				}
			}
			else if (chunkMatrix[x][y] == nullptr)
			{
				if (y - n >= 0 && chunkMatrix[x][y - n] != nullptr)
				{
					if (chunkValid[x][y - n])
						validCount--;

					delete chunkMatrix[x][y - n];
				}
			}
			else
			{
				if (y - n >= 0 && chunkMatrix[x][y - n] != nullptr)
				{
					chunkMatrix[x][y] = chunkMatrix[x][y - n];
					chunkValid[x][y] = chunkValid[x][y - n];
				}
				else
				{
					chunkMatrix[x][y] = new Chunk();
					chunkValid[x][y] = false;
				}
			}
		}
	}
}

void ChunkMan::cleanChunks()
{
	for (int i = 0; i < spiralCount; i++)
	{
		glm::ivec2 matrixCoords = spiralOrder[i] + glm::ivec2(6, 6);

		if (chunkMatrix[matrixCoords.x][matrixCoords.y] != nullptr)
			delete chunkMatrix[matrixCoords.x][matrixCoords.y];
		chunkMatrix[matrixCoords.x][matrixCoords.y] = new Chunk();
	}

	for (int x = 0; x < 13; x++)
	{
		for (int y = 0; y < 13; y++)
			chunkValid[x][y] = false;
	}

	validCount = 0;
}

void ChunkMan::readLevelData(std::string name)
{
	std::ifstream in("resources/levels/" + name + ".levelgeo");
	std::string line = "";
	initSeed("");

	bool loadedPlayer = false;
	bool loadedAccent = false;
	bool loadedAmbient = false;
	bool loadedSun = false;

	std::getline(in, line);
	while (line != "")
	{
		std::istringstream splitter(line);
		std::string lineType;
		splitter >> lineType;

		if (lineType == "plr")
		{
			splitter >> initialPlayerChunk.x >> initialPlayerChunk.y
				>> initialPlayerPos.x >> initialPlayerPos.y >> initialPlayerPos.z
				>> initialPlayerYaw;

			currentChunk = initialPlayerChunk;
			initialPlayerYaw = glm::radians(initialPlayerYaw);
			loadedPlayer = true;
		}
		else if (lineType == "sed")
		{
			std::string seedString;

			splitter >> seedString;

			initSeed(seedString);
		}
		else if (lineType == "acc")
		{
			splitter >> accent.r >> accent.g >> accent.b;
			loadedAccent = true;
		}
		else if (lineType == "amb")
		{
			splitter >> ambient.r >> ambient.g >> ambient.b;
			loadedAmbient = true;
		}
		else if (lineType == "sun")
		{
			splitter >> sun.color.r >> sun.color.g >> sun.color.b >> sun.dir.x >> sun.dir.y;
			loadedSun = true;
		}
		else if (lineType == "add")
		{
			manualAdd readAdd;

			splitter >> readAdd.chunkPos.x >> readAdd.chunkPos.y
				>> readAdd.addition.pos.x >> readAdd.addition.pos.y >> readAdd.addition.pos.z
				>> readAdd.addition.dim.x >> readAdd.addition.dim.y >> readAdd.addition.dim.z
				>> readAdd.type;

			manualAdditions.push_back(readAdd);
		}
		else if (lineType == "del")
		{
			manualDel readDel;

			splitter >> readDel.chunkPos.x >> readDel.chunkPos.y
				>> readDel.deletion.x >> readDel.deletion.y >> readDel.deletion.z;

			manualDeletions.push_back(readDel);
		}

		std::getline(in, line);
	}

	generateLevelParameters(loadedPlayer, loadedAccent, loadedAmbient, loadedSun);
}

void ChunkMan::generateLevelParameters(bool loadedPlayer, bool loadedAccent, bool loadedAmbient, bool loadedSun)
{
	glm::ivec2 tempCurrentChunk = currentChunk;
	currentChunk = glm::ivec2(0, 0);
	genPlatforms(glm::ivec2(0, 0));
	currentChunk = tempCurrentChunk;

	// te losowania nie moga byc wewnatrz if-ow, bo wtedy z tego samego seeda mozna dostac rozne poziomy
	std::uniform_int_distribution<int> rotationDistrib(0, 3);
	int rotationChoice = rotationDistrib(levelSeedEngine);
	std::uniform_int_distribution<int> accentDistrib(0, 5);
	int accentChoice = accentDistrib(levelSeedEngine);

	if (!loadedPlayer)
	{
		initialPlayerChunk = glm::ivec2(0, 0);
		initialPlayerYaw = glm::radians(rotationChoice * 90.0f);

		bool foundSpawn = false;
		platform chunkPlatform;
		for (int p = 0; p < 2 && !foundSpawn; p++)
		{
			for (unsigned int i = 0; i < chunkMatrix[6][6]->platforms[p].size() && !foundSpawn; i++)
			{
				chunkPlatform = chunkMatrix[6][6]->platforms[p][i];
				initialPlayerPos = chunkPlatform.pos + glm::vec3(0.5f, 1.0f, 0.5f) * chunkPlatform.dim;

				platform initialPlayerBox;
				initialPlayerBox.pos = initialPlayerPos - glm::vec3(1.0f, 0.0f, 1.0f);
				initialPlayerBox.dim = glm::vec3(2.0f, cnst::playerHeight, 2.0f);
				
				bool foundCol = false;
				if (initialPlayerBox.pos.x < -30.0f || initialPlayerBox.pos.x + initialPlayerBox.dim.x > 30.0f ||
					initialPlayerBox.pos.z < -30.0f || initialPlayerBox.pos.z + initialPlayerBox.dim.z > 30.0f)
					foundCol = true;

				for (int pCol = 0; pCol < 2 && !foundCol; pCol++)
				{
					for (unsigned int iCol = 0; iCol < chunkMatrix[6][6]->platforms[pCol].size() && !foundCol; iCol++)
					{
						chunkPlatform = chunkMatrix[6][6]->platforms[pCol][iCol];

						if (initialPlayerBox.pos.x < chunkPlatform.pos.x + chunkPlatform.dim.x &&
							initialPlayerBox.pos.x + initialPlayerBox.dim.x > chunkPlatform.pos.x &&
							initialPlayerBox.pos.y < chunkPlatform.pos.y + chunkPlatform.dim.y &&
							initialPlayerBox.pos.y + initialPlayerBox.dim.y > chunkPlatform.pos.y &&
							initialPlayerBox.pos.z < chunkPlatform.pos.z + chunkPlatform.dim.z &&
							initialPlayerBox.pos.z + initialPlayerBox.dim.z > chunkPlatform.pos.z)
							foundCol = true;
					}
				}

				if (!foundCol)
					foundSpawn = true;
			}
		}
	}

	if (!loadedAccent)
	{
		switch (accentChoice)
		{
		case 0: accent = glm::vec3(0.148f, 0.030f, 0.741f); break; // blue
		case 1: accent = glm::vec3(0.948f, 0.038f, 0.038f); break; // red
		case 2: accent = glm::vec3(1.000f, 0.466f, 0.000f); break; // orange
		case 3: accent = glm::vec3(0.204f, 0.788f, 0.032f); break; // green
		case 4: accent = glm::vec3(0.906f, 0.776f, 0.067f); break; // yellow
		case 5: accent = glm::vec3(0.077f, 0.767f, 0.499f); break; // turquoise
		}
	}

	if (!loadedAmbient)
		ambient = glm::vec3(0.3f, 0.3f, 0.3f);

	if (!loadedSun)
	{
		sun.color = glm::vec3(0.9f, 0.8f, 0.8f);
		sun.dir = glm::vec3(glm::radians(30.0f), glm::radians(30.0f), 0.0f);
	}

	cleanChunks();
}

void ChunkMan::initSeed(std::string seedString)
{
	for (unsigned int i = 0; i < seedString.length(); i++)
		seed[i % cnst::seedLength] += seedString[i];

	for (int i = seedString.length(); i < cnst::seedLength; i++)
		seed[i] = '0';

	for (int i = 0; i < cnst::seedLength; i++)
		seed[i] = (9 + seed[i] % 9 - '1' % 9) % 9 + 1;
}

unsigned int ChunkMan::getSeedPortion(unsigned int pos, unsigned int len)
{
	unsigned int portionValue = 0;

	for (unsigned int i = pos; i < pos + len && i < cnst::seedLength; i++)
	{
		portionValue *= 10;
		portionValue += seed[i];
	}

	return portionValue;
}

void ChunkMan::seedLevelEngine(glm::ivec2 chunkCoords)
{
	std::seed_seq levelEngineSeeds = { (int)getSeedPortion(0, 8),
		(int)getSeedPortion(8 + 8 * (chunkCoords.x % 2 == 0 ? 0 : 1), 8),
		(int)getSeedPortion(24 + 4 * (chunkCoords.y % 2 == 0 ? 0 : 1), 4),
		chunkCoords.x, chunkCoords.y };
	levelSeedEngine.seed(levelEngineSeeds);
}

void ChunkMan::genChunkValues(int chunkArray[20])
{
	std::uniform_int_distribution<int> widthDistrib(-60, 60);
	std::uniform_int_distribution<int> heightDistrib(-400, 400);

	for (int i = 0; i < 4; i++)
	{
		chunkArray[i * 4 + 0] = widthDistrib(levelSeedEngine);
		chunkArray[i * 4 + 1] = heightDistrib(levelSeedEngine);
		chunkArray[i * 4 + 2] = widthDistrib(levelSeedEngine);
		chunkArray[i * 4 + 3] = heightDistrib(levelSeedEngine);
	}

	for (int i = 0; i < 4; i++)
		chunkArray[16 + i] = heightDistrib(levelSeedEngine);
}

void ChunkMan::genPlatforms(glm::ivec2 spiralCoords)
{
	glm::ivec2 chunkCoords = spiralCoords + currentChunk;

	int chunkGenValues[3][3][20];
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			int xMatrix = x + spiralCoords.x + 6;
			int yMatrix = y + spiralCoords.y + 6;
			if (xMatrix >= 0 && xMatrix < 13 && yMatrix >= 0 && yMatrix < 13 && chunkMatrix[xMatrix][yMatrix] != nullptr)
			{
				if (chunkMatrix[xMatrix][yMatrix]->valuesGenerated)
				{
					for (int v = 0; v < 20; v++)
						chunkGenValues[x + 1][y + 1][v] = chunkMatrix[xMatrix][yMatrix]->genValues[v];
				}
				else
				{
					// generujemy 20 wartosci
					seedLevelEngine(chunkCoords + glm::ivec2(x, y));
					genChunkValues(chunkGenValues[x + 1][y + 1]);
					// przepisujemy je do tablicy w chunku na przyszlosc
					for (int v = 0; v < 20; v++)
						chunkMatrix[xMatrix][yMatrix]->genValues[v] = chunkGenValues[x + 1][y + 1][v];
					// zaznaczamy valuesGenerated
					chunkMatrix[xMatrix][yMatrix]->valuesGenerated = true;
				}
			}
			else
			{
				// generujemy 20 wartosci
				seedLevelEngine(chunkCoords + glm::ivec2(x, y));
				genChunkValues(chunkGenValues[x + 1][y + 1]);
			}
		}
	}

	// przygotowuje levelSeedEngine do losowania platform
	seedLevelEngine(chunkCoords);
	genChunkValues(chunkGenValues[1][1]);

	std::vector<intPlatform> fakePlatforms;
	for (int i = 0; i < 8; i++)
	{
		glm::ivec2 hor(-140, 140);
		if (chunkGenValues[1][1][i * 2 + 0] < 0)
			hor.y = chunkGenValues[1][1][i * 2 + 0] + 60;
		else if (chunkGenValues[1][1][i * 2 + 0] > 0)
			hor.x = chunkGenValues[1][1][i * 2 + 0] - 60;

		glm::ivec2 ver;
		ver.x = chunkGenValues[1][1][i * 2 + 1] - 100;
		ver.x = ver.x < -400 ? -400 : ver.x;
		ver.y = chunkGenValues[1][1][i * 2 + 1] + 100;
		ver.y = ver.y > 400 ? 400 : ver.y;

		intPlatform edgeForbiddenZone;
		if (i / 2 == 0) // gora, +z
		{
			edgeForbiddenZone.min.z = 60;
			edgeForbiddenZone.max.z = 140;
			edgeForbiddenZone.min.x = hor.x;
			edgeForbiddenZone.max.x = hor.y;
			edgeForbiddenZone.min.y = ver.x;
			edgeForbiddenZone.max.y = ver.y;
		}
		else if (i / 2 == 1) // prawo, +x
		{
			edgeForbiddenZone.min.x = 60;
			edgeForbiddenZone.max.x = 140;
			edgeForbiddenZone.min.z = hor.x;
			edgeForbiddenZone.max.z = hor.y;
			edgeForbiddenZone.min.y = ver.x;
			edgeForbiddenZone.max.y = ver.y;
		}
		else if (i / 2 == 2) // dol, -z
		{
			edgeForbiddenZone.min.z = -140;
			edgeForbiddenZone.max.z = -60;
			edgeForbiddenZone.min.x = hor.x;
			edgeForbiddenZone.max.x = hor.y;
			edgeForbiddenZone.min.y = ver.x;
			edgeForbiddenZone.max.y = ver.y;
		}
		else // lewo, -x
		{
			edgeForbiddenZone.min.x = -140;
			edgeForbiddenZone.max.x = -60;
			edgeForbiddenZone.min.z = hor.x;
			edgeForbiddenZone.max.z = hor.y;
			edgeForbiddenZone.min.y = ver.x;
			edgeForbiddenZone.max.y = ver.y;
		}
		fakePlatforms.push_back(edgeForbiddenZone);
	}

	glm::ivec2 cornerOffsets[4] = { glm::ivec2(1, 1), glm::ivec2(1, 0), glm::ivec2(0, 0), glm::ivec2(0, 1) };
	for (int i = 0; i < 4; i++)
	{
		std::pair<int, int> cornerValues[4];
		cornerValues[0].first = chunkGenValues[cornerOffsets[i].x + 1][cornerOffsets[i].y + 1][16 + 2];
		cornerValues[1].first = chunkGenValues[cornerOffsets[i].x + 1][cornerOffsets[i].y + 0][16 + 3];
		cornerValues[2].first = chunkGenValues[cornerOffsets[i].x + 0][cornerOffsets[i].y + 0][16 + 0];
		cornerValues[3].first = chunkGenValues[cornerOffsets[i].x + 0][cornerOffsets[i].y + 1][16 + 1];
		cornerValues[0].second = 0;
		cornerValues[1].second = 1;
		cornerValues[2].second = 2;
		cornerValues[3].second = 3;

		int ourIndex = (i + 2) % 4;
		std::sort(cornerValues, cornerValues + 4);

		int ourSortedIndex = 0;
		while (cornerValues[ourSortedIndex].second != ourIndex)
			ourSortedIndex++;

		int lowerBound = ourSortedIndex;
		while (lowerBound > 0 && cornerValues[lowerBound - 1].first == cornerValues[ourSortedIndex].first)
			lowerBound--;
		int upperBound = ourSortedIndex;
		while (upperBound < 3 && cornerValues[upperBound + 1].first == cornerValues[ourSortedIndex].first)
			upperBound++;

		int cornerRangeMin;
		if (lowerBound == 0)
			cornerRangeMin = -400;
		else
			cornerRangeMin = (cornerValues[lowerBound - 1].first + cornerValues[lowerBound].first) / 2;
		int cornerRangeMax;
		if (upperBound == 3)
			cornerRangeMax = 400;
		else
			cornerRangeMax = (cornerValues[upperBound].first + cornerValues[upperBound + 1].first) / 2;

		if (upperBound - lowerBound + 1 > 1)
		{
			int subLength = (cornerRangeMax - cornerRangeMin) / (upperBound - lowerBound + 1);
			cornerRangeMin += subLength * (ourSortedIndex - lowerBound);
			cornerRangeMax -= subLength * (upperBound - ourSortedIndex);
			int remainder = (cornerRangeMax - cornerRangeMin) - subLength * (upperBound - lowerBound + 1);
			cornerRangeMax -= upperBound != ourSortedIndex ? remainder : 0;
		}

		glm::ivec2 cornerXZPos;
		cornerXZPos.x = 200 * cornerOffsets[i].x - 100;
		cornerXZPos.y = 200 * cornerOffsets[i].y - 100;

		if (cornerRangeMin == cornerRangeMax) // pojedyncza kolumna od -400 do 400
		{
			intPlatform cornerForbiddenZone;
			cornerForbiddenZone.min.x = cornerXZPos.x - 40;
			cornerForbiddenZone.max.x = cornerXZPos.x + 40;
			cornerForbiddenZone.min.y = -400;
			cornerForbiddenZone.max.y = 400;
			cornerForbiddenZone.min.z = cornerXZPos.y - 40;
			cornerForbiddenZone.max.z = cornerXZPos.y + 40;

			fakePlatforms.push_back(cornerForbiddenZone);
		}
		else
		{
			if (cornerRangeMin != -400) // od -400 do min
			{
				intPlatform bottomCornerForbiddenZone;
				bottomCornerForbiddenZone.min.x = cornerXZPos.x - 40;
				bottomCornerForbiddenZone.max.x = cornerXZPos.x + 40;
				bottomCornerForbiddenZone.min.y = -400;
				bottomCornerForbiddenZone.max.y = cornerRangeMin;
				bottomCornerForbiddenZone.min.z = cornerXZPos.y - 40;
				bottomCornerForbiddenZone.max.z = cornerXZPos.y + 40;

				fakePlatforms.push_back(bottomCornerForbiddenZone);
			}

			if (cornerRangeMax != 400) // od max do 400
			{
				intPlatform topCornerForbiddenZone;
				topCornerForbiddenZone.min.x = cornerXZPos.x - 40;
				topCornerForbiddenZone.max.x = cornerXZPos.x + 40;
				topCornerForbiddenZone.min.y = cornerRangeMax;
				topCornerForbiddenZone.max.y = 400;
				topCornerForbiddenZone.min.z = cornerXZPos.y - 40;
				topCornerForbiddenZone.max.z = cornerXZPos.y + 40;

				fakePlatforms.push_back(topCornerForbiddenZone);
			}
		}
	}

	glm::ivec2 edgeOffsets[4] = { glm::ivec2(1, 2), glm::ivec2(2, 1), glm::ivec2(1, 0), glm::ivec2(0, 1) };
	std::vector<intPlatform> edgeSafeZones;
	for (int i = 0; i < 8; i++)
	{
		int zoneIndex = ((i + 4) % 8) * 2;
		int xCoord = edgeOffsets[i / 2].x;
		int yCoord = edgeOffsets[i / 2].y;

		// gora, prawo, dol, lewo
		glm::ivec2 hor(-140, 140);
		if (chunkGenValues[xCoord][yCoord][zoneIndex + 0] < 0)
			hor.y = chunkGenValues[xCoord][yCoord][zoneIndex + 0] + 60;
		else if (chunkGenValues[xCoord][yCoord][zoneIndex + 0] > 0)
			hor.x = chunkGenValues[xCoord][yCoord][zoneIndex + 0] - 60;

		glm::ivec2 ver;
		ver.x = chunkGenValues[xCoord][yCoord][zoneIndex + 1] - 100;
		ver.x = ver.x < -400 ? -400 : ver.x;
		ver.y = chunkGenValues[xCoord][yCoord][zoneIndex + 1] + 100;
		ver.y = ver.y > 400 ? 400 : ver.y;

		intPlatform edgeSafeZone;
		if (i / 2 == 0) // gora, +z
		{
			edgeSafeZone.min.z = 60;
			edgeSafeZone.max.z = 140;
			edgeSafeZone.min.x = hor.x;
			edgeSafeZone.max.x = hor.y;
			edgeSafeZone.min.y = ver.x;
			edgeSafeZone.max.y = ver.y;
		}
		else if (i / 2 == 1) // prawo, +x
		{
			edgeSafeZone.min.x = 60;
			edgeSafeZone.max.x = 140;
			edgeSafeZone.min.z = hor.x;
			edgeSafeZone.max.z = hor.y;
			edgeSafeZone.min.y = ver.x;
			edgeSafeZone.max.y = ver.y;
		}
		else if (i / 2 == 2) // dol, -z
		{
			edgeSafeZone.min.z = -140;
			edgeSafeZone.max.z = -60;
			edgeSafeZone.min.x = hor.x;
			edgeSafeZone.max.x = hor.y;
			edgeSafeZone.min.y = ver.x;
			edgeSafeZone.max.y = ver.y;
		}
		else // lewo, -x
		{
			edgeSafeZone.min.x = -140;
			edgeSafeZone.max.x = -60;
			edgeSafeZone.min.z = hor.x;
			edgeSafeZone.max.z = hor.y;
			edgeSafeZone.min.y = ver.x;
			edgeSafeZone.max.y = ver.y;
		}
		edgeSafeZones.push_back(edgeSafeZone);
	}

	int attemptCount = 0;
	bool startNewPlatform = true;
	glm::ivec2 maxDimensions(200, 600);
	glm::ivec2 minDimensions(40, 20);
	intPlatform randomPlatform;
	std::vector<intPlatform> randomPlatforms;
	std::uniform_int_distribution<int> overcorrectDistrib(1, 4);
	std::uniform_int_distribution<int> optionDistrib(0, 11);
	while (randomPlatforms.size() < 10 && attemptCount < 50)
	{
		if (startNewPlatform)
		{
			std::uniform_int_distribution<int> widthDistrib(minDimensions.x / 2, maxDimensions.x);
			std::uniform_int_distribution<int> heightDistrib(minDimensions.y / 2, maxDimensions.y);

			randomPlatform.max.x = widthDistrib(levelSeedEngine);
			std::uniform_int_distribution<int> xDistrib(-140, 140 - randomPlatform.max.x);
			randomPlatform.max.y = heightDistrib(levelSeedEngine);
			std::uniform_int_distribution<int> yDistrib(-400, 400 - randomPlatform.max.y);
			randomPlatform.max.z = widthDistrib(levelSeedEngine);
			std::uniform_int_distribution<int> zDistrib(-140, 140 - randomPlatform.max.z);

			randomPlatform.min.x = xDistrib(levelSeedEngine);
			randomPlatform.max.x += randomPlatform.min.x;
			randomPlatform.min.y = yDistrib(levelSeedEngine);
			randomPlatform.max.y += randomPlatform.min.y;
			randomPlatform.min.z = zDistrib(levelSeedEngine);
			randomPlatform.max.z += randomPlatform.min.z;

			startNewPlatform = false;
			maxDimensions.x = (int)(maxDimensions.x * 0.8f);
			maxDimensions.x = maxDimensions.x < minDimensions.x ? minDimensions.x : maxDimensions.x;
			maxDimensions.y = (int)(maxDimensions.y * 0.8f);
			maxDimensions.y = maxDimensions.y < minDimensions.y ? minDimensions.y : maxDimensions.y;
		}

		bool noCollision = true;

		// opcje / 2 to kierunek (jeden z szesciu)
		// opcje % 2 to uciecie platformy jesli == 0 (modyfikujemy tylko jedna strone platformy)
		// przesuniecie platformy jesli == 1 (modyfikujemy obie strony)

		// poprawki ze wzgledu na fakePlatforms
		// anuluje poprawke jezeli calosc platformy znajdzie sie w jej wyniku poza chunkiem
		// (duze szanse na to, bo wszystkie fakePlatforms sa przy krawedziach chunka)
		for (unsigned int i = 0; i < fakePlatforms.size(); i++)
		{
			if (randomPlatform.min.x <= fakePlatforms[i].max.x && randomPlatform.max.x >= fakePlatforms[i].min.x &&
				randomPlatform.min.y <= fakePlatforms[i].max.y && randomPlatform.max.y >= fakePlatforms[i].min.y &&
				randomPlatform.min.z <= fakePlatforms[i].max.z && randomPlatform.max.z >= fakePlatforms[i].min.z)
			{
				noCollision = false;
				attemptCount++;

				intPlatform backup = randomPlatform;
				int option = optionDistrib(levelSeedEngine);
				if (option / 2 == 0) // +x
				{
					int correction = fakePlatforms[i].max.x - randomPlatform.min.x + overcorrectDistrib(levelSeedEngine);

					randomPlatform.min.x += correction;
					if (option % 2 == 1) // wypycha
						randomPlatform.max.x += correction;
				}
				else if (option / 2 == 1) // -x
				{
					int correction = fakePlatforms[i].min.x - randomPlatform.max.x - overcorrectDistrib(levelSeedEngine);

					randomPlatform.max.x += correction;
					if (option % 2 == 1) // wypycha
						randomPlatform.min.x += correction;
				}
				else if (option / 2 == 2) // +y
				{
					int correction = fakePlatforms[i].max.y - randomPlatform.min.y + overcorrectDistrib(levelSeedEngine);

					randomPlatform.min.y += correction;
					if (option % 2 == 1) // wypycha
						randomPlatform.max.y += correction;
				}
				else if (option / 2 == 3) // -y
				{
					int correction = fakePlatforms[i].min.y - randomPlatform.max.y - overcorrectDistrib(levelSeedEngine);

					randomPlatform.max.y += correction;
					if (option % 2 == 1) // wypycha
						randomPlatform.min.y += correction;
				}
				else if (option / 2 == 4) // +z
				{
					int correction = fakePlatforms[i].max.z - randomPlatform.min.z + overcorrectDistrib(levelSeedEngine);

					randomPlatform.min.z += correction;
					if (option % 2 == 1) // wypycha
						randomPlatform.max.z += correction;
				}
				else // -z
				{
					int correction = fakePlatforms[i].min.z - randomPlatform.max.z - overcorrectDistrib(levelSeedEngine);

					randomPlatform.max.z += correction;
					if (option % 2 == 1) // wypycha
						randomPlatform.min.z += correction;
				}

				if (randomPlatform.min.x >= 140 || randomPlatform.max.x <= -140 ||
					randomPlatform.min.y >= 400 || randomPlatform.max.y <= -400 ||
					randomPlatform.min.z >= 140 || randomPlatform.max.z <= -140)
					randomPlatform = backup;
			}
		}

		// poprawki ze wzgledu na edgeSafeZones
		// jesli znajduje sie na ktorejs krawedzi to sprawdza jej strefy
		// jesli znajduje sie w ktorejs strefie, to juz sie nie przejmuje ta krawedzia
		// jesli dla jakiejs krawedzi na ktorej lezy nie znajduje sie w zadnej strefie w calosci,
		// to albo wypycha/ucina z tej krawedzi, albo wypycha/ucina do losowej strefy (tylko 6 opcji tutaj)
		if (randomPlatform.min.z < 140 && randomPlatform.max.z > 60) // gora, +z
		{
			if (randomPlatform.min.x >= edgeSafeZones[0].min.x && randomPlatform.max.x <= edgeSafeZones[0].max.x &&
				randomPlatform.min.y >= edgeSafeZones[0].min.y && randomPlatform.max.y <= edgeSafeZones[0].max.y ||
				randomPlatform.min.x >= edgeSafeZones[1].min.x && randomPlatform.max.x <= edgeSafeZones[1].max.x &&
				randomPlatform.min.y >= edgeSafeZones[1].min.y && randomPlatform.max.y <= edgeSafeZones[1].max.y)
			{
				// jesli znajduje sie w ktorejs strefie, to juz sie nie przejmuje ta krawedzia
			}
			else
			{
				noCollision = false;
				attemptCount++;

				int option = optionDistrib(levelSeedEngine);
				if (option / 4 == 0) // krawedz
				{
					if (option % 2 == 0) // ucina
						randomPlatform.max.z = 60;
					else // wypycha
					{
						int correction = randomPlatform.max.z - 60;
						randomPlatform.min.z -= correction;
						randomPlatform.max.z -= correction;
					}
				}
				else // odpowiednia strefa
				{
					int safeZoneIndex = option / 4 - 1 + 0;
					glm::ivec3 correction(0);
					if (randomPlatform.min.x < edgeSafeZones[safeZoneIndex].min.x)
						correction.x = edgeSafeZones[safeZoneIndex].min.x - randomPlatform.min.x;
					if (randomPlatform.max.x > edgeSafeZones[safeZoneIndex].max.x &&
						(correction.x == 0 || randomPlatform.max.x - edgeSafeZones[safeZoneIndex].max.x < correction.x))
						correction.x = edgeSafeZones[safeZoneIndex].max.x - randomPlatform.max.x;
					if (correction.x != 0)
					{
						int overcorrection = overcorrectDistrib(levelSeedEngine) - 1;
						correction.x += correction.x > 0 ? overcorrection : -overcorrection;
					}
					if (randomPlatform.min.y < edgeSafeZones[safeZoneIndex].min.y)
						correction.y = edgeSafeZones[safeZoneIndex].min.y - randomPlatform.min.y;
					if (randomPlatform.max.y > edgeSafeZones[safeZoneIndex].max.y &&
						(correction.y == 0 || randomPlatform.max.y - edgeSafeZones[safeZoneIndex].max.y < correction.y))
						correction.y = edgeSafeZones[safeZoneIndex].max.y - randomPlatform.max.y;
					if (correction.y != 0)
					{
						int overcorrection = overcorrectDistrib(levelSeedEngine) - 1;
						correction.y += correction.y > 0 ? overcorrection : -overcorrection;
					}

					if (option % 2 == 0) // ucina
					{
						if (correction.x > 0)
							randomPlatform.min.x += correction.x;
						else
							randomPlatform.max.x += correction.x;

						if (correction.y > 0)
							randomPlatform.min.y += correction.y;
						else
							randomPlatform.max.y += correction.y;
					}
					else // wypycha
					{
						randomPlatform.min += correction;
						randomPlatform.max += correction;
					}
				}
			}
		}
		if (randomPlatform.min.x < 140 && randomPlatform.max.x > 60) // prawo, +x
		{
			if (randomPlatform.min.z >= edgeSafeZones[2].min.z && randomPlatform.max.z <= edgeSafeZones[2].max.z &&
				randomPlatform.min.y >= edgeSafeZones[2].min.y && randomPlatform.max.y <= edgeSafeZones[2].max.y ||
				randomPlatform.min.z >= edgeSafeZones[3].min.z && randomPlatform.max.z <= edgeSafeZones[3].max.z &&
				randomPlatform.min.y >= edgeSafeZones[3].min.y && randomPlatform.max.y <= edgeSafeZones[3].max.y)
			{
				// jesli znajduje sie w ktorejs strefie, to juz sie nie przejmuje ta krawedzia
			}
			else
			{
				noCollision = false;
				attemptCount++;

				int option = optionDistrib(levelSeedEngine);
				if (option / 4 == 0) // krawedz
				{
					if (option % 2 == 0) // ucina
						randomPlatform.max.x = 60;
					else // wypycha
					{
						int correction = randomPlatform.max.x - 60;
						randomPlatform.min.x -= correction;
						randomPlatform.max.x -= correction;
					}
				}
				else // odpowiednia strefa
				{
					int safeZoneIndex = option / 4 - 1 + 2;
					glm::ivec3 correction(0);
					if (randomPlatform.min.z < edgeSafeZones[safeZoneIndex].min.z)
						correction.z = edgeSafeZones[safeZoneIndex].min.z - randomPlatform.min.z;
					if (randomPlatform.max.z > edgeSafeZones[safeZoneIndex].max.z &&
						(correction.z == 0 || randomPlatform.max.z - edgeSafeZones[safeZoneIndex].max.z < correction.z))
						correction.z = edgeSafeZones[safeZoneIndex].max.z - randomPlatform.max.z;
					if (correction.z != 0)
					{
						int overcorrection = overcorrectDistrib(levelSeedEngine) - 1;
						correction.z += correction.z > 0 ? overcorrection : -overcorrection;
					}
					if (randomPlatform.min.y < edgeSafeZones[safeZoneIndex].min.y)
						correction.y = edgeSafeZones[safeZoneIndex].min.y - randomPlatform.min.y;
					if (randomPlatform.max.y > edgeSafeZones[safeZoneIndex].max.y &&
						(correction.y == 0 || randomPlatform.max.y - edgeSafeZones[safeZoneIndex].max.y < correction.y))
						correction.y = edgeSafeZones[safeZoneIndex].max.y - randomPlatform.max.y;
					if (correction.y != 0)
					{
						int overcorrection = overcorrectDistrib(levelSeedEngine) - 1;
						correction.y += correction.y > 0 ? overcorrection : -overcorrection;
					}

					if (option % 2 == 0) // ucina
					{
						if (correction.z > 0)
							randomPlatform.min.z += correction.z;
						else
							randomPlatform.max.z += correction.z;

						if (correction.y > 0)
							randomPlatform.min.y += correction.y;
						else
							randomPlatform.max.y += correction.y;
					}
					else // wypycha
					{
						randomPlatform.min += correction;
						randomPlatform.max += correction;
					}
				}
			}
		}
		if (randomPlatform.min.z < -60 && randomPlatform.max.z > -140) // dol, -z
		{
			if (randomPlatform.min.x >= edgeSafeZones[4].min.x && randomPlatform.max.x <= edgeSafeZones[4].max.x &&
				randomPlatform.min.y >= edgeSafeZones[4].min.y && randomPlatform.max.y <= edgeSafeZones[4].max.y ||
				randomPlatform.min.x >= edgeSafeZones[5].min.x && randomPlatform.max.x <= edgeSafeZones[5].max.x &&
				randomPlatform.min.y >= edgeSafeZones[5].min.y && randomPlatform.max.y <= edgeSafeZones[5].max.y)
			{
				// jesli znajduje sie w ktorejs strefie, to juz sie nie przejmuje ta krawedzia
			}
			else
			{
				noCollision = false;
				attemptCount++;

				int option = optionDistrib(levelSeedEngine);
				if (option / 4 == 0) // krawedz
				{
					if (option % 2 == 0) // ucina
						randomPlatform.min.z = -60;
					else // wypycha
					{
						int correction = randomPlatform.min.z + 60;
						randomPlatform.min.z -= correction;
						randomPlatform.max.z -= correction;
					}
				}
				else // odpowiednia strefa
				{
					int safeZoneIndex = option / 4 - 1 + 4;
					glm::ivec3 correction(0);
					if (randomPlatform.min.x < edgeSafeZones[safeZoneIndex].min.x)
						correction.x = edgeSafeZones[safeZoneIndex].min.x - randomPlatform.min.x;
					if (randomPlatform.max.x > edgeSafeZones[safeZoneIndex].max.x &&
						(correction.x == 0 || randomPlatform.max.x - edgeSafeZones[safeZoneIndex].max.x < correction.x))
						correction.x = edgeSafeZones[safeZoneIndex].max.x - randomPlatform.max.x;
					if (correction.x != 0)
					{
						int overcorrection = overcorrectDistrib(levelSeedEngine) - 1;
						correction.x += correction.x > 0 ? overcorrection : -overcorrection;
					}
					if (randomPlatform.min.y < edgeSafeZones[safeZoneIndex].min.y)
						correction.y = edgeSafeZones[safeZoneIndex].min.y - randomPlatform.min.y;
					if (randomPlatform.max.y > edgeSafeZones[safeZoneIndex].max.y &&
						(correction.y == 0 || randomPlatform.max.y - edgeSafeZones[safeZoneIndex].max.y < correction.y))
						correction.y = edgeSafeZones[safeZoneIndex].max.y - randomPlatform.max.y;
					if (correction.y != 0)
					{
						int overcorrection = overcorrectDistrib(levelSeedEngine) - 1;
						correction.y += correction.y > 0 ? overcorrection : -overcorrection;
					}

					if (option % 2 == 0) // ucina
					{
						if (correction.x > 0)
							randomPlatform.min.x += correction.x;
						else
							randomPlatform.max.x += correction.x;

						if (correction.y > 0)
							randomPlatform.min.y += correction.y;
						else
							randomPlatform.max.y += correction.y;
					}
					else // wypycha
					{
						randomPlatform.min += correction;
						randomPlatform.max += correction;
					}
				}
			}
		}
		if (randomPlatform.min.x < -60 && randomPlatform.max.x > -140) // lewo, -x
		{
			if (randomPlatform.min.z >= edgeSafeZones[6].min.z && randomPlatform.max.z <= edgeSafeZones[6].max.z &&
				randomPlatform.min.y >= edgeSafeZones[6].min.y && randomPlatform.max.y <= edgeSafeZones[6].max.y ||
				randomPlatform.min.z >= edgeSafeZones[7].min.z && randomPlatform.max.z <= edgeSafeZones[7].max.z &&
				randomPlatform.min.y >= edgeSafeZones[7].min.y && randomPlatform.max.y <= edgeSafeZones[7].max.y)
			{
				// jesli znajduje sie w ktorejs strefie, to juz sie nie przejmuje ta krawedzia
			}
			else
			{
				noCollision = false;
				attemptCount++;

				int option = optionDistrib(levelSeedEngine);
				if (option / 4 == 0) // krawedz
				{
					if (option % 2 == 0) // ucina
						randomPlatform.min.x = -60;
					else // wypycha
					{
						int correction = randomPlatform.min.x + 60;
						randomPlatform.min.x -= correction;
						randomPlatform.max.x -= correction;
					}
				}
				else // odpowiednia strefa
				{
					int safeZoneIndex = option / 4 - 1 + 6;
					glm::ivec3 correction(0);
					if (randomPlatform.min.z < edgeSafeZones[safeZoneIndex].min.z)
						correction.z = edgeSafeZones[safeZoneIndex].min.z - randomPlatform.min.z;
					if (randomPlatform.max.z > edgeSafeZones[safeZoneIndex].max.z &&
						(correction.z == 0 || randomPlatform.max.z - edgeSafeZones[safeZoneIndex].max.z < correction.z))
						correction.z = edgeSafeZones[safeZoneIndex].max.z - randomPlatform.max.z;
					if (correction.z != 0)
					{
						int overcorrection = overcorrectDistrib(levelSeedEngine) - 1;
						correction.z += correction.z > 0 ? overcorrection : -overcorrection;
					}
					if (randomPlatform.min.y < edgeSafeZones[safeZoneIndex].min.y)
						correction.y = edgeSafeZones[safeZoneIndex].min.y - randomPlatform.min.y;
					if (randomPlatform.max.y > edgeSafeZones[safeZoneIndex].max.y &&
						(correction.y == 0 || randomPlatform.max.y - edgeSafeZones[safeZoneIndex].max.y < correction.y))
						correction.y = edgeSafeZones[safeZoneIndex].max.y - randomPlatform.max.y;
					if (correction.y != 0)
					{
						int overcorrection = overcorrectDistrib(levelSeedEngine) - 1;
						correction.y += correction.y > 0 ? overcorrection : -overcorrection;
					}

					if (option % 2 == 0) // ucina
					{
						if (correction.z > 0)
							randomPlatform.min.z += correction.z;
						else
							randomPlatform.max.z += correction.z;

						if (correction.y > 0)
							randomPlatform.min.y += correction.y;
						else
							randomPlatform.max.y += correction.y;
					}
					else // wypycha
					{
						randomPlatform.min += correction;
						randomPlatform.max += correction;
					}
				}
			}
		}

		// poprawki ze wzgledu na istniejace randomPlatforms (to samo co fakePlatforms, poza anulowaniem)
		for (unsigned int i = 0; i < randomPlatforms.size(); i++)
		{
			if (randomPlatform.min.x <= randomPlatforms[i].max.x && randomPlatform.max.x >= randomPlatforms[i].min.x &&
				randomPlatform.min.y <= randomPlatforms[i].max.y && randomPlatform.max.y >= randomPlatforms[i].min.y &&
				randomPlatform.min.z <= randomPlatforms[i].max.z && randomPlatform.max.z >= randomPlatforms[i].min.z)
			{
				noCollision = false;
				attemptCount++;

				int option = optionDistrib(levelSeedEngine);
				if (option / 2 == 0) // +x
				{
					int correction = randomPlatforms[i].max.x - randomPlatform.min.x + overcorrectDistrib(levelSeedEngine);

					randomPlatform.min.x += correction;
					if (option % 2 == 1) // wypycha
						randomPlatform.max.x += correction;
				}
				else if (option / 2 == 1) // -x
				{
					int correction = randomPlatforms[i].min.x - randomPlatform.max.x - overcorrectDistrib(levelSeedEngine);

					randomPlatform.max.x += correction;
					if (option % 2 == 1) // wypycha
						randomPlatform.min.x += correction;
				}
				else if (option / 2 == 2) // +y
				{
					int correction = randomPlatforms[i].max.y - randomPlatform.min.y + overcorrectDistrib(levelSeedEngine);

					randomPlatform.min.y += correction;
					if (option % 2 == 1) // wypycha
						randomPlatform.max.y += correction;
				}
				else if (option / 2 == 3) // -y
				{
					int correction = randomPlatforms[i].min.y - randomPlatform.max.y - overcorrectDistrib(levelSeedEngine);

					randomPlatform.max.y += correction;
					if (option % 2 == 1) // wypycha
						randomPlatform.min.y += correction;
				}
				else if (option / 2 == 4) // +z
				{
					int correction = randomPlatforms[i].max.z - randomPlatform.min.z + overcorrectDistrib(levelSeedEngine);

					randomPlatform.min.z += correction;
					if (option % 2 == 1) // wypycha
						randomPlatform.max.z += correction;
				}
				else // -z
				{
					int correction = randomPlatforms[i].min.z - randomPlatform.max.z - overcorrectDistrib(levelSeedEngine);

					randomPlatform.max.z += correction;
					if (option % 2 == 1) // wypycha
						randomPlatform.min.z += correction;
				}
			}
		}

		if (randomPlatform.min.x <= -140 || randomPlatform.max.x >= 140 ||
			randomPlatform.min.z <= -140 || randomPlatform.max.z >= 140)
		{
			noCollision = false;
			attemptCount++;

			int overcorrection;
			if (randomPlatform.min.x <= -140)
			{
				overcorrection = overcorrectDistrib(levelSeedEngine);
				randomPlatform.min.x = overcorrection - 140;
			}
			if (randomPlatform.max.x >= 140)
			{
				overcorrection = overcorrectDistrib(levelSeedEngine);
				randomPlatform.max.x = 140 - overcorrection;
			}
			if (randomPlatform.min.z <= -140)
			{
				overcorrection = overcorrectDistrib(levelSeedEngine);
				randomPlatform.min.z = overcorrection - 140;
			}
			if (randomPlatform.max.z >= 140)
			{
				overcorrection = overcorrectDistrib(levelSeedEngine);
				randomPlatform.max.z = 140 - overcorrection;
			}
		}

		if (randomPlatform.min.y < -400 || randomPlatform.max.y > 400)
		{
			noCollision = false;
			attemptCount++;

			randomPlatform.min.y = randomPlatform.min.y < -400 ? -400 : randomPlatform.min.y;
			randomPlatform.max.y = randomPlatform.max.y > 400 ? 400 : randomPlatform.max.y;
		}

		if (randomPlatform.min.x >= randomPlatform.max.x ||
			randomPlatform.min.y >= randomPlatform.max.y ||
			randomPlatform.min.z >= randomPlatform.max.z)
		{
			// degenerate platform
			noCollision = false;
			attemptCount++;
			startNewPlatform = true;
		}

		if (noCollision)
		{
			randomPlatforms.push_back(randomPlatform);
			startNewPlatform = true;
		}
	}

	std::uniform_int_distribution<int> pDistrib(0, 1);
	for (unsigned int i = 0; i < randomPlatforms.size(); i++)
	{
		int p = pDistrib(levelSeedEngine);
		platform adjustedPlatform;
		adjustedPlatform.pos = randomPlatforms[i].min;
		adjustedPlatform.pos /= 2.0f;
		adjustedPlatform.dim = randomPlatforms[i].max - randomPlatforms[i].min;
		adjustedPlatform.dim /= 2.0f;
		adjustedPlatform.pos.x += chunkCoords.x * 100.0f;
		adjustedPlatform.pos.z += chunkCoords.y * 100.0f;

		chunkMatrix[spiralCoords.x + 6][spiralCoords.y + 6]->platforms[p].push_back(adjustedPlatform);
	}
}

void ChunkMan::applyManuals(glm::ivec2 spiralCoords)
{
	// tych wspolrzednych szukamy w deletions i additions
	glm::ivec2 chunkCoords = spiralCoords + currentChunk;
	Chunk *c = chunkMatrix[spiralCoords.x + 6][spiralCoords.y + 6];

	for (unsigned int i = 0; i < manualDeletions.size(); i++)
	{
		if (manualDeletions[i].chunkPos != chunkCoords)
			continue;

		bool foundDeletion = false;
		for (int p = 0; p < 2 && !foundDeletion; p++)
		{
			for(unsigned int j = 0; j < c->platforms[p].size() && !foundDeletion; j++)
			{
				glm::vec3 delPos = manualDeletions[i].deletion;
				delPos.x += manualDeletions[i].chunkPos.x * 100.0f;
				delPos.z += manualDeletions[i].chunkPos.y * 100.0f;

				if (c->platforms[p][j].pos == delPos)
				{
					c->platforms[p].erase(c->platforms[p].begin() + j);
					foundDeletion = true;
				}
			}
		}
	}

	for (unsigned int i = 0; i < manualAdditions.size(); i++)
	{
		if (manualAdditions[i].chunkPos != chunkCoords)
			continue;

		platform addPltfrm = manualAdditions[i].addition;
		addPltfrm.pos.x += manualAdditions[i].chunkPos.x * 100.0f;
		addPltfrm.pos.z += manualAdditions[i].chunkPos.y * 100.0f;

		int p = manualAdditions[i].type ? 1 : 0;
		c->platforms[p].push_back(addPltfrm);
	}
}

const glm::ivec2 ChunkMan::spiralOrder[spiralCount] =
{
	glm::ivec2(0, 0), glm::ivec2(0, 1), glm::ivec2(1, 1), glm::ivec2(1, 0),
	glm::ivec2(1, -1), glm::ivec2(0, -1), glm::ivec2(-1, -1), glm::ivec2(-1, 0), glm::ivec2(-1, 1),
	glm::ivec2(-1, 2), glm::ivec2(0, 2), glm::ivec2(1, 2), glm::ivec2(2, 2), glm::ivec2(2, 1),
	glm::ivec2(2, 0), glm::ivec2(2, -1), glm::ivec2(2, -2), glm::ivec2(1, -2), glm::ivec2(0, -2),
	glm::ivec2(-1, -2), glm::ivec2(-2, -2), glm::ivec2(-2, -1), glm::ivec2(-2, 0), glm::ivec2(-2, 1),
	glm::ivec2(-2, 2), glm::ivec2(-2, 3), glm::ivec2(-1, 3), glm::ivec2(0, 3), glm::ivec2(1, 3),
	glm::ivec2(2, 3), glm::ivec2(3, 3), glm::ivec2(3, 2), glm::ivec2(3, 1), glm::ivec2(3, 0),
	glm::ivec2(3, -1), glm::ivec2(3, -2), glm::ivec2(3, -3), glm::ivec2(2, -3), glm::ivec2(1, -3),
	glm::ivec2(0, -3), glm::ivec2(-1, -3), glm::ivec2(-2, -3), glm::ivec2(-3, -3), glm::ivec2(-3, -2),
	glm::ivec2(-3, -1), glm::ivec2(-3, 0), glm::ivec2(-3, 1), glm::ivec2(-3, 2), glm::ivec2(-3, 3),
	glm::ivec2(-3, 4), glm::ivec2(-2, 4), glm::ivec2(-1, 4), glm::ivec2(0, 4), glm::ivec2(1, 4),
	glm::ivec2(2, 4), glm::ivec2(3, 4), glm::ivec2(4, 4), glm::ivec2(4, 3), glm::ivec2(4, 2),
	glm::ivec2(4, 1), glm::ivec2(4, 0), glm::ivec2(4, -1), glm::ivec2(4, -2), glm::ivec2(4, -3),
	glm::ivec2(4, -4), glm::ivec2(3, -4), glm::ivec2(2, -4), glm::ivec2(1, -4), glm::ivec2(0, -4),
	glm::ivec2(-1, -4), glm::ivec2(-2, -4), glm::ivec2(-3, -4), glm::ivec2(-4, -4), glm::ivec2(-4, -3),
	glm::ivec2(-4, -2), glm::ivec2(-4, -1), glm::ivec2(-4, 0), glm::ivec2(-4, 1), glm::ivec2(-4, 2),
	glm::ivec2(-4, 3), glm::ivec2(-4, 4), glm::ivec2(-3, 5), glm::ivec2(-2, 5), glm::ivec2(-1, 5),
	glm::ivec2(0, 5), glm::ivec2(1, 5), glm::ivec2(2, 5), glm::ivec2(3, 5), glm::ivec2(5, 3),
	glm::ivec2(5, 2), glm::ivec2(5, 1), glm::ivec2(5, 0), glm::ivec2(5, -1), glm::ivec2(5, -2),
	glm::ivec2(5, -3), glm::ivec2(3, -5), glm::ivec2(2, -5), glm::ivec2(1, -5), glm::ivec2(0, -5),
	glm::ivec2(-1, -5), glm::ivec2(-2, -5), glm::ivec2(-3, -5), glm::ivec2(-5, -3), glm::ivec2(-5, -2),
	glm::ivec2(-5, -1), glm::ivec2(-5, 0), glm::ivec2(-5, 1), glm::ivec2(-5, 2), glm::ivec2(-5, 3),
	glm::ivec2(-1, 6), glm::ivec2(0, 6), glm::ivec2(1, 6), glm::ivec2(6, 1), glm::ivec2(6, 0),
	glm::ivec2(6, -1), glm::ivec2(1, -6), glm::ivec2(0, -6), glm::ivec2(-1, -6), glm::ivec2(-6, -1),
	glm::ivec2(-6, 0), glm::ivec2(-6, 1)
};
