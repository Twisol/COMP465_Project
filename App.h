#pragma once

// Project shared header
#include "shared.h"

// Cross-platform GL context and window toolkit. Handles the boilerplate.
#include <GLFW/glfw3.h>


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
class App : IComponent {
public:
  App()
      : window(NULL), shader_id(0)
  {}

  virtual void OnAcquireContext(GLFWwindow* window);
  virtual void OnReleaseContext();

  virtual void OnKeyEvent(int key, int action);
  virtual void OnTimeStep();
  virtual void OnRedraw();

private:
  GLFWwindow* window;  // The GLFW window for this app
  GLuint shader_id;  // The ID of the current shader program.
};
