#include "App.h"
#include "shaders.h"

// Math library tailored for OpenGL development
#include <glm/glm.hpp>

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
void App::OnRedraw() {
  //...

  // Upload the triangle model to GPU memory
  //
  // TODO: Move this out of the render loop. This only needs to be done once.
  GLuint vbo = GL_NONE;
  {
    glm::vec3 vertices[3] = {
        glm::vec3{-1.0f, -1.0f, 0.0f},
        glm::vec3{1.0f, -1.0f, 0.0f},
        glm::vec3{0.0f, 1.0f, 0.0f},
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * 3 * sizeof(glm::vec3), vertices, GL_STATIC_DRAW);
  }

  // Create a vertex array object (VAO).
  //
  // TODO: I don't really know what these are, but you have to have one.
  GLuint vao = GL_NONE;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Load the model into the VAO and tell it the format and location of vertex information in the model.
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  // TODO: What even is this? It's necessary, but what does it mean?
  glEnableVertexAttribArray(0);

  #if GL_VALIDATE_SHADERS
  {
    // Ensure that all shader inputs are available, and other such stuff.
    glValidateProgram(this->shader_id);

    GLint isValid = GL_FALSE;
    glGetProgramiv(this->shader_id, GL_VALIDATE_STATUS, &isValid);
    if (isValid == GL_FALSE) {
      cout << "Shader validation failed.";
      exit(0);
    }
  }
  #endif

  // Install our shader into the pipeline for this draw batch.
  glUseProgram(this->shader_id);

  // If you ever want to see what's in your GPU memory, comment out these two lines.
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  // Copy the draw buffer to the screen
  glfwSwapBuffers(this->window);

  // Clean up
  glDisableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

  // Delete our triangle model
  glDeleteBuffers(1, &vbo);
}

// Renders a frame.
void App::OnTimeStep() {
  //...
}
