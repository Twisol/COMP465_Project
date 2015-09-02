#include "App.h"
#include "shaders.h"
#include "Instance.h"

#include <iostream>

using namespace std;

void App::OnAcquireContext(GLFWwindow* window) {
  cout << "Running version " << VERSION
    << " with OpenGL version " << glGetString(GL_VERSION)
    <<  ", GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION)
    << "." << endl;

  this->window = window;
  this->shader_id = create_program_from_files(
      "shaders/vertex.glsl",
      "shaders/fragment.glsl"
  );
  this->triangleModel = loadTriangleModel();

  if (this->shader_id == GL_NONE) {
    // TODO: Throw an exception instead so the environment is cleaned up properly.
    exit(1);
  }

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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
  //...
}

// Renders a frame.
void App::OnRedraw() {
  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT);

  // TODO: Iterate over a collection of drawables
  {
    // Use our simple ("orthogonal projection, 100% ambient light") shader for this model.
    glUseProgram(this->shader_id);

    // TODO: Store this instance somewhere more permanent
    Instance tri;
    tri.model = &this->triangleModel;

    // Set up uniform values
    // Uniforms usually involve properties of an _instance_ of a model, such as its reference frame.
    {
      GLint location = glGetUniformLocation(this->shader_id, "transform");
      glUniformMatrix4fv(location, 1, 0, (GLfloat*)&tri.frame);
    }

    // Bind the necessary draw state for this model
    glBindVertexArray(tri.model->vao);
    if (!assertShaderValid(this->shader_id)) {
      // TODO: Throw an exception instead so the environment is cleaned up properly.
      exit(1);
    }

    // Issue a draw task to the GPU
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }

  // Clean up
  glBindVertexArray(GL_NONE);

  // Copy the draw buffer to the screen
  glfwSwapBuffers(this->window);
}
