#include "Mesh.h"
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <iostream>

static bool readTriFile(char const* tri_path, std::vector<GLfloat>* const tri_vector, float* const radius);
static bool readTriLine(FILE* f, std::vector<GLfloat>* const tri_vector, float* const radius);

Mesh loadMeshFromFile(char const* tri_path) {
  Mesh mesh;

  // Make the model's GL state active
  glBindVertexArray(mesh.vao);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

  // get our file and parse it into our vector of GLfloats
  std::vector<GLfloat> tri_vector;
  float radius = 0.0f;
  readTriFile(tri_path, &tri_vector, &radius);

  // Upload the model to GPU memory
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*tri_vector.size(), tri_vector.data(), GL_STATIC_DRAW);

  mesh.primitiveType = GL_TRIANGLES;
  mesh.primitiveCount = (GLsizei)(tri_vector.size()/7);
  mesh.boundingRadius = radius;

  // std::cout << tri_path << " - " << radius << std::endl;

  // Set attribute slot 0 to read the first 3 floats out of every set of 10 floats in the model.
  // In other words, slot 0 refers to the position data.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10*sizeof(GLfloat), (GLvoid*)(0*sizeof(GLfloat)));
  glEnableVertexAttribArray(0);

  // Slot 1 refers to the normal data.
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  // Slot 2 refers to the color data.
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10*sizeof(GLfloat), (GLvoid*)(6*sizeof(GLfloat)));
  glEnableVertexAttribArray(2);

  // As a precaution, unbind the vertex array so that GL operations don't accidentally operate on it.
  // The state and vertex information still exists - we can just bind model.vao to make it active again.
  glBindVertexArray(GL_NONE);

  return mesh;
}

// load a .TRI file into a vector of GLFloats using the provided file path and vector reference
// returns true if successful, false otherwise
static bool readTriFile(char const* tri_path, std::vector<GLfloat>* const tri_vector, float* const radius) {
  FILE* f = fopen(tri_path, "r");
  if (!f) {
    fprintf(stderr, "Unable to open file '%s'.\n", tri_path);
    return false;
  }

  while (!feof(f)) {
    // parse current line into our array
    bool success = readTriLine(f, tri_vector, radius);
    if (!success) {
      break;
    }
  }

  fclose(f);
  return true;
}

// Parses a line from a .TRI file and pushes vertex position and color data into vector
// takes file pointer and reference to vector of GLfloats
static bool readTriLine(FILE* f, std::vector<GLfloat>* const tri_vector, float* const radius) {
  // our vertex position data
  float p1x, p1y, p1z;
  float p2x, p2y, p2z;
  float p3x, p3y, p3z;

  // our vertex color data
  unsigned int rgb;

  // parse the line of elements into local vars
  fscanf(f, "%f %f %f %f %f %f %f %f %f 0x%x",
      &p1x, &p1y, &p1z,
      &p2x, &p2y, &p2z,
      &p3x, &p3y, &p3z,
      &rgb
  );

  // check for file read errors
  if (feof(f) || ferror(f)) {
    return false; // malformed line, oops!
  }

  // bit shifting with masking to separate out RGB color data
  glm::vec4 const color{
      (float)((rgb >> 16) & 0xFF) / 0xFF,  // Red
      (float)((rgb >>  8) & 0xFF) / 0xFF,  // Green
      (float)((rgb >>  0) & 0xFF) / 0xFF,  // Blue
      1.0f,                                // Alpha
  };

  glm::vec3 const p1{p1x, p1y, p1z};
  glm::vec3 const p2{p2x, p2y, p2z};
  glm::vec3 const p3{p3x, p3y, p3z};
  glm::vec3 const normal = glm::normalize(glm::cross(p2 - p1, p3 - p1));

  // push our data into the vector of GLfloats
  tri_vector->push_back(p1.x);
  tri_vector->push_back(p1.y);
  tri_vector->push_back(p1.z);
  tri_vector->push_back(normal.x);
  tri_vector->push_back(normal.y);
  tri_vector->push_back(normal.z);
  tri_vector->push_back(color.r);
  tri_vector->push_back(color.g);
  tri_vector->push_back(color.b);
  tri_vector->push_back(color.a);

  tri_vector->push_back(p2.x);
  tri_vector->push_back(p2.y);
  tri_vector->push_back(p2.z);
  tri_vector->push_back(normal.x);
  tri_vector->push_back(normal.y);
  tri_vector->push_back(normal.z);
  tri_vector->push_back(color.r);
  tri_vector->push_back(color.g);
  tri_vector->push_back(color.b);
  tri_vector->push_back(color.a);

  tri_vector->push_back(p3.x);
  tri_vector->push_back(p3.y);
  tri_vector->push_back(p3.z);
  tri_vector->push_back(normal.x);
  tri_vector->push_back(normal.y);
  tri_vector->push_back(normal.z);
  tri_vector->push_back(color.r);
  tri_vector->push_back(color.g);
  tri_vector->push_back(color.b);
  tri_vector->push_back(color.a);

  *radius = fmax(fmax(glm::length(p1), glm::length(p2)), fmax(glm::length(p3), *radius));

  return true;
}
