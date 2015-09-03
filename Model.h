#pragma once

#include "shared.h"

struct Model {
  GLuint vbo = GL_NONE;  // References vertex attribute information loaded onto the GPU
  GLuint vao = GL_NONE;  // Describes the format and intent of the vertex attribute information

  Model() {
    // Create a GPU memory handle
    // This allows you to allocate and store things in GPU memory.
    // Initially, there is no memory associated with this handle.
    glGenBuffers(1, &this->vbo);

    // Create a vertex array object (VAO).
    // This captures information about which VBOs to look at for which vertex attributes,
    // and where within each VBO each attribute can be found.
    // Binding a VAO makes all of this information immediately active in the GL state machine,
    // making rendering much simpler.
    glGenVertexArrays(1, &this->vao);
  }

  ~Model() {
    glDeleteVertexArrays(1, &this->vao);
    glDeleteBuffers(1, &this->vbo);
  }

  /* Implement move semantics for this type. */
  Model(Model&& other) {
    this->vbo = other.vbo;
    other.vbo = GL_NONE;

    this->vao = other.vao;
    other.vao = GL_NONE;
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

  virtual void BindVertexData() const {
    glBindVertexArray(this->vao);
  }
};


Model loadTriangleModel();
