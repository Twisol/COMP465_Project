#pragma once

#include "Mesh.h"

// Cross-platform GL context and window toolkit. Handles the boilerplate.
#include <GLFW/glfw3.h>
// GLM math objects
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <unordered_map>
#include <vector>
#include <string>

struct PhysicsComponent {
  // Angular velocity relative to the parent.
  double orbital_velocity = 0.0;

  // Angular velocity relative to the entity's center.
  glm::vec3 angular_velocity{0.0f, 0.0f, 0.0f};

  // Translational velocity relative to the entity
  glm::vec3 translational_velocity{0.0f, 0.0f, 0.0f};

  PhysicsComponent(double orbital_velocity, float yaw_velocity)
    : orbital_velocity{orbital_velocity}, angular_velocity{0.0f, yaw_velocity, 0.0f}
  {}
};

struct PositionComponent {
  // The parent whose origin we translate against.
  std::string parent = "::world";

  // Translation relative to the parent.
  glm::vec3 translation{0.0f};

  // Orientation relative to the world.
  glm::quat orientation{};

  PositionComponent(std::string parent, glm::vec3 const& translation, glm::quat const& orientation = glm::quat{})
    : parent{parent}, translation{translation}, orientation{orientation}
  {}
};

struct ModelComponent {
  Mesh const* mesh = nullptr;

  ModelComponent(Mesh const* mesh)
    : mesh{mesh}
  {}
};

struct CameraComponent {
  // Point to look at
  glm::vec3 at{0.0f, 0.0f, 0.0f};
  // Where the top of the camera is pointing
  glm::vec3 up{0.0f, 0.0f, 0.0f};

  CameraComponent(glm::vec3 at, glm::vec3 up)
    : at(at), up(up)
  {}
};

struct SiloComponent {
  // A store of missiles
  int missiles;

  SiloComponent(int missiles)
    : missiles(missiles)
  {}
};

// A structure representing top-level information about the application.
class App  {
public:
  void OnAcquireContext(GLFWwindow* window);
  void OnReleaseContext();

  void OnKeyEvent(int key, int action, int mods);
  void OnTimeStep(double delta);
  void OnRedraw();

  // Returns the simulation's clock speed in game seconds per real second.
  double GetTimeScaling() const;

protected:
  std::string GetTitle() const;
  glm::mat4 GetViewMatrix(std::string const& id) const;
  glm::mat4 GetWorldMatrix(std::string const& id) const;

private:
  GLFWwindow* window = nullptr;  // The GLFW window for this app
  GLuint shader_id = GL_NONE;  // The ID of the current shader program.

  Mesh debugMesh;  // A mesh meant for testing and debugging.
  Mesh ruberMesh;
  Mesh unumMesh;
  Mesh duoMesh;
  Mesh primusMesh;
  Mesh secundusMesh;
  Mesh shipMesh;
  Mesh missileMesh;

  // Transformation from camera space into clip space.
  // This maps all visible content onto the volume of a unit cube centered at the origin.
  glm::mat4 projectionMatrix{1.0f};

  // Index of the active camera in our list of selectable cameras
  int active_camera = 0;

  // Index of the active warp zone in our list of selectable warp zones
  int active_warp = 0;

  // Index of the current ship thrust factor
  int active_thrust_factor = 0;

  // Index of the active coupling factor between game time and real time.
  int time_scaling_idx = 0;

  // Toggles simulation of gravity
  bool gravity_enabled = false;

  // Entity component tables
  std::unordered_map<std::string, PositionComponent> positions;
  std::unordered_map<std::string, PhysicsComponent> physics;
  std::unordered_map<std::string, ModelComponent> models;
  std::unordered_map<std::string, CameraComponent> cameras;
  std::unordered_map<std::string, SiloComponent> silos;
};
