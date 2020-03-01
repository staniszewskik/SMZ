#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

class Text
{
public:
	Text(unsigned int progText);
	~Text();

	void drawText(std::string text, unsigned int x, unsigned int y, unsigned int w, unsigned int h, bool wF);
private:
	unsigned int progTextShader;
	unsigned int interfaceTextureID;

	void copySquare(float *vert, unsigned int xTex, unsigned int yTex, unsigned int xTgt, unsigned int yTgt,
		unsigned int wTgt, unsigned int hTgt);
};
