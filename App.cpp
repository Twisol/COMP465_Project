#include "App.h"
#include "shaders.h"

#include <iostream>
#include <cmath>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

void App::InstantiateOrbitingBodies() {
  this->positions.insert(std::make_pair("Ruber", Position{"::origin", glm::vec3{0.0f, 0.0f, 0.0f}, 0.0f}));
  this->models.insert(std::make_pair("Ruber", Model{&this->debugMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{2000.0f})}));
  this->renderables.push_back("Ruber");

  this->positions.insert(std::make_pair("Unum", Position{"Ruber", glm::vec3{4000.0f, 0.0f, 0.0f}, 2.0*M_PI/63.0}));
  this->models.insert(std::make_pair("Unum", Model{&this->debugMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{200.0f})}));
  this->orbiters.push_back("Unum");
  this->renderables.push_back("Unum");

  this->positions.insert(std::make_pair("Duo", Position{"Ruber", glm::vec3{-9000.0f, 0.0f, 0.0f}, 2.0*M_PI/126.0}));
  this->models.insert(std::make_pair("Duo", Model{&this->debugMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{400.0f})}));
  this->orbiters.push_back("Duo");
  this->renderables.push_back("Duo");

  this->positions.insert(std::make_pair("Primus", Position{"Duo", glm::vec3{900.0f, 0.0f, 0.0f}, 2.0*M_PI/63.0}));
  this->models.insert(std::make_pair("Primus", Model{&this->debugMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{100.0f})}));
  this->orbiters.push_back("Primus");
  this->renderables.push_back("Primus");

  this->positions.insert(std::make_pair("Secundus", Position{"Duo", glm::vec3{1750.0f, 0.0f, 0.0f}, 2.0*M_PI/126.0}));
  this->models.insert(std::make_pair("Secundus", Model{&this->debugMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{150.0f})}));
  this->orbiters.push_back("Secundus");
  this->renderables.push_back("Secundus");
}


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
  // This camera can only see objects between 1 unit and 50001 units away from it,
  // with a 60-degree field of view (along the Y axis). The 4/3 ratio determines the field of view
  // along the X axis, and serves to couple the viewing frustum to the (default) dimensions of the canvas.
  this->projectionMatrix = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 1.0f, 50001.0f);

  // Transformation from world space into camera space.
  // In other words, this describes the position and rotation of the camera, and the perceived scale of the world.
  this->viewMatrix = glm::lookAt(
      glm::vec3{0.0f, 10000.0f, 20000.0f},  // Position of the camera
      glm::vec3{0.0f, 0.0f, 0.0f},  // Point to look towards
      glm::vec3{0.0f, 1.0f, -1.0f}  // Direction towards which the top of the camera faces
  );

  // Load our simplistic model into GPU memory and obtain a reference to it.
  this->debugMesh = loadSphereMesh();

  // Instantiate the Ruber system orbiting bodies.
  this->InstantiateOrbitingBodies();

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
void App::OnTimeStep(double /*delta*/) {
  for (auto orbiter : orbiters) {
    // ...
  }
}

// Renders a frame.
void App::OnRedraw() {
  // Clear the previous render results
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Compute the cumulative transformation from the world basis to clip space.
  glm::mat4 clipTransform = this->projectionMatrix * this->viewMatrix;

  for (auto entity : this->renderables) {
    // If the entity is unrenderable in some way, skip it.
    if (this->positions.find(entity) == this->positions.end()
      || this->models.find(entity) == this->models.end()) {
      continue;
    }

    Position& position = this->positions.at(entity);
    Model& model = this->models.at(entity);

    // Compute the cumulative transformation from the entity to the world basis.
    glm::mat4 worldTransform = glm::translate(glm::mat4{1.0f}, position.translation);
    Position const* current = &position;
    while (models.find(current->parent) != models.end()) {
      current = &this->positions.at(current->parent);
      worldTransform = glm::translate(worldTransform, current->translation);
    }

    // Set up the shader for this instance
    {
      // Use our simple ("orthogonal projection, 100% ambient light") shader.
      glUseProgram(this->shader_id);

      // Configure the render properties of this instance via shader uniforms.
      // Properties specific to each instance may include its position, animation step, etc.

      // This uniform describes the instance's reference frame,
      GLint location = glGetUniformLocation(this->shader_id, "transform");
      glm::mat4 frame = clipTransform * worldTransform * model.transformation;
      glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(frame));
    }

    // Render the instance's geometry
    {
      // Bind the necessary draw state for this model
      // This state was pre-configured when the Mesh was created.
      auto& mesh = model.mesh;
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
