#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>

// Create a NULL-terminated string by reading the provided file
char* readShaderSource(const char* shaderFile);

// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char* vShaderFile, const char* fShaderFile, const char* outputAttributeName, const char* outputAttributeName2 = 0);

#endif