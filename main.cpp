#include "App.h"

#include <iostream>

using namespace std;


static GLFWwindow* setupGLFW(int width, int height, char const* title, GLFWerrorfun error_callback) {
  glfwSetErrorCallback(error_callback);

  // Initialize GLFW and set an error callback
  if (!glfwInit()) {
    cout << "Unable to initialize GLFW" << endl;
    return nullptr;
  }

  // For the sake of OS X, ensure that we get a modern OpenGL context
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // Create a window with the desired dimensions and title
  GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    cout << "Unable to create GLFW window" << endl;
    return nullptr;
  }

  return window;
}

static bool setupGLEW() {
  // Initialize GLEW, which automatically makes available any OpenGL
  // extensions supported on the system.
  //
  // glewInit must only be called after a GL context is made current.
  glewExperimental = GL_TRUE;
  GLenum glewError = glewInit();
  if (glewError != GLEW_OK) {
    cout << "GLEW could not be initialized." << endl;
    return false;
  }

  // Purge the GL_INVALID_ENUM which glewInit may cause.
  while (glGetError() != GL_NO_ERROR) {}

  return true;
}


// Keep a reference to the active app so that GLFW callbacks can access it.
static IComponent* G_APP = nullptr;

// Processes ASCII keyboard input.
void keyboard_callback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mode*/) {
  G_APP->OnKeyEvent(key, action);
}

// A GLFW callback handling GLFW errors
void error_callback(int /*error*/, char const* description) {
  cerr << description << endl;
}

// Entry point.
int main(int /*argc*/, char** /*argv*/) {
  // Initialize GLFW
  GLFWwindow* const window = setupGLFW(1024, 768, "Project Phase 1", &error_callback);
  if (!window) {
    cout << "Unable to initialize GLFW." << endl;
    return 1;
  }

  // Mark the OpenGL context as current. This is necessary for any gl* and glew* actions to apply to this window.
  glfwMakeContextCurrent(window);

  // Initialize GLEW.
  // Note that this has to happen AFTER a GL context is made current.
  if (!setupGLEW()) {
    cout << "GLEW could not be initialized." << endl;
    return 1;
  }

  // Set up our app object.
  // Note that this has to happen AFTER a GL context is made current.
  App app;

  {
    G_APP = &app;

    // Register for keyboard events on this window
    glfwSetKeyCallback(window, &keyboard_callback);

    // Notify the app object that a GL context has been acquired
    G_APP->OnAcquireContext(window);

    // Game Loop pattern
    // More information at http://gameprogrammingpatterns.com/game-loop.html
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      G_APP->OnTimeStep();
      G_APP->OnRedraw();
    }

    // Clean up after ourselves
    G_APP->OnReleaseContext();

    G_APP = nullptr;
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  return 0;
}
