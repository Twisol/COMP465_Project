#pragma once

#include "EntityDatabase.h"

struct GameState {
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
  EntityDatabase entities;
};

extern std::string const CAMERAS[];
extern float const THRUSTS[];
