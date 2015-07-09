#pragma once
#include "GL/freeglut.h"

typedef struct{
	GLuint width;
	GLuint height;
	GLuint frameBuffer;
	GLuint colorTexture;
	GLuint depthTexture;
}frameBuffer;