#include "App.h"
#include "shaders.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace std;

static std::string const CAMERAS[] = {"camera:front", "camera:top", "camera:unum", "camera:duo", "camera:ship"};

static double const SCALINGS[] = {
  1.0,  // ACE_SPEED
  2.5,  // PILOT_SPEED
  6.25, // TRAINEE_SPEED
  12.5, // DEBUG_SPEED
  50.0, // SUPER_DEBUG_SPEED
};


double App::GetTimeScaling() const {
  return SCALINGS[this->time_scaling_idx];
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
  // with a 75-degree field of view (along the Y axis). The 4/3 ratio determines the field of view
  // along the X axis, and serves to couple the viewing frustum to the (default) dimensions of the canvas.
  this->projectionMatrix = glm::perspective(glm::radians(75.0f), 4.0f / 3.0f, 1.0f, 50001.0f);

  // Load our models into GPU memory
  this->debugMesh = loadMeshFromFile("models/debug.tri");
  this->ruberMesh = loadMeshFromFile("models/ruber.tri");
  this->unumMesh = loadMeshFromFile("models/unum.tri");
  this->duoMesh = loadMeshFromFile("models/duo.tri");
  this->primusMesh = loadMeshFromFile("models/primus.tri");
  this->secundusMesh = loadMeshFromFile("models/secundus.tri");
  this->shipMesh = loadMeshFromFile("models/ship.tri");
  this->missileMesh = loadMeshFromFile("models/missile.tri");

  // Instantiate the Ruber system orbiting bodies.
  {
    this->positions.insert(std::make_pair("Ruber", PositionComponent{"::world", glm::vec3{0.0f, 0.0f, 0.0f}}));
    this->models.insert(std::make_pair("Ruber", ModelComponent{&this->ruberMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f})}));

    this->positions.insert(std::make_pair("Unum", PositionComponent{"Ruber", glm::vec3{4000.0f, 0.0f, 0.0f}}));
    this->physics.insert(std::make_pair("Unum", PhysicsComponent{2.0*M_PI/63.0, 2.0*M_PI/63.0}));
    this->models.insert(std::make_pair("Unum", ModelComponent{&this->unumMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f})}));

    this->positions.insert(std::make_pair("Duo", PositionComponent{"Ruber", glm::vec3{-9000.0f, 0.0f, 0.0f}}));
    this->physics.insert(std::make_pair("Duo", PhysicsComponent{2.0*M_PI/126.0, 2.0*M_PI/126.0}));
    this->models.insert(std::make_pair("Duo", ModelComponent{&this->duoMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f})}));

    this->positions.insert(std::make_pair("Primus", PositionComponent{"Duo", glm::vec3{900.0f, 0.0f, 0.0f}}));
    this->physics.insert(std::make_pair("Primus", PhysicsComponent{2.0*M_PI/63.0, 2.0*M_PI/63.0}));
    this->models.insert(std::make_pair("Primus", ModelComponent{&this->primusMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f})}));

    this->positions.insert(std::make_pair("Secundus", PositionComponent{"Duo", glm::vec3{1750.0f, 0.0f, 0.0f}}));
    this->physics.insert(std::make_pair("Secundus", PhysicsComponent{2.0*M_PI/126.0, 2.0*M_PI/126.0}));
    this->models.insert(std::make_pair("Secundus", ModelComponent{&this->secundusMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f})}));

    this->positions.insert(std::make_pair("ship", PositionComponent{"::world", glm::vec3{5000.0f, 1000.0f, 5000.0f}}));
    this->physics.insert(std::make_pair("ship", PhysicsComponent{0.0, 0.0}));
    this->models.insert(std::make_pair("ship", ModelComponent{&this->shipMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f})}));

    this->positions.insert(std::make_pair("missile", PositionComponent{"::world", glm::vec3{4900.0f, 1000.0f, 4850.0f}}));
    this->physics.insert(std::make_pair("missile", PhysicsComponent{0.0, 0.0}));
    this->models.insert(std::make_pair("missile", ModelComponent{&this->missileMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{1.0f})}));
  }

  // Create some cameras
  {
    this->positions.insert(std::make_pair("camera:front", PositionComponent{"::world", glm::vec3{0.0f, 10000.0f, 20000.0f}}));
    this->cameras.insert(std::make_pair("camera:front", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f})));

    this->positions.insert(std::make_pair("camera:top", PositionComponent{"::world", glm::vec3{0.0f, 20000.0f, 0.0f}}));
    this->cameras.insert(std::make_pair("camera:top", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, -1.0f})));

    this->positions.insert(std::make_pair("camera:unum", PositionComponent{"Unum", glm::vec3{0.0f, 0.0f, -2000.0f}}));
    this->physics.insert(std::make_pair("camera:unum", PhysicsComponent{2.0*M_PI/63.0, 2.0*M_PI/63.0}));
    this->cameras.insert(std::make_pair("camera:unum", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f})));
    //this->models.insert(std::make_pair("camera:unum", ModelComponent{&this->debugMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{100.0f})}));

    this->positions.insert(std::make_pair("camera:duo", PositionComponent{"Duo", glm::vec3{0.0f, 0.0f, 2000.0f}}));
    this->physics.insert(std::make_pair("camera:duo", PhysicsComponent{2.0*M_PI/126.0, 2.0*M_PI/126.0}));
    this->cameras.insert(std::make_pair("camera:duo", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f})));
    //this->models.insert(std::make_pair("camera:duo", ModelComponent{&this->debugMesh, glm::scale(glm::mat4{1.0f}, glm::vec3{100.0f})}));

    this->positions.insert(std::make_pair("camera:ship", PositionComponent{"ship", glm::vec3{50.0f, 100.0f, 400.0f}}));
    this->cameras.insert(std::make_pair("camera:ship", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f})));
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
    glfwSetWindowShouldClose(this->window, GL_TRUE);
  }

  if (action == GLFW_PRESS && key == GLFW_KEY_V) {
    this->active_camera = (this->active_camera + 1) % (sizeof(CAMERAS) / sizeof(CAMERAS[0]));
  } else if (action == GLFW_PRESS && key == GLFW_KEY_T) {
    this->time_scaling_idx = (this->time_scaling_idx + 1) % (sizeof(SCALINGS) / sizeof(SCALINGS[0]));
  }

  //...
}

// Updates the application state.
void App::OnTimeStep(double delta) {
  for (auto& entry : this->physics) {
    auto& entity_name = entry.first;

    // Only update things which are positioned in the game world
    if (this->positions.find(entity_name) == this->positions.end()) {
      continue;
    }

    PositionComponent& position = this->positions.at(entity_name);
    PhysicsComponent& physics = entry.second;

    position.rotation_angle += physics.rotational_velocity * delta;
    position.translation = glm::rotateY(
      position.translation,
      (float)(physics.orbital_velocity * delta)
    );
  }
}

// Renders a frame.
void App::OnRedraw() {
  // Clear the previous render results
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Compute the cumulative transformation from the world basis to clip space.
  glm::mat4 viewMatrix = glm::lookAt(
    this->positions.at(CAMERAS[this->active_camera]).translation, // Position of the camera
    this->cameras.at(CAMERAS[this->active_camera]).at,  // Point to look towards
    this->cameras.at(CAMERAS[this->active_camera]).up  // Direction towards which the top of the camera faces
  );
  {
    PositionComponent const* current = &this->positions.at(CAMERAS[this->active_camera]);
    while (models.find(current->parent) != models.end()) {
      current = &this->positions.at(current->parent);
      viewMatrix = viewMatrix * glm::translate(glm::mat4{1.0f}, -current->translation);
    }
  }

  for (auto& entry : this->models) {
    auto& entity_name = entry.first;

    // Only render things which are positioned in the game world
    if (this->positions.find(entity_name) == this->positions.end()) {
      continue;
    }

    PositionComponent& position = this->positions.at(entity_name);
    ModelComponent& model = entry.second;

    // Compute the cumulative transformation from the entity to the world basis.
    glm::mat4 worldTransform =
        glm::translate(glm::mat4{1.0f}, position.translation)
      * glm::rotate(glm::mat4{1.0f}, (float)position.rotation_angle, glm::vec3{0.0f, 1.0f, 0.0f});

    PositionComponent const* current = &position;
    while (models.find(current->parent) != models.end()) {
      current = &this->positions.at(current->parent);
      worldTransform = glm::translate(glm::mat4{1.0f}, current->translation) * worldTransform;
    }

    // Set up the shader for this instance
    {
      // Use our simple ("100% ambient light") shader.
      glUseProgram(this->shader_id);

      // Configure the render properties of this instance via shader uniforms.
      // Properties specific to each instance may include its position, animation step, etc.

      // This uniform describes the transformation from model coodinates into clip coordinates.
      glm::mat4 frame =
          this->projectionMatrix         // Projection
        * (viewMatrix * worldTransform)  // View
        * model.transformation;          // Model
      GLint location = glGetUniformLocation(this->shader_id, "transform");
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
