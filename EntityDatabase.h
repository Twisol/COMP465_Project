#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <string>
#include <unordered_map>
#include <iostream>

#include "Mesh.h"

enum targeting_mode {
  SILO_TARGETING,
  SHIP_TARGETING,
};

struct OrbitComponent {
  // Angular velocity relative to the parent.
  double orbital_velocity = 0.0;

  // Angular velocity relative to the entity's center.
  glm::vec3 angular_velocity{0.0f, 0.0f, 0.0f};

  OrbitComponent(double orbital_velocity, float yaw_velocity)
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
    : at{at}, up{up}
  {}
};

struct SiloComponent {
  // silo's destroyed flag
  bool destroyed = false;
  // silo's auto-fire detection range (0.0 for ship which does not auto-fire)
  double range = 0.0;
  // A store of missiles per site
  int missiles = 0;
  // Name of currently-fired missile
  std::string current_missile = "";

  // generated missiles' targeting range
  double missile_range = 0.0;
  // generated missiles' movement speed
  double missile_speed = 0.0;

  SiloComponent(int missiles, double range, double missile_range, double missile_speed)
    : range{range}, missiles{missiles}, missile_range{missile_range}, missile_speed{missile_speed}
  {}
};

struct MissileComponent {
  // time to death in seconds
  static constexpr double const MAX_LIFETIME = 80.0;
  // time before targeting behavior in seconds
  static constexpr double const IDLE_PERIOD = 8.0;

  // Missile's "Owner"
  std::string owner = "";
  // Target to hit
  std::string target = "";
  // Missile type
  targeting_mode targeting;
  // Missile's range
  double range = 0.0;
  // Missile's speed
  double speed = 0.0;

  double time_to_live = MAX_LIFETIME;

  MissileComponent(std::string owner, targeting_mode targeting, double range, double speed)
    : owner(owner), targeting(targeting), range(range), speed(speed)
  {}
};


struct EntityDatabase;

template<typename T>
struct EntityQuery {
  // typedef T Entity;
  // static bool Query(EntityDatabase& /*entities*/, std::string /*id*/, Entity* const /*entity*/);
};

struct EntityDatabase {
  std::unordered_map<std::string, PositionComponent> positions;
  std::unordered_map<std::string, OrbitComponent> orbits;
  std::unordered_map<std::string, ModelComponent> models;
  std::unordered_map<std::string, CameraComponent> cameras;
  std::unordered_map<std::string, SiloComponent> silos;
  std::unordered_map<std::string, MissileComponent> missiles;

  template<typename T>
  class Iterator {
    using inner_iterator = std::unordered_map<std::string, PositionComponent>::iterator;
    using value_type = typename EntityQuery<T>::Entity;

    EntityDatabase& entities;
    inner_iterator itr;
    inner_iterator const end;

  public:
    Iterator(EntityDatabase& entities, inner_iterator itr, inner_iterator end)
      : entities(entities), itr{itr}, end{end}
    {
      // Make sure `itr` either points to a matching entity or to `end`
      value_type entity;
      while (this->itr != end) {
        if (EntityQuery<T>::Query(entities, this->itr->first, &entity)) {
          break;
        }
        ++this->itr;
      }
    }

    value_type operator*() {
      value_type entity;
      while (itr != end) {
        if (!EntityQuery<T>::Query(entities, itr->first, &entity)) {
          ++itr;
        } else {
          return entity;
        }
      }

      throw new std::exception{};
    }

    Iterator<T>& operator++() {
      ++itr;

      value_type entity;
      while (itr != end) {
        if (EntityQuery<T>::Query(entities, itr->first, &entity)) {
          break;
        }
        ++itr;
      }

      return *this;
    }

    bool operator!=(Iterator<T> const& other) {
      return itr != other.itr;
    }

    void remove() {
      entities.orbits.erase(itr->first);
      entities.models.erase(itr->first);
      entities.cameras.erase(itr->first);
      entities.silos.erase(itr->first);
      entities.missiles.erase(itr->first);

      itr = entities.positions.erase(itr);

      // Make sure `itr` either points to a matching entity or to `end`
      value_type entity;
      while (itr != end) {
        if (EntityQuery<T>::Query(entities, itr->first, &entity)) {
          break;
        }
        ++itr;
      }
    }
  };

  template<typename T>
  struct View {
    EntityDatabase& entities;

    Iterator<T> begin() {
      return Iterator<T>{entities, entities.positions.begin(), entities.positions.end()};
    }

    Iterator<T> end() {
      return Iterator<T>{entities, entities.positions.end(), entities.positions.end()};
    }
  };

  template<typename T>
  View<T> Query() {
    return View<T>{*this};
  }
};
