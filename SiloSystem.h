#pragma once

#include "GameState.h"
#include "Mesh.h"
#include <sstream>

// Implements the firing of missiles from silos when targets are in a certain
// detection range of the silo.
class SiloSystem {
private:
   Mesh* missileMesh;

public:
  SiloSystem(Mesh* missileMesh)
    : missileMesh{missileMesh}
  {}

  void Update(GameState& state, double delta);
  static void FireMissile(GameState& state, std::string owner, targeting_mode targeting, Mesh* missileMesh);
};
