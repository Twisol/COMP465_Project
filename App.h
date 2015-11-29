#pragma once

#include "Mesh.h"
#include "GameState.h"
#include "EntityDatabase.h"

// Cross-platform GL context and window toolkit. Handles the boilerplate.
#include <GLFW/glfw3.h>
// GLM math objects
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <unordered_map>
#include <vector>
#include <string>
#include <exception>
#include <iostream>


// A structure representing top-level information about the application.
class App  {
public:
  void OnAcquireContext(GLFWwindow* window);
  void OnReleaseContext();

  void OnKeyEvent(int key, int action, int mods);
  void OnTimeStep(double delta);

  // Returns the simulation's clock speed in game seconds per real second.
  double GetTimeScaling() const;

protected:
  glm::mat4 GetViewMatrix(std::string const& id) const;
  glm::mat4 GetWorldMatrix(std::string const& id) const;

private:
  GLFWwindow* window = nullptr;  // The GLFW window for this app

public:
  Mesh debugMesh;  // A mesh meant for testing and debugging.
  Mesh ruberMesh;
  Mesh unumMesh;
  Mesh duoMesh;
  Mesh primusMesh;
  Mesh secundusMesh;
  Mesh siloMesh;
  Mesh shipMesh;
  Mesh missileMesh;

  GameState state;
};
