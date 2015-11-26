#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <string>
#include <unordered_map>

#include "Mesh.h"

enum targeting_mode {
  SILO_TARGETING,
  MISSILE_TARGETING,
};

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
    : at{at}, up{up}
  {}
};

struct SiloComponent {
  // A store of missiles per site
  int missiles = 0;
  // Name of currently-fired missile
  std::string current_missile = "";

  SiloComponent(int missiles)
    : missiles{missiles}
  {}
};

struct MissileComponent {
  // time to death in msec
  static constexpr double const MAX_LIFETIME = 50000;
  // time before targeting behavior in msec
  static constexpr double const IDLE_PERIOD = 5000;

  // Target to hit
  std::string target;
  // Missile type
  targeting_mode targeting;

  double time_to_live = 0;

  MissileComponent(std::string target, targeting_mode targeting, double time_to_live)
    : target(target), targeting(targeting), time_to_live(time_to_live)
  {}
};



struct EntityDatabase;

template<typename Entity>
struct EntityQuery {
  static bool Query(EntityDatabase& /*entities*/, std::string /*id*/, Entity* const /*entity*/) {
    return false;
  }
};

struct EntityDatabase {
  std::unordered_map<std::string, PositionComponent> positions;
  std::unordered_map<std::string, PhysicsComponent> physics;
  std::unordered_map<std::string, ModelComponent> models;
  std::unordered_map<std::string, CameraComponent> cameras;
  std::unordered_map<std::string, SiloComponent> silos;

  template<typename T>
  class Iterator {
    using inner_iterator = std::unordered_map<std::string, PositionComponent>::iterator;

    EntityDatabase& entities;
    inner_iterator itr;
    inner_iterator const end;

  public:
    Iterator(EntityDatabase& entities, inner_iterator itr, inner_iterator end)
      : entities{entities}, itr{itr}, end{end}
    {}

    typename EntityQuery<T>::Entity operator*() {
      typename EntityQuery<T>::Entity entity;
      while (itr != end) {
        if (!EntityQuery<T>::Query(entities, itr->first, &entity)) {
          itr++;
        } else {
          return entity;
        }
      }

      throw new std::exception{};
    }

    Iterator<T>& operator++() {
      itr++;

      typename EntityQuery<T>::Entity entity;
      while (itr != end) {
        if (EntityQuery<T>::Query(entities, itr->first, &entity)) {
          break;
        }
        itr++;
      }

      return *this;
    }

    bool operator!=(Iterator<T> const& other) {
      return itr != other.itr;
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
