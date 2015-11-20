#pragma once

#include <GL/glew.h>
#include <vector>

struct Mesh {
  GLuint vbo = GL_NONE;  // References vertex attribute information loaded onto the GPU
  GLuint vao = GL_NONE;  // Describes the format and intent of the vertex attribute information
  GLenum primitiveType = GL_TRIANGLES;  // Describe the mapping between vertices and primitives
  GLsizei primitiveCount = 0;  // The number of primitives in the mesh

  float boundingRadius = 1;  // The radius of a sphere bounding the mesh

  Mesh() {
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

  ~Mesh() {
    glDeleteVertexArrays(1, &this->vao);
    glDeleteBuffers(1, &this->vbo);
  }

  /* Disable copy semantics for this type. */
  Mesh(Mesh const&) = delete;
  Mesh& operator=(Mesh const&) = delete;

  /* Implement move semantics for this type. */
  Mesh(Mesh&& other) {
    this->vbo = other.vbo;
    other.vbo = GL_NONE;

    this->vao = other.vao;
    other.vao = GL_NONE;

    this->primitiveType = other.primitiveType;
    this->primitiveCount = other.primitiveCount;
  }
  Mesh& operator=(Mesh&& other) {
    if (this == &other) {
      return *this;
    }

    glDeleteBuffers(1, &this->vbo);
    this->vbo = other.vbo;
    other.vbo = GL_NONE;

    glDeleteVertexArrays(1, &this->vao);
    this->vao = other.vao;
    other.vao = GL_NONE;

    this->primitiveType = other.primitiveType;
    this->primitiveCount = other.primitiveCount;

    return *this;
  }
};


// Loads .TRI mesh file from the filesystem.
Mesh loadMeshFromFile(char const* tri_path);
