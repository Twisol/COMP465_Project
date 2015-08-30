#include "App.h"

// Cross-platform GL context and window toolkit. Handles the boilerplate.
#include <GLFW/glfw3.h>

#include <iostream>

using namespace std;


// Store top-level application information as a static singleton, so that
// the GLUT callbacks can access it properly.
static App G_APP;


// Processes ASCII keyboard input.
void keyboard_callback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mode*/) {
  G_APP.OnKeyEvent(key, action);
}


// A GLFW callback handling GLFW errors
void error_callback(int /*error*/, char const* description) {
  cerr << description << endl;
}

// Entry point.
int main(int /*argc*/, char** /*argv*/) {
  // Initialize GLFW and set an error callback
  if (!glfwInit()) {
    cout << "Unable to initialize GLFW" << endl;
    return 1;
  }

  glfwSetErrorCallback(&error_callback);

  // For the sake of OS X, ensure that we get a modern OpenGL context
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // Create a window with the desired dimensions and title
  GLFWwindow* window = glfwCreateWindow(1024, 768, "Project Phase 1", NULL, NULL);
  if (!window) {
    glfwTerminate();
    cout << "Unable to create GLFW window" << endl;
    return 1;
  }

  // Mark the OpenGL context as current. This is necessary for any gl* and glew* actions to apply to this window.
  glfwMakeContextCurrent(window);

  // Register for keyboard events on this window
  glfwSetKeyCallback(window, &keyboard_callback);

  // Initialize GLEW, which automatically makes available any OpenGL
  // extensions supported on the system.
  //
  // glewInit must only be called after a GL context is made current.
  glewExperimental = GL_TRUE;
  GLenum glewError = glewInit();
  if (glewError != GLEW_OK) {
    cout << "GLEW could not be initialized." << endl;
    return 1;
  }
  while (glGetError() != GL_NO_ERROR) {}  // Purge the GL_INVALID_ENUM which glewInit may cause.

  // Initialize our OpenGL rendering context.
  G_APP.OnAcquireContext(window);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    G_APP.OnTimeStep();
    G_APP.OnRedraw();
  }

  // Clean up after ourselves
  G_APP.OnReleaseContext();
  glfwDestroyWindow(window);

  glfwTerminate();
  return 0;
}
