#pragma once

#include <glm/glm.hpp>

namespace cnst
{
	extern float minDrawTime; // trzeba bedzie powoli przenosic te rozne wartosci ze stalych do ustawien

	const unsigned int defaultShadowWidth = 4096;
	const unsigned int defaultShadowHeight = 4096;
	const float near = 0.1f;
	const float far = 1000.0f;
	const unsigned int cascadeCount = 6;
	const float cascadePortions[cascadeCount - 1] = { 0.02f, 0.05f, 0.1f, 0.2f, 0.4f };

	const unsigned int depthMapSlotStart = 7;

	const unsigned int baseTextureSlot = 0;
	const unsigned int specularMapSlot = 1;
	const unsigned int normalMapSlot = 2;
	const unsigned int diffuseLutSlot = 3;

	const unsigned int skyboxGradientSlot = 4;
	const unsigned int sunLutSlot = 5;

	const unsigned int interfaceAtlasSlot = 6;

	const unsigned int jitterSampleDim = 4;
	const unsigned int jitterSampleCount = jitterSampleDim * jitterSampleDim;

	const char interfacePath[] = "resources/levels/interface.png";
	const char cylinderPath[] = "resources/levels/cylinder.obj";

	const char skyboxGradientPath[] = "resources/textures/level0_g.png";
	const unsigned char sunLutTexture[] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 8,
		8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 17, 17,
		17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 30, 30, 31, 32, 32,
		33, 34, 34, 35, 36, 37, 37, 38, 39, 40, 41, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 55, 56, 57, 58, 59,
		61, 62, 63, 65, 66, 68, 69, 71, 72, 74, 76, 78, 79, 81, 83, 85, 87, 89, 92, 94, 96, 99, 101, 104, 107, 110, 112, 115,
		119, 122, 125, 129, 133, 136, 140, 144, 148, 152, 156, 160, 164, 168, 172, 175, 179, 183, 186, 189, 192, 195, 199,
		201, 204, 206, 209, 211, 214, 216, 218, 220, 222, 224, 225, 227, 229, 230, 232, 233, 234, 236, 237, 238, 240, 241,
		242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 252, 253, 254, 255
	};

	const glm::vec3 playerColor(0.863f, 0.078f, 0.235f);
	const glm::vec3 clearColor(0.3f, 0.3f, 0.5f);

	const float platformTextureScale = 0.25f;
	const unsigned int platformAttribCount = 4;
	const unsigned int platformAttribSizes[platformAttribCount] = { 3, 2, 3, 4 };
	const float pi = 3.1415927f;

	const unsigned int diffuseLutSampleCount = 256;
	const unsigned char diffuseLutTexture[] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
		31, 32, 33, 35, 36, 37, 38, 39, 41, 42, 43, 44, 45, 47, 48, 49, 51, 52, 53, 55, 56, 57, 59, 60, 62, 63, 65, 66, 68,
		69, 71, 73, 74, 76, 77, 79, 81, 83, 85, 86, 88, 90, 92, 94, 96, 98, 101, 103, 105, 107, 109, 112, 114, 116, 118, 121,
		123, 125, 128, 130, 133, 135, 137, 140, 142, 144, 147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167, 168, 170,
		172, 173, 175, 176, 178, 179, 181, 182, 183, 185, 186, 187, 188, 189, 191, 192, 193, 194, 195, 196, 197, 198, 199,
		200, 201, 201, 203, 203, 204, 205, 206, 207, 207, 208, 209, 210, 210, 211, 212, 213, 213, 214, 215, 215, 216, 217,
		217, 218, 218, 219, 220, 220, 221, 221, 222, 222, 223, 224, 224, 224, 225, 226, 226, 227, 227, 228, 228, 229, 229,
		229, 230, 230, 231, 231, 232, 232, 233, 233, 233, 234, 234, 235, 235, 236, 236, 236, 237, 237, 237, 238, 238, 239,
		239, 239, 240, 240, 241, 241, 241, 242, 242, 242, 243, 243, 243, 244, 244, 244, 245, 245, 245, 245, 246, 246, 246,
		247, 247, 247, 248, 248, 248, 249, 249, 249, 249, 250, 250, 250, 251, 251, 251, 252, 252, 252, 252, 252, 253, 253,
		253, 254, 254, 254, 254, 255, 255
	};

	const float skyboxVertices[] =
	{
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
}
