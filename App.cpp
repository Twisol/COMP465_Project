#include "App.h"
#include "shaders.h"

#include <iostream>

using namespace std;

void App::OnAcquireContext(GLFWwindow* window) {
  this->window = window;
  this->shader_id = create_program_from_files(
      "shaders/vertex.glsl",
      "shaders/fragment.glsl"
  );

  cout << "Running version " << VERSION
  << " with OpenGL version " << glGetString(GL_VERSION)
  << ", GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION)
  << "." << endl;
}

void App::OnReleaseContext() {
  this->window = NULL;
}

// Processes keyboard input.
void App::OnKeyEvent(int key, int action) {
  if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  //...
}

// Updates the application state.
void App::OnTimeStep() {
  glClear(GL_COLOR_BUFFER_BIT);
  //...

  #if GL_SAFETY
  glValidateProgram(this->shader_id);
  #endif

  //...

  glfwSwapBuffers(this->window);
}

// Renders a frame.
void App::OnRedraw() {
  //...
}
