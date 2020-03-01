#include "Chunk.h"

#include "constantsRender.h"
#include "utility.h"

Chunk::Chunk()
{
	dataGenerated = false;

	platformVAO[0] = 0;
	platformVAO[1] = 0;
	platformVBO[0] = 0;
	platformVBO[1] = 0;
	platformVert[0] = nullptr;
	platformVert[1] = nullptr;
	platformVertSize[0] = 0;
	platformVertSize[1] = 0;
}

Chunk::~Chunk()
{
	cleanChunkData();
}

void Chunk::genChunkGeo()
{
	if (dataGenerated)
		cleanChunkData();

	for (int i = 0; i < 2; i++)
	{
		if (platforms[i].size() > 0)
			genChunkGeoForType(i);
	}

	dataGenerated = true;
}

void Chunk::cleanChunkData()
{
	for (int i = 0; i < 2; i++)
	{
		platforms[i].clear();
		delete[] platformVert[i];
	}

	glDeleteVertexArrays(2, platformVAO);
	glDeleteBuffers(2, platformVBO);

	dataGenerated = false;
}

void Chunk::genChunkGeoForType(unsigned int type)
{
	// tworzymy tablice wierzcholkow, pozycji tekstur i normali
	// tekstury maja sie powtarzac wiec ustawiamy je na pozycje wierzcholkow XZ, XY albo YZ wymnozone przez skale w cnst
	// poza tym to po prostu rozciagniete szesciany

	unsigned int pat = 0; // platformAttribTotal
	for (int i = 0; i < cnst::platformAttribCount; i++)
		pat += cnst::platformAttribSizes[i];

	// 36 wierzcholkow na platforme
	platformVertSize[type] = sizeof(float) * platforms[type].size() * 36 * pat;
	platformVert[type] = new float[platforms[type].size() * 36 * pat];

	for (unsigned int i = 0; i < platforms[type].size(); i++)
	{
		// kierunek (-1,  0,  0)
		// trojkat 1
		platformVert[type][i * 36 * pat + 0 * pat + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * pat + 0 * pat + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * pat + 0 * pat + 2] = platforms[type][i].pos.z;

		platformVert[type][i * 36 * pat + 0 * pat + 3] = platforms[type][i].pos.z * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 0 * pat + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 0 * pat + 5] = -1.0f;
		platformVert[type][i * 36 * pat + 0 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 0 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 0 * pat + 8] = 0.0f;
		platformVert[type][i * 36 * pat + 0 * pat + 9] = 0.0f;
		platformVert[type][i * 36 * pat + 0 * pat + 10] = platforms[type][i].dim.z;
		platformVert[type][i * 36 * pat + 0 * pat + 11] = platforms[type][i].dim.y;
		//
		platformVert[type][i * 36 * pat + 1 * pat + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * pat + 1 * pat + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 1 * pat + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;

		platformVert[type][i * 36 * pat + 1 * pat + 3] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 1 * pat + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 1 * pat + 5] = -1.0f;
		platformVert[type][i * 36 * pat + 1 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 1 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 1 * pat + 8] = platforms[type][i].dim.z;
		platformVert[type][i * 36 * pat + 1 * pat + 9] = platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 1 * pat + 10] = 0.0f;
		platformVert[type][i * 36 * pat + 1 * pat + 11] = 0.0f;
		//
		platformVert[type][i * 36 * pat + 2 * pat + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * pat + 2 * pat + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 2 * pat + 2] = platforms[type][i].pos.z;

		platformVert[type][i * 36 * pat + 2 * pat + 3] = platforms[type][i].pos.z * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 2 * pat + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 2 * pat + 5] = -1.0f;
		platformVert[type][i * 36 * pat + 2 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 2 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 2 * pat + 8] = 0.0f;
		platformVert[type][i * 36 * pat + 2 * pat + 9] = platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 2 * pat + 10] = platforms[type][i].dim.z;
		platformVert[type][i * 36 * pat + 2 * pat + 11] = 0.0f;

		// trojkat 2
		platformVert[type][i * 36 * pat + 3 * pat + 0] = platformVert[type][i * 36 * pat + 0 * pat + 0];
		platformVert[type][i * 36 * pat + 3 * pat + 1] = platformVert[type][i * 36 * pat + 0 * pat + 1];
		platformVert[type][i * 36 * pat + 3 * pat + 2] = platformVert[type][i * 36 * pat + 0 * pat + 2];

		platformVert[type][i * 36 * pat + 3 * pat + 3] = platformVert[type][i * 36 * pat + 0 * pat + 3];
		platformVert[type][i * 36 * pat + 3 * pat + 4] = platformVert[type][i * 36 * pat + 0 * pat + 4];

		platformVert[type][i * 36 * pat + 3 * pat + 5] = platformVert[type][i * 36 * pat + 0 * pat + 5];
		platformVert[type][i * 36 * pat + 3 * pat + 6] = platformVert[type][i * 36 * pat + 0 * pat + 6];
		platformVert[type][i * 36 * pat + 3 * pat + 7] = platformVert[type][i * 36 * pat + 0 * pat + 7];

		platformVert[type][i * 36 * pat + 3 * pat + 8] = platformVert[type][i * 36 * pat + 0 * pat + 8];
		platformVert[type][i * 36 * pat + 3 * pat + 9] = platformVert[type][i * 36 * pat + 0 * pat + 9];
		platformVert[type][i * 36 * pat + 3 * pat + 10] = platformVert[type][i * 36 * pat + 0 * pat + 10];
		platformVert[type][i * 36 * pat + 3 * pat + 11] = platformVert[type][i * 36 * pat + 0 * pat + 11];
		//
		platformVert[type][i * 36 * pat + 4 * pat + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * pat + 4 * pat + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * pat + 4 * pat + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;

		platformVert[type][i * 36 * pat + 4 * pat + 3] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 4 * pat + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 4 * pat + 5] = -1.0f;
		platformVert[type][i * 36 * pat + 4 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 4 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 4 * pat + 8] = platforms[type][i].dim.z;
		platformVert[type][i * 36 * pat + 4 * pat + 9] = 0.0f;
		platformVert[type][i * 36 * pat + 4 * pat + 10] = 0.0f;
		platformVert[type][i * 36 * pat + 4 * pat + 11] = platforms[type][i].dim.y;
		//
		platformVert[type][i * 36 * pat + 5 * pat + 0] = platformVert[type][i * 36 * pat + 1 * pat + 0];
		platformVert[type][i * 36 * pat + 5 * pat + 1] = platformVert[type][i * 36 * pat + 1 * pat + 1];
		platformVert[type][i * 36 * pat + 5 * pat + 2] = platformVert[type][i * 36 * pat + 1 * pat + 2];

		platformVert[type][i * 36 * pat + 5 * pat + 3] = platformVert[type][i * 36 * pat + 1 * pat + 3];
		platformVert[type][i * 36 * pat + 5 * pat + 4] = platformVert[type][i * 36 * pat + 1 * pat + 4];

		platformVert[type][i * 36 * pat + 5 * pat + 5] = platformVert[type][i * 36 * pat + 1 * pat + 5];
		platformVert[type][i * 36 * pat + 5 * pat + 6] = platformVert[type][i * 36 * pat + 1 * pat + 6];
		platformVert[type][i * 36 * pat + 5 * pat + 7] = platformVert[type][i * 36 * pat + 1 * pat + 7];

		platformVert[type][i * 36 * pat + 5 * pat + 8] = platformVert[type][i * 36 * pat + 1 * pat + 8];
		platformVert[type][i * 36 * pat + 5 * pat + 9] = platformVert[type][i * 36 * pat + 1 * pat + 9];
		platformVert[type][i * 36 * pat + 5 * pat + 10] = platformVert[type][i * 36 * pat + 1 * pat + 10];
		platformVert[type][i * 36 * pat + 5 * pat + 11] = platformVert[type][i * 36 * pat + 1 * pat + 11];

		// kierunek ( 0,  0,  1)
		// trojkat 1
		platformVert[type][i * 36 * pat + 6 * pat + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * pat + 6 * pat + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * pat + 6 * pat + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;

		platformVert[type][i * 36 * pat + 6 * pat + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 6 * pat + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 6 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 6 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 6 * pat + 7] = 1.0f;

		platformVert[type][i * 36 * pat + 6 * pat + 8] = 0.0f;
		platformVert[type][i * 36 * pat + 6 * pat + 9] = 0.0f;
		platformVert[type][i * 36 * pat + 6 * pat + 10] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 6 * pat + 11] = platforms[type][i].dim.y;
		//
		platformVert[type][i * 36 * pat + 7 * pat + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 7 * pat + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 7 * pat + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;

		platformVert[type][i * 36 * pat + 7 * pat + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 7 * pat + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 7 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 7 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 7 * pat + 7] = 1.0f;

		platformVert[type][i * 36 * pat + 7 * pat + 8] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 7 * pat + 9] = platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 7 * pat + 10] = 0.0f;
		platformVert[type][i * 36 * pat + 7 * pat + 11] = 0.0f;
		//
		platformVert[type][i * 36 * pat + 8 * pat + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * pat + 8 * pat + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 8 * pat + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;

		platformVert[type][i * 36 * pat + 8 * pat + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 8 * pat + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 8 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 8 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 8 * pat + 7] = 1.0f;

		platformVert[type][i * 36 * pat + 8 * pat + 8] = 0.0f;
		platformVert[type][i * 36 * pat + 8 * pat + 9] = platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 8 * pat + 10] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 8 * pat + 11] = 0.0f;

		// trojkat 2
		platformVert[type][i * 36 * pat + 9 * pat + 0] = platformVert[type][i * 36 * pat + 6 * pat + 0];
		platformVert[type][i * 36 * pat + 9 * pat + 1] = platformVert[type][i * 36 * pat + 6 * pat + 1];
		platformVert[type][i * 36 * pat + 9 * pat + 2] = platformVert[type][i * 36 * pat + 6 * pat + 2];

		platformVert[type][i * 36 * pat + 9 * pat + 3] = platformVert[type][i * 36 * pat + 6 * pat + 3];
		platformVert[type][i * 36 * pat + 9 * pat + 4] = platformVert[type][i * 36 * pat + 6 * pat + 4];

		platformVert[type][i * 36 * pat + 9 * pat + 5] = platformVert[type][i * 36 * pat + 6 * pat + 5];
		platformVert[type][i * 36 * pat + 9 * pat + 6] = platformVert[type][i * 36 * pat + 6 * pat + 6];
		platformVert[type][i * 36 * pat + 9 * pat + 7] = platformVert[type][i * 36 * pat + 6 * pat + 7];

		platformVert[type][i * 36 * pat + 9 * pat + 8] = platformVert[type][i * 36 * pat + 6 * pat + 8];
		platformVert[type][i * 36 * pat + 9 * pat + 9] = platformVert[type][i * 36 * pat + 6 * pat + 9];
		platformVert[type][i * 36 * pat + 9 * pat + 10] = platformVert[type][i * 36 * pat + 6 * pat + 10];
		platformVert[type][i * 36 * pat + 9 * pat + 11] = platformVert[type][i * 36 * pat + 6 * pat + 11];
		//
		platformVert[type][i * 36 * pat + 10 * pat + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 10 * pat + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * pat + 10 * pat + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;

		platformVert[type][i * 36 * pat + 10 * pat + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 10 * pat + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 10 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 10 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 10 * pat + 7] = 1.0f;

		platformVert[type][i * 36 * pat + 10 * pat + 8] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 10 * pat + 9] = 0.0f;
		platformVert[type][i * 36 * pat + 10 * pat + 10] = 0.0f;
		platformVert[type][i * 36 * pat + 10 * pat + 11] = platforms[type][i].dim.y;
		//
		platformVert[type][i * 36 * pat + 11 * pat + 0] = platformVert[type][i * 36 * pat + 7 * pat + 0];
		platformVert[type][i * 36 * pat + 11 * pat + 1] = platformVert[type][i * 36 * pat + 7 * pat + 1];
		platformVert[type][i * 36 * pat + 11 * pat + 2] = platformVert[type][i * 36 * pat + 7 * pat + 2];

		platformVert[type][i * 36 * pat + 11 * pat + 3] = platformVert[type][i * 36 * pat + 7 * pat + 3];
		platformVert[type][i * 36 * pat + 11 * pat + 4] = platformVert[type][i * 36 * pat + 7 * pat + 4];

		platformVert[type][i * 36 * pat + 11 * pat + 5] = platformVert[type][i * 36 * pat + 7 * pat + 5];
		platformVert[type][i * 36 * pat + 11 * pat + 6] = platformVert[type][i * 36 * pat + 7 * pat + 6];
		platformVert[type][i * 36 * pat + 11 * pat + 7] = platformVert[type][i * 36 * pat + 7 * pat + 7];

		platformVert[type][i * 36 * pat + 11 * pat + 8] = platformVert[type][i * 36 * pat + 7 * pat + 8];
		platformVert[type][i * 36 * pat + 11 * pat + 9] = platformVert[type][i * 36 * pat + 7 * pat + 9];
		platformVert[type][i * 36 * pat + 11 * pat + 10] = platformVert[type][i * 36 * pat + 7 * pat + 10];
		platformVert[type][i * 36 * pat + 11 * pat + 11] = platformVert[type][i * 36 * pat + 7 * pat + 11];

		// kierunek ( 1,  0,  0)
		// trojkat 1
		platformVert[type][i * 36 * pat + 12 * pat + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 12 * pat + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * pat + 12 * pat + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;

		platformVert[type][i * 36 * pat + 12 * pat + 3] = platforms[type][i].pos.z * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 12 * pat + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 12 * pat + 5] = 1.0f;
		platformVert[type][i * 36 * pat + 12 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 12 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 12 * pat + 8] = 0.0f;
		platformVert[type][i * 36 * pat + 12 * pat + 9] = 0.0f;
		platformVert[type][i * 36 * pat + 12 * pat + 10] = platforms[type][i].dim.z;
		platformVert[type][i * 36 * pat + 12 * pat + 11] = platforms[type][i].dim.y;
		//
		platformVert[type][i * 36 * pat + 13 * pat + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 13 * pat + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 13 * pat + 2] = platforms[type][i].pos.z;

		platformVert[type][i * 36 * pat + 13 * pat + 3] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 13 * pat + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 13 * pat + 5] = 1.0f;
		platformVert[type][i * 36 * pat + 13 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 13 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 13 * pat + 8] = platforms[type][i].dim.z;
		platformVert[type][i * 36 * pat + 13 * pat + 9] = platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 13 * pat + 10] = 0.0f;
		platformVert[type][i * 36 * pat + 13 * pat + 11] = 0.0f;
		//
		platformVert[type][i * 36 * pat + 14 * pat + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 14 * pat + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 14 * pat + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;

		platformVert[type][i * 36 * pat + 14 * pat + 3] = platforms[type][i].pos.z * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 14 * pat + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 14 * pat + 5] = 1.0f;
		platformVert[type][i * 36 * pat + 14 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 14 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 14 * pat + 8] = 0.0f;
		platformVert[type][i * 36 * pat + 14 * pat + 9] = platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 14 * pat + 10] = platforms[type][i].dim.z;
		platformVert[type][i * 36 * pat + 14 * pat + 11] = 0.0f;

		// trojkat 2
		platformVert[type][i * 36 * pat + 15 * pat + 0] = platformVert[type][i * 36 * pat + 12 * pat + 0];
		platformVert[type][i * 36 * pat + 15 * pat + 1] = platformVert[type][i * 36 * pat + 12 * pat + 1];
		platformVert[type][i * 36 * pat + 15 * pat + 2] = platformVert[type][i * 36 * pat + 12 * pat + 2];

		platformVert[type][i * 36 * pat + 15 * pat + 3] = platformVert[type][i * 36 * pat + 12 * pat + 3];
		platformVert[type][i * 36 * pat + 15 * pat + 4] = platformVert[type][i * 36 * pat + 12 * pat + 4];

		platformVert[type][i * 36 * pat + 15 * pat + 5] = platformVert[type][i * 36 * pat + 12 * pat + 5];
		platformVert[type][i * 36 * pat + 15 * pat + 6] = platformVert[type][i * 36 * pat + 12 * pat + 6];
		platformVert[type][i * 36 * pat + 15 * pat + 7] = platformVert[type][i * 36 * pat + 12 * pat + 7];

		platformVert[type][i * 36 * pat + 15 * pat + 8] = platformVert[type][i * 36 * pat + 12 * pat + 8];
		platformVert[type][i * 36 * pat + 15 * pat + 9] = platformVert[type][i * 36 * pat + 12 * pat + 9];
		platformVert[type][i * 36 * pat + 15 * pat + 10] = platformVert[type][i * 36 * pat + 12 * pat + 10];
		platformVert[type][i * 36 * pat + 15 * pat + 11] = platformVert[type][i * 36 * pat + 12 * pat + 11];
		//
		platformVert[type][i * 36 * pat + 16 * pat + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 16 * pat + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * pat + 16 * pat + 2] = platforms[type][i].pos.z;

		platformVert[type][i * 36 * pat + 16 * pat + 3] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 16 * pat + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 16 * pat + 5] = 1.0f;
		platformVert[type][i * 36 * pat + 16 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 16 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 16 * pat + 8] = platforms[type][i].dim.z;
		platformVert[type][i * 36 * pat + 16 * pat + 9] = 0.0f;
		platformVert[type][i * 36 * pat + 16 * pat + 10] = 0.0f;
		platformVert[type][i * 36 * pat + 16 * pat + 11] = platforms[type][i].dim.y;
		//
		platformVert[type][i * 36 * pat + 17 * pat + 0] = platformVert[type][i * 36 * pat + 13 * pat + 0];
		platformVert[type][i * 36 * pat + 17 * pat + 1] = platformVert[type][i * 36 * pat + 13 * pat + 1];
		platformVert[type][i * 36 * pat + 17 * pat + 2] = platformVert[type][i * 36 * pat + 13 * pat + 2];

		platformVert[type][i * 36 * pat + 17 * pat + 3] = platformVert[type][i * 36 * pat + 13 * pat + 3];
		platformVert[type][i * 36 * pat + 17 * pat + 4] = platformVert[type][i * 36 * pat + 13 * pat + 4];

		platformVert[type][i * 36 * pat + 17 * pat + 5] = platformVert[type][i * 36 * pat + 13 * pat + 5];
		platformVert[type][i * 36 * pat + 17 * pat + 6] = platformVert[type][i * 36 * pat + 13 * pat + 6];
		platformVert[type][i * 36 * pat + 17 * pat + 7] = platformVert[type][i * 36 * pat + 13 * pat + 7];

		platformVert[type][i * 36 * pat + 17 * pat + 8] = platformVert[type][i * 36 * pat + 13 * pat + 8];
		platformVert[type][i * 36 * pat + 17 * pat + 9] = platformVert[type][i * 36 * pat + 13 * pat + 9];
		platformVert[type][i * 36 * pat + 17 * pat + 10] = platformVert[type][i * 36 * pat + 13 * pat + 10];
		platformVert[type][i * 36 * pat + 17 * pat + 11] = platformVert[type][i * 36 * pat + 13 * pat + 11];

		// kierunek ( 0,  0, -1)
		// trojkat 1
		platformVert[type][i * 36 * pat + 18 * pat + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 18 * pat + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * pat + 18 * pat + 2] = platforms[type][i].pos.z;

		platformVert[type][i * 36 * pat + 18 * pat + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 18 * pat + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 18 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 18 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 18 * pat + 7] = -1.0f;

		platformVert[type][i * 36 * pat + 18 * pat + 8] = 0.0f;
		platformVert[type][i * 36 * pat + 18 * pat + 9] = 0.0f;
		platformVert[type][i * 36 * pat + 18 * pat + 10] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 18 * pat + 11] = platforms[type][i].dim.y;
		//
		platformVert[type][i * 36 * pat + 19 * pat + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * pat + 19 * pat + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 19 * pat + 2] = platforms[type][i].pos.z;

		platformVert[type][i * 36 * pat + 19 * pat + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 19 * pat + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 19 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 19 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 19 * pat + 7] = -1.0f;

		platformVert[type][i * 36 * pat + 19 * pat + 8] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 19 * pat + 9] = platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 19 * pat + 10] = 0.0f;
		platformVert[type][i * 36 * pat + 19 * pat + 11] = 0.0f;
		//
		platformVert[type][i * 36 * pat + 20 * pat + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 20 * pat + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 20 * pat + 2] = platforms[type][i].pos.z;

		platformVert[type][i * 36 * pat + 20 * pat + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 20 * pat + 4] = (platforms[type][i].pos.y + platforms[type][i].dim.y) * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 20 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 20 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 20 * pat + 7] = -1.0f;

		platformVert[type][i * 36 * pat + 20 * pat + 8] = 0.0f;
		platformVert[type][i * 36 * pat + 20 * pat + 9] = platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 20 * pat + 10] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 20 * pat + 11] = 0.0f;

		// trojkat 2
		platformVert[type][i * 36 * pat + 21 * pat + 0] = platformVert[type][i * 36 * pat + 18 * pat + 0];
		platformVert[type][i * 36 * pat + 21 * pat + 1] = platformVert[type][i * 36 * pat + 18 * pat + 1];
		platformVert[type][i * 36 * pat + 21 * pat + 2] = platformVert[type][i * 36 * pat + 18 * pat + 2];

		platformVert[type][i * 36 * pat + 21 * pat + 3] = platformVert[type][i * 36 * pat + 18 * pat + 3];
		platformVert[type][i * 36 * pat + 21 * pat + 4] = platformVert[type][i * 36 * pat + 18 * pat + 4];

		platformVert[type][i * 36 * pat + 21 * pat + 5] = platformVert[type][i * 36 * pat + 18 * pat + 5];
		platformVert[type][i * 36 * pat + 21 * pat + 6] = platformVert[type][i * 36 * pat + 18 * pat + 6];
		platformVert[type][i * 36 * pat + 21 * pat + 7] = platformVert[type][i * 36 * pat + 18 * pat + 7];

		platformVert[type][i * 36 * pat + 21 * pat + 8] = platformVert[type][i * 36 * pat + 18 * pat + 8];
		platformVert[type][i * 36 * pat + 21 * pat + 9] = platformVert[type][i * 36 * pat + 18 * pat + 9];
		platformVert[type][i * 36 * pat + 21 * pat + 10] = platformVert[type][i * 36 * pat + 18 * pat + 10];
		platformVert[type][i * 36 * pat + 21 * pat + 11] = platformVert[type][i * 36 * pat + 18 * pat + 11];
		//
		platformVert[type][i * 36 * pat + 22 * pat + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * pat + 22 * pat + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * pat + 22 * pat + 2] = platforms[type][i].pos.z;

		platformVert[type][i * 36 * pat + 22 * pat + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 22 * pat + 4] = platforms[type][i].pos.y * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 22 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 22 * pat + 6] = 0.0f;
		platformVert[type][i * 36 * pat + 22 * pat + 7] = -1.0f;

		platformVert[type][i * 36 * pat + 22 * pat + 8] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 22 * pat + 9] = 0.0f;
		platformVert[type][i * 36 * pat + 22 * pat + 10] = 0.0f;
		platformVert[type][i * 36 * pat + 22 * pat + 11] = platforms[type][i].dim.y;
		//
		platformVert[type][i * 36 * pat + 23 * pat + 0] = platformVert[type][i * 36 * pat + 19 * pat + 0];
		platformVert[type][i * 36 * pat + 23 * pat + 1] = platformVert[type][i * 36 * pat + 19 * pat + 1];
		platformVert[type][i * 36 * pat + 23 * pat + 2] = platformVert[type][i * 36 * pat + 19 * pat + 2];

		platformVert[type][i * 36 * pat + 23 * pat + 3] = platformVert[type][i * 36 * pat + 19 * pat + 3];
		platformVert[type][i * 36 * pat + 23 * pat + 4] = platformVert[type][i * 36 * pat + 19 * pat + 4];

		platformVert[type][i * 36 * pat + 23 * pat + 5] = platformVert[type][i * 36 * pat + 19 * pat + 5];
		platformVert[type][i * 36 * pat + 23 * pat + 6] = platformVert[type][i * 36 * pat + 19 * pat + 6];
		platformVert[type][i * 36 * pat + 23 * pat + 7] = platformVert[type][i * 36 * pat + 19 * pat + 7];

		platformVert[type][i * 36 * pat + 23 * pat + 8] = platformVert[type][i * 36 * pat + 19 * pat + 8];
		platformVert[type][i * 36 * pat + 23 * pat + 9] = platformVert[type][i * 36 * pat + 19 * pat + 9];
		platformVert[type][i * 36 * pat + 23 * pat + 10] = platformVert[type][i * 36 * pat + 19 * pat + 10];
		platformVert[type][i * 36 * pat + 23 * pat + 11] = platformVert[type][i * 36 * pat + 19 * pat + 11];

		// kierunek ( 0, -1,  0)
		// trojkat 1
		platformVert[type][i * 36 * pat + 24 * pat + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * pat + 24 * pat + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * pat + 24 * pat + 2] = platforms[type][i].pos.z;

		platformVert[type][i * 36 * pat + 24 * pat + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 24 * pat + 4] = platforms[type][i].pos.z * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 24 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 24 * pat + 6] = -1.0f;
		platformVert[type][i * 36 * pat + 24 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 24 * pat + 8] = 0.0f;
		platformVert[type][i * 36 * pat + 24 * pat + 9] = 0.0f;
		platformVert[type][i * 36 * pat + 24 * pat + 10] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 24 * pat + 11] = platforms[type][i].dim.z;
		//
		platformVert[type][i * 36 * pat + 25 * pat + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 25 * pat + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * pat + 25 * pat + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;

		platformVert[type][i * 36 * pat + 25 * pat + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 25 * pat + 4] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 25 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 25 * pat + 6] = -1.0f;
		platformVert[type][i * 36 * pat + 25 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 25 * pat + 8] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 25 * pat + 9] = platforms[type][i].dim.z;
		platformVert[type][i * 36 * pat + 25 * pat + 10] = 0.0f;
		platformVert[type][i * 36 * pat + 25 * pat + 11] = 0.0f;
		//
		platformVert[type][i * 36 * pat + 26 * pat + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * pat + 26 * pat + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * pat + 26 * pat + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;

		platformVert[type][i * 36 * pat + 26 * pat + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 26 * pat + 4] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 26 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 26 * pat + 6] = -1.0f;
		platformVert[type][i * 36 * pat + 26 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 26 * pat + 8] = 0.0f;
		platformVert[type][i * 36 * pat + 26 * pat + 9] = platforms[type][i].dim.z;
		platformVert[type][i * 36 * pat + 26 * pat + 10] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 26 * pat + 11] = 0.0f;

		// trojkat 2
		platformVert[type][i * 36 * pat + 27 * pat + 0] = platformVert[type][i * 36 * pat + 24 * pat + 0];
		platformVert[type][i * 36 * pat + 27 * pat + 1] = platformVert[type][i * 36 * pat + 24 * pat + 1];
		platformVert[type][i * 36 * pat + 27 * pat + 2] = platformVert[type][i * 36 * pat + 24 * pat + 2];

		platformVert[type][i * 36 * pat + 27 * pat + 3] = platformVert[type][i * 36 * pat + 24 * pat + 3];
		platformVert[type][i * 36 * pat + 27 * pat + 4] = platformVert[type][i * 36 * pat + 24 * pat + 4];

		platformVert[type][i * 36 * pat + 27 * pat + 5] = platformVert[type][i * 36 * pat + 24 * pat + 5];
		platformVert[type][i * 36 * pat + 27 * pat + 6] = platformVert[type][i * 36 * pat + 24 * pat + 6];
		platformVert[type][i * 36 * pat + 27 * pat + 7] = platformVert[type][i * 36 * pat + 24 * pat + 7];

		platformVert[type][i * 36 * pat + 27 * pat + 8] = platformVert[type][i * 36 * pat + 24 * pat + 8];
		platformVert[type][i * 36 * pat + 27 * pat + 9] = platformVert[type][i * 36 * pat + 24 * pat + 9];
		platformVert[type][i * 36 * pat + 27 * pat + 10] = platformVert[type][i * 36 * pat + 24 * pat + 10];
		platformVert[type][i * 36 * pat + 27 * pat + 11] = platformVert[type][i * 36 * pat + 24 * pat + 11];
		//
		platformVert[type][i * 36 * pat + 28 * pat + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 28 * pat + 1] = platforms[type][i].pos.y;
		platformVert[type][i * 36 * pat + 28 * pat + 2] = platforms[type][i].pos.z;

		platformVert[type][i * 36 * pat + 28 * pat + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 28 * pat + 4] = platforms[type][i].pos.z * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 28 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 28 * pat + 6] = -1.0f;
		platformVert[type][i * 36 * pat + 28 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 28 * pat + 8] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 28 * pat + 9] = 0.0f;
		platformVert[type][i * 36 * pat + 28 * pat + 10] = 0.0f;
		platformVert[type][i * 36 * pat + 28 * pat + 11] = platforms[type][i].dim.z;
		//
		platformVert[type][i * 36 * pat + 29 * pat + 0] = platformVert[type][i * 36 * pat + 25 * pat + 0];
		platformVert[type][i * 36 * pat + 29 * pat + 1] = platformVert[type][i * 36 * pat + 25 * pat + 1];
		platformVert[type][i * 36 * pat + 29 * pat + 2] = platformVert[type][i * 36 * pat + 25 * pat + 2];

		platformVert[type][i * 36 * pat + 29 * pat + 3] = platformVert[type][i * 36 * pat + 25 * pat + 3];
		platformVert[type][i * 36 * pat + 29 * pat + 4] = platformVert[type][i * 36 * pat + 25 * pat + 4];

		platformVert[type][i * 36 * pat + 29 * pat + 5] = platformVert[type][i * 36 * pat + 25 * pat + 5];
		platformVert[type][i * 36 * pat + 29 * pat + 6] = platformVert[type][i * 36 * pat + 25 * pat + 6];
		platformVert[type][i * 36 * pat + 29 * pat + 7] = platformVert[type][i * 36 * pat + 25 * pat + 7];

		platformVert[type][i * 36 * pat + 29 * pat + 8] = platformVert[type][i * 36 * pat + 25 * pat + 8];
		platformVert[type][i * 36 * pat + 29 * pat + 9] = platformVert[type][i * 36 * pat + 25 * pat + 9];
		platformVert[type][i * 36 * pat + 29 * pat + 10] = platformVert[type][i * 36 * pat + 25 * pat + 10];
		platformVert[type][i * 36 * pat + 29 * pat + 11] = platformVert[type][i * 36 * pat + 25 * pat + 11];

		// kierunek ( 0,  1,  0)
		// trojkat 1
		platformVert[type][i * 36 * pat + 30 * pat + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * pat + 30 * pat + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 30 * pat + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;

		platformVert[type][i * 36 * pat + 30 * pat + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 30 * pat + 4] = platforms[type][i].pos.z * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 30 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 30 * pat + 6] = 1.0f;
		platformVert[type][i * 36 * pat + 30 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 30 * pat + 8] = 0.0f;
		platformVert[type][i * 36 * pat + 30 * pat + 9] = 0.0f;
		platformVert[type][i * 36 * pat + 30 * pat + 10] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 30 * pat + 11] = platforms[type][i].dim.z;
		//
		platformVert[type][i * 36 * pat + 31 * pat + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 31 * pat + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 31 * pat + 2] = platforms[type][i].pos.z;

		platformVert[type][i * 36 * pat + 31 * pat + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 31 * pat + 4] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 31 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 31 * pat + 6] = 1.0f;
		platformVert[type][i * 36 * pat + 31 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 31 * pat + 8] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 31 * pat + 9] = platforms[type][i].dim.z;
		platformVert[type][i * 36 * pat + 31 * pat + 10] = 0.0f;
		platformVert[type][i * 36 * pat + 31 * pat + 11] = 0.0f;
		//
		platformVert[type][i * 36 * pat + 32 * pat + 0] = platforms[type][i].pos.x;
		platformVert[type][i * 36 * pat + 32 * pat + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 32 * pat + 2] = platforms[type][i].pos.z;

		platformVert[type][i * 36 * pat + 32 * pat + 3] = platforms[type][i].pos.x * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 32 * pat + 4] = (platforms[type][i].pos.z + platforms[type][i].dim.z) * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 32 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 32 * pat + 6] = 1.0f;
		platformVert[type][i * 36 * pat + 32 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 32 * pat + 8] = 0.0f;
		platformVert[type][i * 36 * pat + 32 * pat + 9] = platforms[type][i].dim.z;
		platformVert[type][i * 36 * pat + 32 * pat + 10] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 32 * pat + 11] = 0.0f;

		// trojkat 2
		platformVert[type][i * 36 * pat + 33 * pat + 0] = platformVert[type][i * 36 * pat + 30 * pat + 0];
		platformVert[type][i * 36 * pat + 33 * pat + 1] = platformVert[type][i * 36 * pat + 30 * pat + 1];
		platformVert[type][i * 36 * pat + 33 * pat + 2] = platformVert[type][i * 36 * pat + 30 * pat + 2];

		platformVert[type][i * 36 * pat + 33 * pat + 3] = platformVert[type][i * 36 * pat + 30 * pat + 3];
		platformVert[type][i * 36 * pat + 33 * pat + 4] = platformVert[type][i * 36 * pat + 30 * pat + 4];

		platformVert[type][i * 36 * pat + 33 * pat + 5] = platformVert[type][i * 36 * pat + 30 * pat + 5];
		platformVert[type][i * 36 * pat + 33 * pat + 6] = platformVert[type][i * 36 * pat + 30 * pat + 6];
		platformVert[type][i * 36 * pat + 33 * pat + 7] = platformVert[type][i * 36 * pat + 30 * pat + 7];

		platformVert[type][i * 36 * pat + 33 * pat + 8] = platformVert[type][i * 36 * pat + 30 * pat + 8];
		platformVert[type][i * 36 * pat + 33 * pat + 9] = platformVert[type][i * 36 * pat + 30 * pat + 9];
		platformVert[type][i * 36 * pat + 33 * pat + 10] = platformVert[type][i * 36 * pat + 30 * pat + 10];
		platformVert[type][i * 36 * pat + 33 * pat + 11] = platformVert[type][i * 36 * pat + 30 * pat + 11];
		//
		platformVert[type][i * 36 * pat + 34 * pat + 0] = platforms[type][i].pos.x + platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 34 * pat + 1] = platforms[type][i].pos.y + platforms[type][i].dim.y;
		platformVert[type][i * 36 * pat + 34 * pat + 2] = platforms[type][i].pos.z + platforms[type][i].dim.z;

		platformVert[type][i * 36 * pat + 34 * pat + 3] = (platforms[type][i].pos.x + platforms[type][i].dim.x) * cnst::platformTextureScale;
		platformVert[type][i * 36 * pat + 34 * pat + 4] = platforms[type][i].pos.z * cnst::platformTextureScale;

		platformVert[type][i * 36 * pat + 34 * pat + 5] = 0.0f;
		platformVert[type][i * 36 * pat + 34 * pat + 6] = 1.0f;
		platformVert[type][i * 36 * pat + 34 * pat + 7] = 0.0f;

		platformVert[type][i * 36 * pat + 34 * pat + 8] = platforms[type][i].dim.x;
		platformVert[type][i * 36 * pat + 34 * pat + 9] = 0.0f;
		platformVert[type][i * 36 * pat + 34 * pat + 10] = 0.0f;
		platformVert[type][i * 36 * pat + 34 * pat + 11] = platforms[type][i].dim.z;
		//
		platformVert[type][i * 36 * pat + 35 * pat + 0] = platformVert[type][i * 36 * pat + 31 * pat + 0];
		platformVert[type][i * 36 * pat + 35 * pat + 1] = platformVert[type][i * 36 * pat + 31 * pat + 1];
		platformVert[type][i * 36 * pat + 35 * pat + 2] = platformVert[type][i * 36 * pat + 31 * pat + 2];

		platformVert[type][i * 36 * pat + 35 * pat + 3] = platformVert[type][i * 36 * pat + 31 * pat + 3];
		platformVert[type][i * 36 * pat + 35 * pat + 4] = platformVert[type][i * 36 * pat + 31 * pat + 4];

		platformVert[type][i * 36 * pat + 35 * pat + 5] = platformVert[type][i * 36 * pat + 31 * pat + 5];
		platformVert[type][i * 36 * pat + 35 * pat + 6] = platformVert[type][i * 36 * pat + 31 * pat + 6];
		platformVert[type][i * 36 * pat + 35 * pat + 7] = platformVert[type][i * 36 * pat + 31 * pat + 7];

		platformVert[type][i * 36 * pat + 35 * pat + 8] = platformVert[type][i * 36 * pat + 31 * pat + 8];
		platformVert[type][i * 36 * pat + 35 * pat + 9] = platformVert[type][i * 36 * pat + 31 * pat + 9];
		platformVert[type][i * 36 * pat + 35 * pat + 10] = platformVert[type][i * 36 * pat + 31 * pat + 10];
		platformVert[type][i * 36 * pat + 35 * pat + 11] = platformVert[type][i * 36 * pat + 31 * pat + 11];
	}

	// robienie EBO do tego chyba bedzie zbyt skomplikowane, ale nie trzeba
	// bo zrobilismy mozliwosc bindowania VAO VN albo VTN bez indices
	util::bindVAO_Platform(&platformVAO[type], &platformVBO[type], nullptr, platformVert[type], platformVertSize[type], nullptr, 0);
}
