#pragma once

// Math library tailored for OpenGL development
#include <glm/glm.hpp>

struct Model;

// Combines a shared model with an instance-specific reference frame
struct Instance {
  Model* model = nullptr;
  glm::mat4 frame{1.0f};

  Model const* GetModel() const { return this->model; }
  glm::mat4 GetFrame() const { return this->frame; }
};
