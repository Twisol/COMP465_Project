#pragma once

// Project shared header
#include "shared.h"
#include "Mesh.h"
#include "Instance.h"

// Cross-platform GL context and window toolkit. Handles the boilerplate.
#include <GLFW/glfw3.h>
// GLM forward declarations
#include <glm/fwd.hpp>

#include <vector>


// An interface for top-level render loop hooks.
class IComponent {
public:
  virtual ~IComponent() {}

  virtual void OnAcquireContext(GLFWwindow* /*window*/) {}
  virtual void OnReleaseContext() {}

  virtual void OnKeyEvent(int /*key*/, int /*action*/) {}
  virtual void OnTimeStep() {}
  virtual void OnRedraw() {}
};


// A structure representing top-level information about the application.
class App : public IComponent {
public:
  virtual void OnAcquireContext(GLFWwindow* window);
  virtual void OnReleaseContext();

  virtual void OnKeyEvent(int key, int action);
  virtual void OnTimeStep();
  virtual void OnRedraw();

private:
  GLFWwindow* window = nullptr;  // The GLFW window for this app
  GLuint shader_id = GL_NONE;  // The ID of the current shader program.
  Mesh triangleMesh;  // A reference to vertex data in GPU memory

  // Transformation from camera space into clip space.
  // This maps all visible content onto the volume of a unit cube centered at the origin.
  glm::mat4 projectionMatrix{1.0f};

  // Transformation from world space into camera space.
  // In other words, this describes the position and rotation of the camera, and the perceived scale of the world.
  glm::mat4 viewMatrix{1.0f};

  std::vector<Instance> drawables;  // The set of things which will be drawn on redraw.
};
