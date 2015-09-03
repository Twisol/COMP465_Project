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
  this->shader_id = create_program_from_files("shaders/vertex.glsl", "shaders/fragment.glsl");

  this->triangleModel = loadTriangleModel();
  {
    Instance instance;
    instance.model = &this->triangleModel;
    instance.frame = glm::mat4 {
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    this->drawables.push_back(instance);
  }

  if (this->shader_id == GL_NONE) {
    // TODO: Throw an exception instead so the environment is cleaned up properly.
    exit(1);
  }

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void App::OnReleaseContext() {
  this->window = nullptr;
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

  // Iterate over all of the drawable objects
  for (Instance instance : this->drawables) {
    // Use our simple ("orthogonal projection, 100% ambient light") shader for this object.
    glUseProgram(this->shader_id);

    // Set up uniform values
    // Uniforms usually involve properties of an _instance_ of a model, such as its reference frame.
    {
      GLint location = glGetUniformLocation(this->shader_id, "transform");
      glm::mat4 frame = instance.GetFrame();
      glUniformMatrix4fv(location, 1, 0, (GLfloat const*)&frame);
    }

    // Bind the necessary draw state for this model
    // This state was pre-configured when the Model was created.
    instance.GetModel()->BindVertexData();
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
