#pragma once

#include "GameState.h"
#include "Mesh.h"
#include <sstream>

static float const DETECTION_RANGE = 5000.0f;

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
