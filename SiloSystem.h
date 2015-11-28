#pragma once

#include "GameState.h"
#include <sstream>

class SiloSystem {
public:
  SiloSystem() {}

  void Update(GameState& state, double delta);
  static void FireMissile(GameState& state, std::string owner, targeting_mode targeting, Mesh* missileMesh);
};
