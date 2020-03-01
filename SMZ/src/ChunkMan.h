#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>
#include <random>

#include "constantsGameplay.h"
#include "constantsRender.h"
#include "helperStructs.h"
#include "Chunk.h"
#include "Player.h"

class ChunkMan
{
public:
	ChunkMan(std::string name, unsigned int progPlatform, unsigned int progShadow, unsigned int progCube);
	~ChunkMan();
	void update(glm::vec3 playerPos);
	void draw(glm::mat4 view, glm::mat4 proj, glm::vec3 viewPos, glm::vec3 playerPyr);
	void drawDepthMap(glm::vec3 camPos, glm::vec3 pyr);
	void drawChunkPlatformsDepth(Chunk *c);
	void calcDepthMapHelpers(float fovY, float fovX);
	void invalidateCascades();
	
	void restartLevel();

	void collidePlayer(Player *p, float deltaTime);

	unsigned int seed[cnst::seedLength] = { 0 };
	glm::vec3 accent;
	glm::vec3 ambient;
	sunLight sun;

	unsigned int restartCount;

	glm::ivec2 initialPlayerChunk = glm::ivec2(0);
	glm::vec3 initialPlayerPos = glm::vec3(0);
	float initialPlayerYaw = 0;
private:
	bool collideSingle(glm::vec3 pos, float r, float h, platform plt, glm::vec3 *cor, bool *closeToWall);
	platform calcCascadeProjection(glm::vec3 pos, glm::vec3 pyr, int cascadeIndex, glm::mat4 lightView);
	void initDraw();
	void genDepthMaps();
	void genJitterTable();

	glm::mat4 mulRPY = glm::mat4(1.0f);
	glm::vec4 subfrustum[8] = { glm::vec4(0.0f) };
	float ratioY, ratioX;

	std::random_device trueRandom;
	std::mt19937 randomEngine, levelSeedEngine;

	glm::ivec2 currentChunk = glm::ivec2(0);
	Chunk *chunkMatrix[13][13];
	bool chunkValid[13][13];
	std::vector<glm::ivec2> newChunkCoords;

	void leftShift(int n);
	void rightShift(int n);
	void downShift(int n);
	void upShift(int n);
	void cleanChunks();

	void readLevelData(std::string name);
	void generateLevelParameters(bool loadedPlayer, bool loadedAccent, bool loadedAmbient, bool loadedSun);
	void initSeed(std::string seedString);
	unsigned int getSeedPortion(unsigned int pos, unsigned int len);
	void seedLevelEngine(glm::ivec2 chunkCoords);
	void genChunkValues(int chunkArray[20]);
	void genPlatforms(glm::ivec2 spiralCoords);
	void applyManuals(glm::ivec2 spiralCoords);

	std::vector<manualAdd> manualAdditions;
	std::vector<manualDel> manualDeletions;

	// std::vector<Chunk *> staleChunks - do zmniejszania skali chunkow ktore sa wyladowywane,
	// zeby nie znikaly tak po prostu moze
	// dopiero jak skala osiagnie odpowiednio mala wartosc to robimy delete

	unsigned int platformTextureID;
	unsigned int platformSpecularID;
	unsigned int platformNormalID;
	unsigned int diffuseLutID;

	unsigned int cubeVAO, cubeVBO, cubeTextureID;
	unsigned int sunLutID;

	unsigned int depthMapFBO[cnst::cascadeCount], depthMapID[cnst::cascadeCount];
	glm::mat4 lightSpaceMatrix[cnst::cascadeCount] = { glm::mat4(1.0f) };
	float cascadeZBounds[cnst::cascadeCount + 1] = { 0.0f };
	float cascadeRadius[cnst::cascadeCount] = { 0.0f };
	bool cascadeValid[cnst::cascadeCount] = { false };
	glm::vec3 cascadeCenter[cnst::cascadeCount] = { glm::vec3(0.0f) };
	platform cascadeBoundingBox[cnst::cascadeCount];
	glm::vec2 jitterTable[cnst::jitterSampleCount] = { glm::vec2(1.0f) };

	unsigned int progPlatformShader;
	unsigned int progShadowShader;
	unsigned int progCubeShader;

	// platform shader uniform locations
	int sunDirLoc;
	int viewPosLoc;
	int VPLoc;
	int lightSpaceMatrixLoc;
	int cascadeEndClipSpaceLoc;
	int accentLoc;

	int lastGenIndex = 0;
	int validCount = 0;
	const static int spiralCount = 121;
	const static glm::ivec2 spiralOrder[spiralCount];
};
