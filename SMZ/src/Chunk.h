#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <vector>

#include "helperStructs.h"

class Chunk
{
public:
	Chunk();
	~Chunk();

	void genChunkGeo();

	bool valuesGenerated;
	int genValues[20];

	bool dataGenerated;
	std::vector<platform> platforms[2];
	unsigned int platformVAO[2], platformVBO[2];
	float* platformVert[2];
	unsigned int platformVertSize[2];
private:
	void cleanChunkData();

	void genChunkGeoForType(unsigned int type);
};
