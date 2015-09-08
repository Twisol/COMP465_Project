#pragma once

#include <glm/vec3.hpp>

// Represents a point and orientation in space relative to some basis.
struct Frame {
  // Frame which we are positioned relative to.
  // A null value represents the world's fixed basis
  Frame const* parent = nullptr;

  // Translation relative to the parent frame.
  glm::vec3 translation{0.0f};

/* Constructors */
  Frame() = default;

  Frame(Frame const* parent, glm::vec3&& translation)
      : parent{parent}, translation{translation}
  {}

/* Copy semantics */
  Frame(Frame const& other) = default;
  Frame& operator=(Frame const& other) = default;
};
