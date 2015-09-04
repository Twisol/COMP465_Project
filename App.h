#pragma once

// Project shared header
#include "shared.h"
#include "Model.h"
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
  Model triangleModel;  // A reference to model data in GPU memory

  // This is the projection matrix from camera space to clip space.
  // Remember that in clip space, only content within a unit cube centered at the origin is considered visible.
  // Everything else is clipped - hence the name, "clip space".
  glm::mat4 projectionMatrix{1.0f};

  // This is the matrix that maps from world space to camera space.
  glm::mat4 viewMatrix{1.0f};

  std::vector<Instance> drawables;  // The set of things which will be drawn on redraw.
};
