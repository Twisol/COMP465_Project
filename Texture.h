#pragma once

#include <GL/glew.h>


/* Based on example code developed by Mike Barnes (11/5/2013)

freeTexture() deletes texture resources

loadRawTexture() expects three bytes per pixel (24bpp).
The program loads the texture top-left (1,0) to bottom-right(1,0).
Your image may appear upside down depending on how you saved the *.raw
file.  You might need to change the texture coordinates. */

void freeTexture(GLuint texture) {
  glDeleteTextures(1, &texture);
}

GLuint loadRawTexture(GLuint texture, const char* filename, int width, int height) {
  unsigned char* data;
  FILE* file;
  int readResult;

  // open texture data
  file = fopen(filename, "rb");
  if (file == NULL) {
     printf("File %s can't be opened\n", filename);
     return 0;
  }
  // allocate buffer -- 3 bytes / texel -- rgb
  data = (unsigned char*) malloc(width * height * 3);
  // read texture data
  readResult = fread(data, width * height * 3, 1, file);
  if (readResult != 1) {
     printf("File %s was not read correctly\n", filename);
     return 0;
  }
  fclose(file);

  glGenTextures(1, &texture); // generate the texture with the loaded data
  glBindTexture(GL_TEXTURE_2D, texture); // bind the texture
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // set texture environment parameters
  // set texture parameters
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  // generate the texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  free(data); // free the texture
  return texture; // return whether it was successful
}

unsigned char* loadRawData(const char* filename, int width, int height) {
  // loads .RAW image data from specified filename into application memory
  // and returns pointer
  unsigned char* data = NULL;
  FILE* file;
  int readResult;

  file = fopen(filename, "rb");
  if (file==NULL) {
    printf("file %s ca't be opened!\n", filename);
    data = NULL;
  }

  data = (unsigned char*) malloc(width * height * 3);
  readResult = fread(data, width * height * 3, 1, file);
  if (readResult !=1) {
    printf("File %s was not read correctly!\n", filename);
    data = NULL;
  }
  fclose(file);
  if (data != NULL) {
    printf("File %s was loaded!\n", filename);
  }
  return data;
}


GLuint makeCubeMap(GLuint texture, unsigned char* texData[6], int edge) {
  // creates cube map texture object and initializes it with image data in
  // passed-in array; edge is number of pixels in each edge of cube

  // set cube map texture parameters
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture); // bind the texture
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

  // place texture data into cube map's texel array
  for (int face = 0; face < 6; face++) {
    GLenum target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
    glTexSubImage2D(target,           // face
                    0,                // level
                    0, 0,             // X, Y offset
                    edge, edge,       // size of face
                    GL_RGB,           // format
                    GL_UNSIGNED_BYTE, // type
                    texData[face]);   // image data
  }
  // TODO: Implement cube texture deletion?
  return texture;
}