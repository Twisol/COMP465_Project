#pragma once

#include "Frame.h"
#include "Mesh.h"

#include <glm/mat4x4.hpp>
#include <string>

struct Entity {
public:
  Entity(std::string name)
      : name(name)
  {}

public:
  std::unique_ptr<Frame> frame = nullptr;  // Positional frame, possibly with respect to another entity

  Mesh const* mesh = nullptr;  // Visible geometry
  glm::mat4 transformation{1.0f};  // Model-relative transformation

  std::string name = "<unnamed>";
};
