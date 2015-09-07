#include "App.h"
#include "shaders.h"
#include "Instance.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

void App::OnAcquireContext(GLFWwindow* window) {
  cout << "Running version " << VERSION
    << " with OpenGL version " << glGetString(GL_VERSION)
    <<  ", GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION)
    << "." << endl;

  this->window = window;

  this->shader_id = create_program_from_files("shaders/vertex.glsl", "shaders/fragment.glsl");
  if (this->shader_id == GL_NONE) {
    // TODO: Throw an exception instead so the environment is cleaned up properly.
    exit(1);
  }

  // Transformation from camera space into clip space.
  // This gives a foreshortening effect, and maps all visible geometry into the volume of a unit cube.
  //
  // This camera can only see objects between 1 unit and 101 units away from it,
  // with a 45-degree field of view (along the Y axis). The 4/3 ratio determines the field of view
  // along the X axis, and serves to couple the viewing frustum to the (default) dimensions of the canvas.
  this->projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 1.0f, 101.0f);

  // Transformation from world space into camera space.
  // In other words, this describes the position and rotation of the camera, and the perceived scale of the world.
  // The current value, (0, 0, -20), positions the camera 20 units along +Z.
  // The value is negative because this matrix actually positions the *world space* origin relative to the *camera*.
  this->viewMatrix = glm::translate(glm::mat4{1.0f}, glm::vec3(0.0f, 0.0f, -20.0f));

  // Load our simplistic model into GPU memory and obtain a reference to it.
  this->debugMesh = loadSphereMesh();
  {
    Instance instance;
    instance.mesh = &this->debugMesh;

    // Transformation from model space into world space.
    // In other words, this describes the position, rotation, and scaling of the model relative to the world origin
    //
    // The current value, (0, 0, -5), positions the camera 5 units along -Z.
    instance.frame = glm::translate(instance.frame, glm::vec3{0.0f, 0.0f, -5.0f});
    instance.frame = glm::rotate(instance.frame, glm::radians(45.0f), glm::vec3(0, 1, 0));
    instance.frame = glm::rotate(instance.frame, glm::radians(45.0f), glm::vec3(1, 0, 0));

    this->drawables.push_back(instance);
  }

  // Prevent rendering of fragments which lie behind other fragments
  glEnable(GL_DEPTH_TEST);
  glClearDepth(1.0f);
  glDepthFunc(GL_LESS);

  // Prune geometry (pre-fragment shader) which is facing away from the camera.
  // A triangle is facing "toward" the camera if its vertices are wound counter-clockwise,
  // and facing "away from" the camera if its vertices are wound clockwise.
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);

  // Clearing the color buffer will make everything black.
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
  // Clear the previous render results
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Iterate over all of the drawable objects
  for (Instance instance : this->drawables) {
    // Set up the shader for this instance
    {
      // Use our simple ("orthogonal projection, 100% ambient light") shader.
      glUseProgram(this->shader_id);

      // Configure the render properties of this instance via shader uniforms.
      // Properties specific to each instance may include its position, animation step, etc.

      // This uniform describes the instance's reference frame,
      GLint location = glGetUniformLocation(this->shader_id, "transform");
      glm::mat4 frame = this->projectionMatrix * this->viewMatrix * instance.GetFrame();
      glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(frame));
    }

    // Render the instance's geometry
    {
      // Bind the necessary draw state for this model
      // This state was pre-configured when the Mesh was created.
      Mesh const* const mesh = instance.GetMesh();
      glBindVertexArray(mesh->vao);

      // Confirm that the shader has everything it needs to operate.
      if (!assertShaderValid(this->shader_id)) {
        // TODO: Throw an exception instead so the environment is cleaned up properly.
        exit(1);
      }

      // Issue a draw task to the GPU
      glDrawArrays(mesh->primitiveType, 0, mesh->primitiveCount);
    }
  }

  // Clean up
  glBindVertexArray(GL_NONE);

  // Copy the draw buffer to the screen
  glfwSwapBuffers(this->window);
}
