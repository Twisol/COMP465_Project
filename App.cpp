#include "App.h"
#include "shaders.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;


void App::InstantiateOrbitingBodies() {
  // Ruber
  std::unique_ptr<Entity> ruber{new Entity("Ruber")};
  ruber->mesh = &this->debugMesh;
  ruber->transformation = glm::scale(glm::mat4{1.0f}, glm::vec3{2000.0f});
  ruber->frame.reset(new Frame{});

  // Unum
  std::unique_ptr<Entity> unum{new Entity("Unum")};
  unum->mesh = &this->debugMesh;
  unum->transformation = glm::scale(glm::mat4{1.0f}, glm::vec3{200.0f});
  unum->frame.reset(new Frame{ruber->frame.get(), glm::vec3{4000.0f, 0.0f, 0.0f}});

  // Duo
  std::unique_ptr<Entity> duo{new Entity("Duo")};
  duo->mesh = &this->debugMesh;
  duo->transformation = glm::scale(glm::mat4{1.0f}, glm::vec3{400.0f});
  duo->frame.reset(new Frame{ruber->frame.get(), glm::vec3{-9000.0f, 0.0f, 0.0f}});

  // Primus
  std::unique_ptr<Entity> primus{new Entity("Primus")};
  primus->mesh = &this->debugMesh;
  primus->transformation = glm::scale(glm::mat4{1.0f}, glm::vec3{100.0f});
  primus->frame.reset(new Frame{duo->frame.get(), glm::vec3{900.0f, 0.0f, 0.0f}});

  // Secundus
  std::unique_ptr<Entity> secundus{new Entity("Secundus")};
  secundus->mesh = &this->debugMesh;
  secundus->transformation = glm::scale(glm::mat4{1.0f}, glm::vec3{150.0f});
  secundus->frame.reset(new Frame{duo->frame.get(), glm::vec3{1750.0f, 0.0f, 0.0f}});

  this->entities.push_back(std::move(ruber));
  this->entities.push_back(std::move(unum));
  this->entities.push_back(std::move(duo));
  this->entities.push_back(std::move(primus));
  this->entities.push_back(std::move(secundus));
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
  //...
}

// Renders a frame.
void App::OnRedraw() {
  // Clear the previous render results
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Compute the cumulative transformation from the world basis to clip space.
  glm::mat4 clipTransform = this->projectionMatrix * this->viewMatrix;

  // Iterate over all entities.
  for (auto& entity : this->entities) {
    // If the entity is unrenderable in some way, skip it.
    if (!(entity->mesh && entity->frame)) {
      continue;
    }

    // Compute the cumulative transformation from the entity to the world basis.
    glm::mat4 worldTransform = glm::translate(glm::mat4{1.0f}, entity->frame->translation);
    Frame const* parent = entity->frame->parent;
    while (parent) {
      worldTransform = glm::translate(worldTransform, parent->translation);
      parent = parent->parent;
    }

    // Set up the shader for this instance
    {
      // Use our simple ("orthogonal projection, 100% ambient light") shader.
      glUseProgram(this->shader_id);

      // Configure the render properties of this instance via shader uniforms.
      // Properties specific to each instance may include its position, animation step, etc.

      // This uniform describes the instance's reference frame,
      GLint location = glGetUniformLocation(this->shader_id, "transform");
      glm::mat4 frame = clipTransform * worldTransform * entity->transformation;
      glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(frame));
    }

    // Render the instance's geometry
    {
      // Bind the necessary draw state for this model
      // This state was pre-configured when the Mesh was created.
      auto& mesh = entity->mesh;
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
