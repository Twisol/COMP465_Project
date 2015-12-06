#pragma once

#include <GL/glew.h>

void freeTexture(GLuint texture);

GLuint loadRawTexture(GLuint texture, const char* filename, int width, int height);

unsigned char* loadRawData(const char* filename, int width, int height);

GLuint makeCubeMap(unsigned char* texData[6], int edge);
