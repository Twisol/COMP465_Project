#include "App.h"
#include "shaders.h"

#include <GL/glew.h>
#include <iostream>
#include <sstream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace std;

static std::string const CAMERAS[] = {"View: Front", "View: Top", "View: Unum", "View: Duo", "View: Ship"};
static std::string const WARPS[] = {"View: Unum", "View: Duo"};
static float const THRUSTS[] = {10.0f, 50.0f, 200.0f};
static double const SCALINGS[] = {
  1.00, // ACE_SPEED
  0.40, // PILOT_SPEED
  0.16, // TRAINEE_SPEED
  0.08, // DEBUG_SPEED
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
  // This camera can only see objects between 1 unit and 100,001 units away from it,
  // with a 75-degree field of view (along the Y axis). The 4/3 ratio determines the field of view
  // along the X axis, and serves to couple the viewing frustum to the (default) dimensions of the canvas.
  this->projectionMatrix = glm::perspective(glm::radians(75.0f), 4.0f / 3.0f, 1.0f, 100001.0f);

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
    this->models.insert(std::make_pair("Ruber", ModelComponent{&this->ruberMesh}));

    this->positions.insert(std::make_pair("Unum", PositionComponent{"Ruber", glm::vec3{4000.0f, 0.0f, 0.0f}}));
    this->physics.insert(std::make_pair("Unum", PhysicsComponent{2.0*M_PI/63.0, 2.0*M_PI/63.0}));
    this->models.insert(std::make_pair("Unum", ModelComponent{&this->unumMesh}));
    this->silos.insert(std::make_pair("Unum", SiloComponent{5}));

    this->positions.insert(std::make_pair("Duo", PositionComponent{"Ruber", glm::vec3{-9000.0f, 0.0f, 0.0f}}));
    this->physics.insert(std::make_pair("Duo", PhysicsComponent{2.0*M_PI/126.0, 2.0*M_PI/126.0}));
    this->models.insert(std::make_pair("Duo", ModelComponent{&this->duoMesh}));

    this->positions.insert(std::make_pair("Primus", PositionComponent{"Duo", glm::vec3{900.0f, 0.0f, 0.0f}}));
    this->physics.insert(std::make_pair("Primus", PhysicsComponent{2.0*M_PI/63.0, 2.0*M_PI/63.0}));
    this->models.insert(std::make_pair("Primus", ModelComponent{&this->primusMesh}));

    this->positions.insert(std::make_pair("Secundus", PositionComponent{"Duo", glm::vec3{1750.0f, 0.0f, 0.0f}}));
    this->physics.insert(std::make_pair("Secundus", PhysicsComponent{2.0*M_PI/126.0, 2.0*M_PI/126.0}));
    this->models.insert(std::make_pair("Secundus", ModelComponent{&this->secundusMesh}));
    this->silos.insert(std::make_pair("Secundus", SiloComponent{5}));

    this->positions.insert(std::make_pair("ship", PositionComponent{"::world", glm::vec3{5000.0f, 1000.0f, 5000.0f}}));
    this->physics.insert(std::make_pair("ship", PhysicsComponent{0.0, 0.0}));
    this->models.insert(std::make_pair("ship", ModelComponent{&this->shipMesh}));
    this->silos.insert(std::make_pair("ship", SiloComponent{9}));

    this->positions.insert(std::make_pair("missile: ship: 9", PositionComponent{
      "::world",
      this->positions.at("ship").translation + glm::vec3{0.0, 0.0, -40.0},
      this->positions.at("ship").orientation,
    }));
    this->models.insert(std::make_pair("missile: ship: 9", ModelComponent{&this->missileMesh}));
    this->missiles.insert(std::make_pair("missile: ship: 9", MissileComponent{SILO_TARGETING}));
  }

  // Create some cameras
  {
    this->positions.insert(std::make_pair("View: Front", PositionComponent{"::world", glm::vec3{0.0f, 10000.0f, 20000.0f}}));
    this->cameras.insert(std::make_pair("View: Front", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f})));

    this->positions.insert(std::make_pair("View: Top", PositionComponent{"::world", glm::vec3{0.0f, 20000.0f, 0.0f}}));
    this->cameras.insert(std::make_pair("View: Top", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, -1.0f})));

    this->positions.insert(std::make_pair("View: Unum", PositionComponent{"Unum", glm::vec3{0.0f, 0.0f, -2000.0f}}));
    this->cameras.insert(std::make_pair("View: Unum", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f})));

    this->positions.insert(std::make_pair("View: Duo", PositionComponent{"Duo", glm::vec3{0.0f, 0.0f, 2000.0f}}));
    this->cameras.insert(std::make_pair("View: Duo", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f})));

    this->positions.insert(std::make_pair("View: Ship", PositionComponent{"ship", glm::vec3{0.0f, 0.0f, 400.0f}}));
    this->cameras.insert(std::make_pair("View: Ship", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f})));
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


static bool g_IS_MODDED = false;

// Generates simulation window title text
// TODO: Implement frame rate
std::string App::GetTitle() const {
  std::stringstream builder;
  builder << "Warbird: " << this->silos.at("ship").missiles
          << " | Unum: " << this->silos.at("Unum").missiles
          << " | Secundus: " << this->silos.at("Secundus").missiles
          << " | U/S: " << (1000.0 * GetTimeScaling()) / 40.0
          << " | F/S: ??"
          << " | " << CAMERAS[this->active_camera]
          << " | Gravity: " << (gravity_enabled ? "On" : "Off")
          << " | Thrust: " << THRUSTS[active_thrust_factor]
          ;
  return builder.str();
}

glm::mat4 App::GetViewMatrix(std::string const& id) const {
  PositionComponent const& position = this->positions.at(id);
  CameraComponent const& camera = this->cameras.at(id);

  glm::mat4 viewMatrix = glm::lookAt(
    position.translation, // Position of the camera
    camera.at,  // Point to look towards
    camera.up  // Direction towards which the top of the camera faces
  );

  if (positions.find(position.parent) != positions.end()) {
    PositionComponent const& parent = this->positions.at(position.parent);
    viewMatrix *= glm::mat4_cast(glm::inverse(parent.orientation));
  }

  PositionComponent const* current = &position;
  while (positions.find(current->parent) != positions.end()) {
    current = &this->positions.at(current->parent);
    viewMatrix *= glm::translate(glm::mat4{1.0f}, -current->translation);
  }

  return viewMatrix;
}

glm::mat4 App::GetWorldMatrix(std::string const& id) const {
  PositionComponent const& position = this->positions.at(id);

  glm::mat4 worldMatrix =
      glm::translate(glm::mat4{1.0f}, position.translation)
    * glm::mat4_cast(position.orientation);

  PositionComponent const* current = &position;
  while (positions.find(current->parent) != positions.end()) {
    current = &this->positions.at(current->parent);
    worldMatrix = glm::translate(glm::mat4{1.0f}, current->translation) * worldMatrix;
  }

  return worldMatrix;
}

// Processes keyboard input.
void App::OnKeyEvent(int key, int action, int mods) {
  // This is a workaround for a bug in GLFW which prevents modifier key releases
  // from being properly recognized during polling (which we do in the
  // OnTimeStep method).
  g_IS_MODDED = (mods & GLFW_KEY_LEFT_ALT);

  if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
    glfwSetWindowShouldClose(this->window, GL_TRUE);
  }

  if (action == GLFW_PRESS && key == GLFW_KEY_V) {
    this->active_camera = (this->active_camera + 1) % (sizeof(CAMERAS) / sizeof(CAMERAS[0]));
  } else if (action == GLFW_PRESS && key == GLFW_KEY_T) {
    this->time_scaling_idx = (this->time_scaling_idx + 1) % (sizeof(SCALINGS) / sizeof(SCALINGS[0]));
  } else if (action == GLFW_PRESS && key == GLFW_KEY_S) {
    this->active_thrust_factor = (this->active_thrust_factor + 1) % (sizeof(THRUSTS) / sizeof(THRUSTS[0]));
  } else if (action == GLFW_PRESS && key == GLFW_KEY_W) {
    glm::mat4 worldMatrix = glm::inverse(GetViewMatrix(WARPS[this->active_warp]));
    this->positions.at("ship").translation = glm::vec3{worldMatrix * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};
    this->positions.at("ship").orientation = glm::normalize(glm::quat{glm::mat3{glm::inverseTranspose(worldMatrix)}});

    this->active_warp = (this->active_warp + 1) % (sizeof(WARPS) / sizeof(WARPS[0]));
  } else if (action == GLFW_PRESS && key == GLFW_KEY_G) {
    this->gravity_enabled = !gravity_enabled;
  }
}


static void get_input_vectors(GLFWwindow* const window, glm::vec3* const rotation, glm::vec3* const translation) {
  if (glfwGetKey(window, GLFW_KEY_UP) && !g_IS_MODDED) {
    *translation += glm::vec3{0.0f, 0.0f, -1.0f};
  }

  if (glfwGetKey(window, GLFW_KEY_DOWN) && !g_IS_MODDED) {
    *translation += glm::vec3{0.0f, 0.0f, 1.0f};
  }

  if (glfwGetKey(window, GLFW_KEY_LEFT) && !g_IS_MODDED) {
    *rotation += glm::vec3{0.0f, glm::degrees(0.02f), 0.0f};
  }

  if (glfwGetKey(window, GLFW_KEY_RIGHT) && !g_IS_MODDED) {
    *rotation += glm::vec3{0.0f, glm::degrees(-0.02f), 0.0f};
  }

  if (glfwGetKey(window, GLFW_KEY_UP) && g_IS_MODDED) {
    *rotation += glm::vec3{glm::degrees(-0.02f), 0.0f, 0.0f};
  }

  if (glfwGetKey(window, GLFW_KEY_DOWN) && g_IS_MODDED) {
    *rotation += glm::vec3{glm::degrees(0.02f), 0.0f, 0.0f};
  }

  if (glfwGetKey(window, GLFW_KEY_LEFT) && g_IS_MODDED) {
    *rotation += glm::vec3{0.0f, 0.0f, glm::degrees(0.02f)};
  }

  if (glfwGetKey(window, GLFW_KEY_RIGHT) && g_IS_MODDED) {
    *rotation += glm::vec3{0.0f, 0.0f, glm::degrees(-0.02f)};
  }
}

// Updates the application state.
void App::OnTimeStep(double delta) {
  // viewing window title update
  glfwSetWindowTitle(this->window, this->GetTitle().c_str());

  {
    // ship navigation
    PositionComponent& ship_position = this->positions.at("ship");

    // Determine ship thrusts from user input
    glm::vec3 rotation{0.0f};
    glm::vec3 translation{0.0f};
    get_input_vectors(this->window, &rotation, &translation);

    // Scale ship thrusts by time and thrust factors
    rotation *= (float)delta;
    translation *= THRUSTS[this->active_thrust_factor]*(float)delta;

    if (glm::length(rotation) != 0) {
      ship_position.orientation =
          glm::normalize(glm::rotate(
            ship_position.orientation,
            glm::length(rotation),
            rotation
          ));
    }

    // Orient the translation vector down the ship's heading.
    ship_position.translation += ship_position.orientation * translation;

    // Update ship's position with respect to Ruber's gravity
    if (this->gravity_enabled) {
      PositionComponent& sun_position = this->positions.at("Ruber");

      glm::vec3 distance_vector = sun_position.translation - ship_position.translation;
      float distance = glm::length(distance_vector);

      glm::vec3 gravity_vector = (90000000.0f / (distance*distance)) * (distance_vector / distance);
      ship_position.translation += gravity_vector * (float)delta;
    }
  }

  // Update all entities based on their physical motion
  for (auto& entry : this->physics) {
    auto& entity_name = entry.first;

    // Only update things which are positioned in the game world
    if (this->positions.find(entity_name) == this->positions.end()) {
      continue;
    }

    PositionComponent& position = this->positions.at(entity_name);
    PhysicsComponent& physics = entry.second;

    // Rotate the entity
    if (glm::length(physics.angular_velocity) != 0) {
      position.orientation =
          glm::normalize(glm::rotate(
            position.orientation,
            glm::length(physics.angular_velocity * (float)delta),
            physics.angular_velocity
          ));
    }

    // Translate the entity
    position.translation =
        ( physics.translational_velocity * (float)delta
        + glm::rotate(
            position.translation,
            (float)(physics.orbital_velocity * delta),
            glm::vec3{0.0f, 1.0f, 0.0f}
          )
        );
  }
}

// Renders a frame.
void App::OnRedraw() {
  // Clear the previous render results
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Compute the cumulative transformation from the world basis to clip space.
  glm::mat4 const viewMatrix = GetViewMatrix(CAMERAS[this->active_camera]);

  for (auto& entry : this->models) {
    auto& entity_name = entry.first;

    // Only render things which are positioned in the game world
    if (this->positions.find(entity_name) == this->positions.end()) {
      continue;
    }

    ModelComponent const& model = entry.second;

    // Set up the shader for this instance
    {
      // Use our simple ("100% ambient light") shader.
      glUseProgram(this->shader_id);

      // Configure the render properties of this instance via shader uniforms.
      // Properties specific to each instance may include its position, animation step, etc.

      glm::mat4 const modelview = viewMatrix * GetWorldMatrix(entity_name);
      GLint modelviewLocation = glGetUniformLocation(this->shader_id, "modelview");
      glUniformMatrix4fv(modelviewLocation, 1, GL_FALSE, glm::value_ptr(modelview));

      GLint projectionLocation = glGetUniformLocation(this->shader_id, "projection");
      glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(this->projectionMatrix));

      GLint normalMatrixLocation = glGetUniformLocation(this->shader_id, "normalMatrix");
      glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat3{glm::inverseTranspose(modelview)}));
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
