#pragma once

#include <GL/glew.h>
// Cross-platform GL context and window toolkit. Handles the boilerplate.
#include <GLFW/glfw3.h>

#include "GameState.h"

class RenderSystem {
private:
  GLFWwindow* window = nullptr;
  GLuint shader_id = GL_NONE;  // The ID of the current shader program.
  GLuint skybox_shader_id = GL_NONE;  // The ID of the skybox shader.

  Mesh skyboxMesh;
  GLuint cubeMap = GL_NONE;  // The cube map texture

  // Transformation from camera space into clip space.
  // This maps all visible content onto the volume of a unit cube centered at the origin.
  glm::mat4 projectionMatrix{1.0f};

public:
  RenderSystem(GLFWwindow* window, glm::mat4 projectionMatrix);

  void Render(GameState& state);
};
