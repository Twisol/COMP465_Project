#pragma once

#include <GL/glew.h>
// Cross-platform GL context and window toolkit. Handles the boilerplate.
#include <GLFW/glfw3.h>

#include "GameState.h"

struct RenderSystem {
public:
  GLFWwindow* window = nullptr;
  GLuint shader_id = GL_NONE;  // The ID of the current shader program.

  // Transformation from camera space into clip space.
  // This maps all visible content onto the volume of a unit cube centered at the origin.
  glm::mat4 projectionMatrix{1.0f};

  RenderSystem(GLFWwindow* window, GLuint shader_id, glm::mat4 projectionMatrix)
    : window{window}, shader_id{shader_id}, projectionMatrix{projectionMatrix}
  {}

  void Render(GameState& state);
};
