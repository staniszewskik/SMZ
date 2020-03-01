#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include "stb_image_write.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <new>
#include <iostream>

#include "constantsRender.h"
#include "utility.h"

struct texturedKey
{
	unsigned int vertInd;
	unsigned int textInd;
	unsigned int normInd;
};

struct texturedValue
{
	glm::vec3 vertVal;
	glm::vec2 textVal;
	glm::vec3 normVal;
	unsigned int indInd;
};

bool operator< (texturedKey a, texturedKey b)
{
	return std::make_pair(a.vertInd, std::make_pair(a.textInd, a.normInd)) <
		std::make_pair(b.vertInd, std::make_pair(b.textInd, b.normInd));
}

struct untexturedKey
{
	unsigned int vertInd;
	unsigned int normInd;
};

struct untexturedValue
{
	glm::vec3 vertVal;
	glm::vec3 normVal;
	unsigned int indInd;
};

bool operator< (untexturedKey a, untexturedKey b)
{
	return std::make_pair(a.vertInd, a.normInd) < std::make_pair(b.vertInd, b.normInd);
}

bool util::readObj(const char *path, float **vertices, unsigned int *vertSize, unsigned int **indices, unsigned int *indcSize)
{
	// zakladamy ze wszystkie v to trojkaty,
	// ze sa normale, opcjonalnie sa tez wspolrzedne tekstur
	// (czyli w indeksach moze byc <nr wierzcholka>//<nr normala>
	bool textured = false;
	std::ifstream in(path);
	std::string line = "";

	std::vector<glm::vec3> vert;
	std::vector<glm::vec2> text;
	std::vector<glm::vec3> norm;
	std::vector<unsigned int> indc;

	std::getline(in, line);
	while (line != "")
	{
		std::istringstream splitter(line);
		std::string lineType;
		splitter >> lineType;

		if (lineType == "v")
		{
			glm::vec3 readVert;
			splitter >> readVert.x >> readVert.y >> readVert.z;
			vert.push_back(readVert);
		}
		else if (lineType == "vt")
		{
			textured = true;
			glm::vec3 readText;
			splitter >> readText.s >> readText.t;
			text.push_back(readText);
		}
		else if (lineType == "vn")
		{
			glm::vec3 readNorm;
			splitter >> readNorm.x >> readNorm.y >> readNorm.z;
			norm.push_back(readNorm);
		}
		else if (lineType == "f")
			break;

		std::getline(in, line);
	}

	// UWAGA - indices w formacie .obj sa od 1 (i teoretycznie moga byc ujemne, ale ignoruje ten przypadek)
	// za to w EBO numeracja jest od 0

	if (textured)
	{
		unsigned int indInd = 0;
		unsigned int triCount = 0;
		std::vector<texturedValue> mergedVerts;
		std::map<texturedKey, unsigned int> indicesMap;
		std::vector<unsigned int> indicesVec;
		while (line != "")
		{
			texturedKey keyA, keyB, keyC;
			sscanf(line.c_str(), "f %u/%u/%u %u/%u/%u %u/%u/%u",
				&keyA.vertInd, &keyA.textInd, &keyA.normInd,
				&keyB.vertInd, &keyB.textInd, &keyB.normInd,
				&keyC.vertInd, &keyC.textInd, &keyC.normInd);

			texturedValue valA, valB, valC;
			valA.vertVal = vert[keyA.vertInd - 1]; valA.textVal = text[keyA.textInd - 1]; valA.normVal = norm[keyA.normInd - 1];
			valB.vertVal = vert[keyB.vertInd - 1]; valB.textVal = text[keyB.textInd - 1]; valB.normVal = norm[keyB.normInd - 1];
			valC.vertVal = vert[keyC.vertInd - 1]; valC.textVal = text[keyC.textInd - 1]; valC.normVal = norm[keyC.normInd - 1];

			if (indicesMap.count(keyA) == 0)
			{
				indicesMap[keyA] = indInd;
				valA.indInd = indInd++;
				mergedVerts.push_back(valA);
			}
			indicesVec.push_back(indicesMap[keyA]);
			if (indicesMap.count(keyB) == 0)
			{
				indicesMap[keyB] = indInd;
				valB.indInd = indInd++;
				mergedVerts.push_back(valB);
			}
			indicesVec.push_back(indicesMap[keyB]);
			if (indicesMap.count(keyC) == 0)
			{
				indicesMap[keyC] = indInd;
				valC.indInd = indInd++;
				mergedVerts.push_back(valC);
			}
			indicesVec.push_back(indicesMap[keyC]);

			triCount++;
			std::getline(in, line);
		}

		*vertSize = sizeof(float) * indicesMap.size() * 8;
		*vertices = new float[indicesMap.size() * 8];
		for (unsigned int v = 0; v < mergedVerts.size(); v++)
		{
			(*vertices)[v * 8 + 0] = mergedVerts[v].vertVal.x;
			(*vertices)[v * 8 + 1] = mergedVerts[v].vertVal.y;
			(*vertices)[v * 8 + 2] = mergedVerts[v].vertVal.z;

			(*vertices)[v * 8 + 3] = mergedVerts[v].textVal.s;
			(*vertices)[v * 8 + 4] = mergedVerts[v].textVal.t;

			(*vertices)[v * 8 + 5] = mergedVerts[v].normVal.x;
			(*vertices)[v * 8 + 6] = mergedVerts[v].normVal.y;
			(*vertices)[v * 8 + 7] = mergedVerts[v].normVal.z;
		}

		*indcSize = sizeof(unsigned int) * triCount * 3;
		*indices = new unsigned int[triCount * 3];
		for (unsigned int i = 0; i < triCount; i++)
		{
			(*indices)[i * 3 + 0] = indicesVec[i * 3 + 0];
			(*indices)[i * 3 + 1] = indicesVec[i * 3 + 1];
			(*indices)[i * 3 + 2] = indicesVec[i * 3 + 2];
		}
	}
	else
	{
		unsigned int indInd = 0;
		unsigned int triCount = 0;
		std::vector<untexturedValue> mergedVerts;
		std::map<untexturedKey, unsigned int> indicesMap;
		std::vector<unsigned int> indicesVec;
		while (line != "")
		{
			untexturedKey keyA, keyB, keyC;
			sscanf(line.c_str(), "f %u//%u %u//%u %u//%u",
				&keyA.vertInd, &keyA.normInd,
				&keyB.vertInd, &keyB.normInd,
				&keyC.vertInd, &keyC.normInd);

			untexturedValue valA, valB, valC;
			valA.vertVal = vert[keyA.vertInd - 1]; valA.normVal = norm[keyA.normInd - 1];
			valB.vertVal = vert[keyB.vertInd - 1]; valB.normVal = norm[keyB.normInd - 1];
			valC.vertVal = vert[keyC.vertInd - 1]; valC.normVal = norm[keyC.normInd - 1];

			if (indicesMap.count(keyA) == 0)
			{
				indicesMap[keyA] = indInd;
				valA.indInd = indInd++;
				mergedVerts.push_back(valA);
			}
			indicesVec.push_back(indicesMap[keyA]);
			if (indicesMap.count(keyB) == 0)
			{
				indicesMap[keyB] = indInd;
				valB.indInd = indInd++;
				mergedVerts.push_back(valB);
			}
			indicesVec.push_back(indicesMap[keyB]);
			if (indicesMap.count(keyC) == 0)
			{
				indicesMap[keyC] = indInd;
				valC.indInd = indInd++;
				mergedVerts.push_back(valC);
			}
			indicesVec.push_back(indicesMap[keyC]);

			triCount++;
			std::getline(in, line);
		}

		*vertSize = sizeof(float) * indicesMap.size() * 6;
		*vertices = new float[indicesMap.size() * 6];
		for (unsigned int v = 0; v < mergedVerts.size(); v++)
		{
			(*vertices)[v * 6 + 0] = mergedVerts[v].vertVal.x;
			(*vertices)[v * 6 + 1] = mergedVerts[v].vertVal.y;
			(*vertices)[v * 6 + 2] = mergedVerts[v].vertVal.z;

			(*vertices)[v * 6 + 3] = mergedVerts[v].normVal.x;
			(*vertices)[v * 6 + 4] = mergedVerts[v].normVal.y;
			(*vertices)[v * 6 + 5] = mergedVerts[v].normVal.z;
		}

		*indcSize = sizeof(unsigned int) * triCount * 3;
		*indices = new unsigned int[triCount * 3];
		for (unsigned int i = 0; i < triCount; i++)
		{
			(*indices)[i * 3 + 0] = indicesVec[i * 3 + 0];
			(*indices)[i * 3 + 1] = indicesVec[i * 3 + 1];
			(*indices)[i * 3 + 2] = indicesVec[i * 3 + 2];
		}
	}

	return textured;
}

void util::readLights(std::string name, bool level, glm::vec3* ambient, sunLight* sun)
{
	std::ifstream in("resources/lights/" + name + ".lightdat");
	std::string line = "";

	std::getline(in, line);
	while (line != "")
	{
		std::istringstream splitter(line);
		std::string lineType;
		splitter >> lineType;

		if (level && lineType == "amb")
			splitter >> ambient->r >> ambient->g >> ambient->b;
		else if (level && lineType == "sun")
			splitter >> sun->color.r >> sun->color.g >> sun->color.b >> sun->dir.x >> sun->dir.y >> sun->dir.z;

		std::getline(in, line);
	}
}

void util::bindVAO_VN(unsigned int *VAO, unsigned int *VBO, unsigned int *EBO,
	const float vertices[], unsigned int vertSize, const unsigned int indices[], unsigned int indcSize)
{
	glGenVertexArrays(1, VAO);
	glBindVertexArray(*VAO);

	glGenBuffers(1, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);

	if (EBO != nullptr && indices != nullptr)
	{
		glGenBuffers(1, EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indcSize, indices, GL_STATIC_DRAW);
	}

	unsigned int stride = 6 * sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void util::bindVAO_Platform(unsigned int *VAO, unsigned int *VBO, unsigned int *EBO,
	const float vertices[], unsigned int vertSize, const unsigned int indices[], unsigned int indcSize)
{
	glGenVertexArrays(1, VAO);
	glBindVertexArray(*VAO);

	glGenBuffers(1, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);

	if (EBO != nullptr && indices != nullptr)
	{
		glGenBuffers(1, EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indcSize, indices, GL_STATIC_DRAW);
	}

	unsigned int platformAttribTotal = 0;
	for (int i = 0; i < cnst::platformAttribCount; i++)
		platformAttribTotal += cnst::platformAttribSizes[i];
	unsigned int stride = platformAttribTotal * sizeof(float);
	platformAttribTotal = 0;
	for (int i = 0; i < cnst::platformAttribCount; i++)
	{
		glVertexAttribPointer(i, cnst::platformAttribSizes[i], GL_FLOAT, GL_FALSE,
			stride, (void*)(platformAttribTotal * sizeof(float)));
		glEnableVertexAttribArray(i);
		platformAttribTotal += cnst::platformAttribSizes[i];
	}
}

glm::mat4 util::lookAtPYR(glm::vec3 pos, glm::vec3 pyr)
{
	// wykonuje w kolejnosci roll, pitch, yaw
	// czyli reDir nie zalezy od roll, to dobrze dziala w kamerze FPP bez roll
	// a przy okazji roll na poczatku pozwala na latwe obracanie kamery overview wokol jej osi
	glm::mat4 mulRPY(1.0f);
	mulRPY = glm::rotate(mulRPY, pyr.y, glm::vec3(0.0f, 1.0f, 0.0f));
	mulRPY = glm::rotate(mulRPY, pyr.x, glm::vec3(1.0f, 0.0f, 0.0f));
	mulRPY = glm::rotate(mulRPY, pyr.z, glm::vec3(0.0f, 0.0f, 1.0f));

	glm::vec3 right = mulRPY * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 camUp = mulRPY * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	glm::vec3 reDir = mulRPY * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

	glm::mat4 lookCoordSys(1.0f); // domyslnie tworzy jednostkowa, indeksuje elementy [nr kolumny][nr wiersza]
	lookCoordSys[0][0] = right.x;	lookCoordSys[1][0] = right.y;	lookCoordSys[2][0] = right.z;
	lookCoordSys[0][1] = camUp.x;	lookCoordSys[1][1] = camUp.y;	lookCoordSys[2][1] = camUp.z;
	lookCoordSys[0][2] = reDir.x;	lookCoordSys[1][2] = reDir.y;	lookCoordSys[2][2] = reDir.z;

	glm::mat4 lookTranslation(1.0f);
	lookTranslation[3][0] = -pos.x;
	lookTranslation[3][1] = -pos.y;
	lookTranslation[3][2] = -pos.z;

	return lookCoordSys * lookTranslation;
}

void util::sampleBezierCurve(float a, float b, float c, float d, unsigned int sampleCount)
{
	// diffuseLut wygenerowane tym:
	//util::sampleBezierCurve(0.55f, 0.47f, 0.17f, 0.79f, cnst::diffuseLutSampleCount);

	// sunLut wygenerowane tym:
	//util::sampleBezierCurve(1.0f, 0.0f, 0.65f, 0.75f, cnst::diffuseLutSampleCount);

	float sampleSize = 1.0f / sampleCount;
	unsigned int stepCount = 4 * sampleCount;
	float stepSize = 1.0f / stepCount;

	std::cout << "const unsigned char lutTexture[] = { 0, ";
	float t = stepSize;
	float xValPrev = 0.0f;
	float yValPrev = 0.0f;
	for(unsigned int i = 1; i < sampleCount - 1;)
	{
		float xVal = (3 * a + ((3 * c - 6 * a) + (1 + 3 * a - 3 * c) * t) * t) * t;
		float yVal = (3 * b + ((3 * d - 6 * b) + (1 + 3 * b - 3 * d) * t) * t) * t;

		float currentTargetX = sampleSize * i + 0.5f * sampleSize;
		if (glm::sign(currentTargetX - xValPrev) != glm::sign(currentTargetX - xVal))
		{
			float yPick = glm::abs(currentTargetX - xValPrev) < glm::abs(currentTargetX - xVal) ? yValPrev : yVal;
			int yOutput = (int)glm::round(255.0f * yPick);
			std::cout << yOutput << ", ";
			i++;
		}
		else
		{
			t += stepSize;
			xValPrev = xVal;
			yValPrev = yVal;
		}
	}
	std::cout << "255 };" << std::endl;
}

unsigned int util::createLutTexture(unsigned int lutSampleCount, const unsigned char lutTextureData[])
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_1D, texture);

	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load and generate the texture
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R8, lutSampleCount, 0, GL_RED, GL_UNSIGNED_BYTE, lutTextureData);
	//glGenerateMipmap(GL_TEXTURE_1D);

	return texture;
}

std::string util::readFile(const char *fileName)
{
	std::ifstream ifs(fileName, std::ios::in | std::ios::binary | std::ios::ate);

	std::ifstream::pos_type fileSize = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	std::vector<char> bytes((const unsigned int)fileSize);
	ifs.read(bytes.data(), fileSize);

	return std::string(bytes.data(), (const unsigned int)fileSize);
}

unsigned int util::createShader(const char *path, GLenum shaderType)
{
	std::string shaderString = readFile(path);
	const char *const shaderCStr = shaderString.c_str();

	unsigned int shaderID;
	shaderID = glCreateShader(shaderType);
	glShaderSource(shaderID, 1, &shaderCStr, NULL);
	glCompileShader(shaderID);

	GLint isCompiled = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);

		std::cout << "Failed to compile \"" << path << "\", error log:" << std::endl;
		std::cout << errorLog.data();
	}

	return shaderID;
}

unsigned int util::buildShaderProgram(unsigned int vert, unsigned int frag)
{
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vert);
	glAttachShader(shaderProgram, frag);
	glLinkProgram(shaderProgram);

	return shaderProgram;
}

unsigned int util::getShaderProgramFromName(std::string name)
{
	unsigned int vertShader = util::createShader(("shaders/" + name + ".vertexshader").c_str(), GL_VERTEX_SHADER);
	unsigned int fragShader = util::createShader(("shaders/" + name + ".fragmentshader").c_str(), GL_FRAGMENT_SHADER);
	unsigned int progShader = util::buildShaderProgram(vertShader, fragShader);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return progShader;
}

unsigned int util::createBasicTexture(const char *path)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load and generate the texture
	int width, height, nrChannels;
	unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture:" << std::endl << path << std::endl;

	stbi_image_free(data);
	return texture;
}

unsigned int util::createCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int util::createSkyboxGradient()
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_1D, texture);

	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load and generate the texture
	int width, height, nrChannels;
	unsigned char *data = stbi_load(cnst::skyboxGradientPath, &width, &height, &nrChannels, 0);
	if (data && height == 1)
	{
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, width, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_1D);
	}
	else
		std::cout << "Failed to load skybox gradient texture" << std::endl;

	stbi_image_free(data);
	return texture;
}

void util::bindVAO_Cubemap(unsigned int *VAO, unsigned int *VBO)
{
	glGenVertexArrays(1, VAO);
	glBindVertexArray(*VAO);

	glGenBuffers(1, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cnst::skyboxVertices), cnst::skyboxVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

int util::initWindow(const char *title, GLFWwindow **window, unsigned int *width, unsigned int *height)
{
	/* Initialize the library */
	if (!glfwInit())
		return -1;

	// mode zawiera informacje o natywnej rozdzielczosci
	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	*width = mode->width; *height = mode->height;
	// podanie monitora zamiast NULL uruchamia okno w trybie pelnoekranowym
	*window = glfwCreateWindow(mode->width, mode->height, title, glfwGetPrimaryMonitor(), NULL);
	if (*window == nullptr)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(*window);
	if (glewInit() != GLEW_OK)
		return -1;

	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	std::cout << glGetString(GL_VERSION) << std::endl;
	stbi_set_flip_vertically_on_load(true);
	stbi_flip_vertically_on_write(true);

	return 0;
}

unsigned int util::createInterfaceTexture()
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// load and generate the texture
	int width, height, nrChannels;
	unsigned char *data = stbi_load(cnst::interfacePath, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load interface texture" << std::endl;

	stbi_image_free(data);
	return texture;
}

void util::bindVAO_Text(unsigned int *VAO, unsigned int *VBO, const float vertices[], unsigned int vertSize)
{
	glGenVertexArrays(1, VAO);
	glBindVertexArray(*VAO);

	glGenBuffers(1, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);

	unsigned int stride = 4 * sizeof(float);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
}
