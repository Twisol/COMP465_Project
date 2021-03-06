#include "App.h"
#include "SiloSystem.h"

#include <GL/glew.h>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace std;

static double const SILO_RANGE = 5000.0;
static double const SHIP_RANGE = 0.0;
static double const MISSILE_RANGE = 5000.0;
static double const SILO_MISSILE_SPEED = 125.0f;
static double const SHIP_MISSILE_SPEED = 500.0f;
static int const SILO_COUNT = 5;
static int const SHIP_COUNT = 10;

std::string const CAMERAS[] = {"View: Front", "View: Top", "View: Unum", "View: Duo", "View: Ship"};
float const THRUSTS[] = {250.0f, 1250.0f, 5000.0f};
static std::string const WARPS[] = {"View: Unum", "View: Duo"};
static double const SCALINGS[] = {
  1.00, // ACE_SPEED
  0.40, // PILOT_SPEED
  0.16, // TRAINEE_SPEED
  0.08, // DEBUG_SPEED
};

// Query result object for interfacing with the EntityDatabase
struct OrbitalEntity {
  std::string id;
  PositionComponent* position;
  OrbitComponent* orbit;
};
template<>
struct EntityQuery<OrbitalEntity> {
  typedef OrbitalEntity Entity;

  static bool Query(EntityDatabase& entities, std::string id, OrbitalEntity* const entity) {
    auto posItr = entities.positions.find(id);
    auto orbitItr = entities.orbits.find(id);

    if (posItr == entities.positions.end() || orbitItr == entities.orbits.end()) {
      return false;
    }

    entity->id = id;
    entity->position = &posItr->second;
    entity->orbit = &orbitItr->second;
    return true;
  }
};

// Query result object for interfacing with the EntityDatabase
struct CollidableEntity {
  std::string id;
  PositionComponent* position;
  ModelComponent* model;
};

template<>
struct EntityQuery<CollidableEntity> {
  typedef CollidableEntity Entity;

  static bool Query(EntityDatabase& entities, std::string id, CollidableEntity* const entity) {
    auto posItr = entities.positions.find(id);
    auto modelItr = entities.models.find(id);

    if (posItr == entities.positions.end() || modelItr == entities.models.end()) {
      return false;
    }

    entity->id = id;
    entity->position = &posItr->second;
    entity->model = &modelItr->second;
    return true;
  }
};


// Provides the current time-coupling between the game world and the real world.
double App::GetTimeScaling() const {
  return SCALINGS[this->state.time_scaling_idx];
}

void App::OnAcquireContext(GLFWwindow* window) {
  cout << "Running version " << VERSION
       << " with OpenGL version " << glGetString(GL_VERSION)
       << ", GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION)
       << "." << endl;

  this->window = window;

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

  // Enable seamless cube mapping
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  // Clearing the color buffer will make everything black.
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  // Load our models into GPU memory
  this->debugMesh = loadMeshFromFile("models/debug.tri");
  this->ruberMesh = loadMeshFromFile("models/ruber.tri");
  this->unumMesh = loadMeshFromFile("models/unum.tri");
  this->duoMesh = loadMeshFromFile("models/duo.tri");
  this->primusMesh = loadMeshFromFile("models/primus.tri");
  this->secundusMesh = loadMeshFromFile("models/secundus.tri");
  this->siloMesh = loadMeshFromFile("models/silo.tri");
  this->shipMesh = loadMeshFromFile("models/ship.tri");
  this->missileMesh = loadMeshFromFile("models/missile.tri");

  // Give ships and missiles a larger bounding sphere for collision detection
  this->shipMesh.boundingRadius += 10;
  this->missileMesh.boundingRadius += 10;

  // Instantiate the Ruber system orbiting bodies.
  {
    state.entities.positions.insert(std::make_pair("Ruber", PositionComponent{"::world", glm::vec3{0.0f, 0.0f, 0.0f}}));
    state.entities.models.insert(std::make_pair("Ruber", ModelComponent{&this->ruberMesh}));

    state.entities.positions.insert(std::make_pair("Unum", PositionComponent{"Ruber", glm::vec3{4000.0f, 0.0f, 0.0f}}));
    state.entities.orbits.insert(std::make_pair("Unum", OrbitComponent{2.0*M_PI/63.0, 2.0*M_PI/63.0}));
    state.entities.models.insert(std::make_pair("Unum", ModelComponent{&this->unumMesh}));

    state.entities.positions.insert(std::make_pair("Unum Silo", PositionComponent{"Unum", glm::vec3{0.0f, 250.0f, 0.0f}}));
    state.entities.models.insert(std::make_pair("Unum Silo", ModelComponent{&this->siloMesh}));
    state.entities.silos.insert(std::make_pair("Unum Silo", SiloComponent{SILO_COUNT, SILO_RANGE, MISSILE_RANGE, SILO_MISSILE_SPEED}));

    state.entities.positions.insert(std::make_pair("Duo", PositionComponent{"Ruber", glm::vec3{-9000.0f, 0.0f, 0.0f}}));
    state.entities.orbits.insert(std::make_pair("Duo", OrbitComponent{2.0*M_PI/126.0, 2.0*M_PI/126.0}));
    state.entities.models.insert(std::make_pair("Duo", ModelComponent{&this->duoMesh}));

    state.entities.positions.insert(std::make_pair("Primus", PositionComponent{"Duo", glm::vec3{900.0f, 0.0f, 0.0f}}));
    state.entities.orbits.insert(std::make_pair("Primus", OrbitComponent{2.0*M_PI/63.0, 2.0*M_PI/63.0}));
    state.entities.models.insert(std::make_pair("Primus", ModelComponent{&this->primusMesh}));

    state.entities.positions.insert(std::make_pair("Secundus", PositionComponent{"Duo", glm::vec3{1750.0f, 0.0f, 0.0f}}));
    state.entities.orbits.insert(std::make_pair("Secundus", OrbitComponent{2.0*M_PI/126.0, 2.0*M_PI/126.0}));
    state.entities.models.insert(std::make_pair("Secundus", ModelComponent{&this->secundusMesh}));

    state.entities.positions.insert(std::make_pair("Secundus Silo", PositionComponent{"Secundus", glm::vec3{0.0f, 200.0f, 0.0f}}));
    state.entities.models.insert(std::make_pair("Secundus Silo", ModelComponent{&this->siloMesh}));
    state.entities.silos.insert(std::make_pair("Secundus Silo", SiloComponent{SILO_COUNT, SILO_RANGE, MISSILE_RANGE, SILO_MISSILE_SPEED}));

    state.entities.positions.insert(std::make_pair("ship", PositionComponent{"::world", glm::vec3{5000.0f, 1000.0f, 5000.0f}}));
    state.entities.models.insert(std::make_pair("ship", ModelComponent{&this->shipMesh}));
    state.entities.silos.insert(std::make_pair("ship", SiloComponent{SHIP_COUNT, SHIP_RANGE, MISSILE_RANGE, SHIP_MISSILE_SPEED}));
  }

  // Create some cameras
  {
    state.entities.positions.insert(std::make_pair("View: Front", PositionComponent{"::world", glm::vec3{0.0f, 10000.0f, 20000.0f}}));
    state.entities.cameras.insert(std::make_pair("View: Front", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f})));

    state.entities.positions.insert(std::make_pair("View: Top", PositionComponent{"::world", glm::vec3{0.0f, 20000.0f, 0.0f}}));
    state.entities.cameras.insert(std::make_pair("View: Top", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, -1.0f})));

    state.entities.positions.insert(std::make_pair("View: Ship", PositionComponent{"ship", glm::vec3{0.0f, 300.0f, 1000.0f}}));
    state.entities.cameras.insert(std::make_pair("View: Ship", CameraComponent(glm::vec3{0.0f, 300.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f})));

    state.entities.positions.insert(std::make_pair("View: Unum", PositionComponent{"Unum", glm::vec3{0.0f, 0.0f, -8000.0f}}));
    state.entities.cameras.insert(std::make_pair("View: Unum", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f})));

    state.entities.positions.insert(std::make_pair("View: Duo", PositionComponent{"Duo", glm::vec3{0.0f, 0.0f, 8000.0f}}));
    state.entities.cameras.insert(std::make_pair("View: Duo", CameraComponent(glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f})));
  }
}

void App::OnReleaseContext() {
  this->window = nullptr;
}


static bool g_IS_MODDED = false;

// Computes the view matrix from the world to the given entity.
glm::mat4 App::GetViewMatrix(std::string const& id) const {
  PositionComponent const& position = state.entities.positions.at(id);
  CameraComponent const& camera = state.entities.cameras.at(id);

  glm::mat4 viewMatrix = glm::lookAt(
    position.translation, // Position of the camera
    camera.at,  // Point to look towards
    camera.up  // Direction towards which the top of the camera faces
  );

  if (state.entities.positions.find(position.parent) != state.entities.positions.end()) {
    PositionComponent const& parent = state.entities.positions.at(position.parent);
    viewMatrix *= glm::mat4_cast(glm::inverse(parent.orientation));
  }

  PositionComponent const* current = &position;
  while (state.entities.positions.find(current->parent) != state.entities.positions.end()) {
    current = &state.entities.positions.at(current->parent);
    viewMatrix *= glm::translate(glm::mat4{1.0f}, -current->translation);
  }

  return viewMatrix;
}

// Computes the model matrix from the given entity to the world.
glm::mat4 App::GetWorldMatrix(std::string const& id) const {
  PositionComponent const& position = state.entities.positions.at(id);

  glm::mat4 worldMatrix =
      glm::translate(glm::mat4{1.0f}, position.translation)
    * glm::mat4_cast(position.orientation);

  PositionComponent const* current = &position;
  while (state.entities.positions.find(current->parent) != state.entities.positions.end()) {
    current = &state.entities.positions.at(current->parent);
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
    this->state.active_camera = (this->state.active_camera + 1) % (sizeof(CAMERAS) / sizeof(CAMERAS[0]));
  } else if (action == GLFW_PRESS && key == GLFW_KEY_X) {
    this->state.active_camera = (this->state.active_camera + (sizeof(CAMERAS) / sizeof(CAMERAS[0])) - 1) % (sizeof(CAMERAS) / sizeof(CAMERAS[0]));
  } else if (action == GLFW_PRESS && key == GLFW_KEY_T) {
    this->state.time_scaling_idx = (this->state.time_scaling_idx + 1) % (sizeof(SCALINGS) / sizeof(SCALINGS[0]));
  } else if (action == GLFW_PRESS && key == GLFW_KEY_S) {
    this->state.active_thrust_factor = (this->state.active_thrust_factor + 1) % (sizeof(THRUSTS) / sizeof(THRUSTS[0]));
  } else if (action == GLFW_PRESS && key == GLFW_KEY_W) {
    glm::mat4 worldMatrix = glm::inverse(GetViewMatrix(WARPS[this->state.active_warp]));
    state.entities.positions.at("ship").translation = glm::vec3{worldMatrix * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};
    state.entities.positions.at("ship").orientation = glm::normalize(glm::quat{glm::mat3{glm::inverseTranspose(worldMatrix)}});

    this->state.active_warp = (this->state.active_warp + 1) % (sizeof(WARPS) / sizeof(WARPS[0]));
  } else if (action == GLFW_PRESS && key == GLFW_KEY_G) {
    this->state.gravity_enabled = !this->state.gravity_enabled;
  } else if (action == GLFW_PRESS && key == GLFW_KEY_F) {
    SiloSystem::FireMissile(state, "ship", SILO_TARGETING, &this->missileMesh);
  } else if (action == GLFW_PRESS && key == GLFW_KEY_A) {
    this->state.is_lit_global = !state.is_lit_global;
  } else if (action == GLFW_PRESS && key == GLFW_KEY_P) {
    this->state.is_lit_ruber = !state.is_lit_ruber;
  } else if (action == GLFW_PRESS && key == GLFW_KEY_H) {
    this->state.is_lit_headlight = !state.is_lit_headlight;
  }
}

// Computes the raw input vectors from the user input
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
  // Handle ship navigation, if we aren't dead...
  if (!state.entities.silos.at("ship").destroyed) {
    // ship navigation
    PositionComponent& ship_position = state.entities.positions.at("ship");

    // Determine ship thrusts from user input
    glm::vec3 rotation{0.0f};
    glm::vec3 translation{0.0f};
    get_input_vectors(this->window, &rotation, &translation);

    // Scale ship thrusts by time and thrust factors
    rotation *= (float)delta;
    translation *= THRUSTS[this->state.active_thrust_factor]*(float)delta;

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
    if (this->state.gravity_enabled) {
      PositionComponent& sun_position = state.entities.positions.at("Ruber");

      glm::vec3 distance_vector = sun_position.translation - ship_position.translation;
      float distance = glm::length(distance_vector);

      glm::vec3 gravity_vector = (90000000.0f / (distance*distance)) * (distance_vector / distance);
      ship_position.translation += gravity_vector * (float)delta;
    }
  }

  // Update all orbiting bodies
  for (auto entity : state.entities.Query<OrbitalEntity>()) {
    // Rotate the entity
    if (glm::length(entity.orbit->angular_velocity) != 0) {
      entity.position->orientation =
          glm::normalize(glm::rotate(
            entity.position->orientation,
            glm::length(entity.orbit->angular_velocity * (float)delta),
            entity.orbit->angular_velocity
          ));
    }

    // Translate the entity
    entity.position->translation = glm::rotate(
      entity.position->translation,
      (float)(entity.orbit->orbital_velocity * delta),
      glm::vec3{0.0f, 1.0f, 0.0f}
    );
  }

  // Check for collisions.
  auto view = state.entities.Query<CollidableEntity>();
  for (auto itr1 = view.begin(); itr1 != view.end();) {
    auto entity = *itr1;
    bool entity_destroyed = false;

    // Don't test missiles which are not targeting for collision
    if (state.entities.missiles.find(entity.id) != state.entities.missiles.end()) {
      if (state.entities.missiles.at(entity.id).time_to_live > MissileComponent::MAX_LIFETIME - MissileComponent::IDLE_PERIOD) {
        ++itr1;
        continue;
      }
    }

    for (auto itr2 = view.begin(); itr2 != view.end();) {
      auto collidable = *itr2;
      auto collidable_destroyed = false;

      // Don't test missiles which are not targeting for collision
      if (state.entities.missiles.find(collidable.id) != state.entities.missiles.end()) {
        if (state.entities.missiles.at(collidable.id).time_to_live > MissileComponent::MAX_LIFETIME - MissileComponent::IDLE_PERIOD) {
          ++itr2;
          continue;
        }
      }

      // Don't collide with ourself
      if (collidable.id != entity.id) {
        glm::vec3 pos1 = glm::vec3{GetWorldMatrix(entity.id) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};
        glm::vec3 pos2 = glm::vec3{GetWorldMatrix(collidable.id) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};

        if (glm::length(pos2 - pos1) < entity.model->mesh->boundingRadius + collidable.model->mesh->boundingRadius) {
        // Collision! The bounding spheres overlap.
          if (state.entities.silos.find(entity.id) != state.entities.silos.end()) {
          // Mark silos as destroyed
            state.entities.silos.at(entity.id).destroyed = true;
          } else if (state.entities.missiles.find(entity.id) != state.entities.missiles.end()) {
          // Remove missiles from the database
            entity_destroyed = true;
            state.entities.silos.at(state.entities.missiles.at(entity.id).owner).current_missile = "";
          }

          if (state.entities.silos.find(collidable.id) != state.entities.silos.end()) {
          // Mark silos as destroyed
            state.entities.silos.at(collidable.id).destroyed = true;
          } else if (state.entities.missiles.find(collidable.id) != state.entities.missiles.end()) {
          // Remove missiles from the database
            collidable_destroyed = true;
            state.entities.silos.at(state.entities.missiles.at(collidable.id).owner).current_missile = "";
          }
        }
      }

      if (collidable_destroyed) {
        itr2.remove();
      } else {
        ++itr2;
      }
    }

    if (entity_destroyed) {
      itr1.remove();
    } else {
      ++itr1;
    }
  }
}
