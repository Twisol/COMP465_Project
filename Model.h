#pragma once

#include "shared.h"

struct Model {
  GLuint vbo = GL_NONE;  // References vertex attribute information loaded onto the GPU
  GLuint vao = GL_NONE;  // Describes the format and intent of the vertex attribute information

  Model() {
    // Create a GPU memory handle
    glGenBuffers(1, &this->vbo);

    // Create a vertex array object (VAO).
    // TODO: I don't really know what these are, but you have to have one.
    glGenVertexArrays(1, &this->vao);
  }

  Model(Model&& other) {
    this->vbo = other.vbo;
    this->vao = other.vao;
  }

  Model& operator=(Model&& other) {
    if (this == &other) {
      return *this;
    }

    glDeleteBuffers(1, &this->vbo);
    this->vbo = other.vbo;
    other.vbo = GL_NONE;

    glDeleteVertexArrays(1, &this->vao);
    this->vao = other.vao;
    other.vao = GL_NONE;

    return *this;
  }

  ~Model() {
    glDeleteVertexArrays(1, &this->vao);
    glDeleteBuffers(1, &this->vbo);
  }
};


Model loadTriangleModel();
