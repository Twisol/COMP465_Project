#pragma once

// Project shared header
#include "shared.h"

#include "Mesh.h"

// Cross-platform GL context and window toolkit. Handles the boilerplate.
#include <GLFW/glfw3.h>
// GLM math objects
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <unordered_map>
#include <vector>
#include <string>

struct PositionComponent {
  // The parent frame.
  std::string parent = "::origin";

  // Translation relative to the parent frame.
  glm::vec3 translation{0.0f};

  // Rotation (in radians) about the Y-axis relative to the entity's own frame
  double rotation_angle = 0.0f;

  // Angular velocity relative to the parent.
  // TODO: This belongs in a PhysicsComponent
  double angular_velocity = 0.0;


  PositionComponent(std::string parent, glm::vec3 const& translation, double angular_velocity)
    : parent{parent}, translation{translation}, angular_velocity{angular_velocity}
  {}
};

struct ModelComponent {
  Mesh const* mesh = nullptr;
  glm::mat4 transformation{1.0f};


  ModelComponent(Mesh const* mesh, glm::mat4 const& transformation)
    : mesh{mesh}, transformation{transformation}
  {}
};


// A structure representing top-level information about the application.
class App  {
public:
  void OnAcquireContext(GLFWwindow* window);
  void OnReleaseContext();

  void OnKeyEvent(int key, int action);
  void OnTimeStep(double delta);
  void OnRedraw();

  // Returns the game's clock speed in game seconds per real second.
  double GetTimeScaling() const {
    return this->time_scaling;
  }

private:
  GLFWwindow* window = nullptr;  // The GLFW window for this app
  GLuint shader_id = GL_NONE;  // The ID of the current shader program.

  Mesh debugMesh;  // A mesh meant for testing and debugging.
  Mesh ruberMesh;
  Mesh unumMesh;
  Mesh duoMesh;
  Mesh primusMesh;
  Mesh secundusMesh;

  // Transformation from camera space into clip space.
  // This maps all visible content onto the volume of a unit cube centered at the origin.
  glm::mat4 projectionMatrix{1.0f};

  int active_camera = 0;

  // Coupling factor between game time and real time.
  // This is the number of "game seconds" per "real seconds",
  // or more humorously, the number of seconds per second.
  double time_scaling = 1.0;

  // Entity component tables
  std::unordered_map<std::string, PositionComponent> positions;
  std::unordered_map<std::string, ModelComponent> models;
};
