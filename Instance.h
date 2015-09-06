#pragma once

// Math library tailored for OpenGL development
#include <glm/glm.hpp>

struct Model;

// Combines a shared model with an instance-specific reference frame
struct Instance {
  Mesh* mesh = nullptr;
  glm::mat4 frame{1.0f};

  Mesh const* GetMesh() const { return this->mesh; }
  glm::mat4 GetFrame() const { return this->frame; }
};
