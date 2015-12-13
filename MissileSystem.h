#pragma once

#include "GameState.h"

#include <glm/gtc/matrix_access.hpp>
#include <cmath>

// Query result object for interfacing with the EntityDatabase
struct DirectableEntity {
  std::string id;
  PositionComponent* position;
  MissileComponent* missile;
};
template<>
struct EntityQuery<PositionComponent> {
  typedef PositionComponent* Entity;

  static bool Query(EntityDatabase& entities, std::string id, PositionComponent** const entity) {
    auto posItr = entities.positions.find(id);

    if (posItr == entities.positions.end()) {
      return false;
    }

    *entity = &posItr->second;
    return true;
  }
};

// Query result object for interfacing with the EntityDatabase
template<>
struct EntityQuery<DirectableEntity> {
  typedef DirectableEntity Entity;

  static bool Query(EntityDatabase& entities, std::string id, DirectableEntity* const entity) {
    auto posItr = entities.positions.find(id);
    auto missileItr = entities.missiles.find(id);

    if (posItr == entities.positions.end() || missileItr == entities.missiles.end()) {
      return false;
    }

    entity->id = id;
    entity->position = &posItr->second;
    entity->missile = &missileItr->second;
    return true;
  }
};

// Implements missile orientation, propulsion, and tracking of tarets
class MissileSystem {
protected:
  static glm::mat4 GetWorldMatrix(EntityDatabase& entities, std::string const& id) {
    PositionComponent const& position = entities.positions.at(id);

    glm::mat4 worldMatrix =
        glm::translate(glm::mat4{1.0f}, position.translation)
      * glm::mat4_cast(position.orientation);

    PositionComponent const* current = &position;
    while (entities.positions.find(current->parent) != entities.positions.end()) {
      current = &entities.positions.at(current->parent);
      worldMatrix = glm::translate(glm::mat4{1.0f}, current->translation) * worldMatrix;
    }

    return worldMatrix;
  }

  static float GetDistance(EntityDatabase& entities, std::string const& id1, std::string const& id2) {
    auto const pos1 = glm::vec3{GetWorldMatrix(entities, id1) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};
    auto const pos2 = glm::vec3{GetWorldMatrix(entities, id2) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};
    return glm::length(pos1 - pos2);
  }

public:
  void Update(GameState& state, double delta) {
    auto view = state.entities.Query<DirectableEntity>();
    for (auto itr = view.begin(); itr != view.end();) {
      auto entity = *itr;

      entity.missile->time_to_live -= delta;
      std::cout << entity.id << ": " << entity.missile->time_to_live << std::endl;

      if (entity.missile->time_to_live <= 0) {
      // It's dead now
        state.entities.silos.at(entity.missile->owner).current_missile = "";
        itr.remove();
        continue;
      } else if (entity.missile->time_to_live <= MissileComponent::MAX_LIFETIME - MissileComponent::IDLE_PERIOD) {
      // Aim towards the target (if target unassigned, assign target)
        if (entity.missile->target == "" || state.entities.silos.at(entity.missile->target).destroyed) {
          // Categories of target
          static const std::string ships[] = {"ship"};
          static const std::string silos[] = {"Unum Silo", "Secundus Silo"};

          // Figure out which category of target to aim for
          std::string const* targets = nullptr;
          int len = 0;
          if (entity.missile->targeting == SHIP_TARGETING) {
            targets = ships;
            len = sizeof(ships)/sizeof(ships[0]);
          } else if (entity.missile->targeting == SILO_TARGETING) {
            targets = silos;
            len = sizeof(silos)/sizeof(silos[0]);
          } else {
            // WHAT HAPPENED HERE
          }

          // Find the nearest of target within our range
          std::string target = "";
          float target_distance = 0.0f;
          for (int i = 0; i < len; ++i) {
            auto candidate = targets[i];
            if (state.entities.silos.at(candidate).destroyed) {
            // Skip candidate targets which have already been destroyed
              continue;
            }

            float distance = GetDistance(state.entities, entity.id, candidate);
            if (distance < entity.missile->range) {
              if (target == "" || distance < target_distance) {
                target = candidate;
              }
            }
          }

          entity.missile->target = target;
        }

        PositionComponent* target = nullptr;
        if (EntityQuery<PositionComponent>::Query(state.entities, entity.missile->target, &target)) {
          // Find the world-relative position of the entity
          auto const target_position = glm::vec3{GetWorldMatrix(state.entities, entity.missile->target) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};
          auto const missile_position = glm::vec3{GetWorldMatrix(state.entities, entity.id) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};

          // Calculate the axis of rotation for the missile
          auto const target_direction = glm::normalize(target_position - missile_position);
          auto const missile_at = glm::normalize(entity.position->orientation * glm::vec3{0.0f, 0.0f, -1.0f});
          auto const rotation_axis = glm::cross(missile_at, target_direction);

          // If the missile isn't pointing at the target already, rotate to face it.
          if (fabs(glm::dot(missile_at, target_direction)) < 1) {
            auto rotation = glm::normalize(glm::rotate(
              glm::quat{1.0f, 0.0f, 0.0f, 0.0f},
              acosf(glm::dot(missile_at, target_direction)),
              rotation_axis
            ));
            entity.position->orientation = rotation * entity.position->orientation;
          }
        } else {
          entity.missile->target = "";
        }
      }

      entity.position->translation += entity.position->orientation * (((float)delta)*glm::vec3{0.0f, 0.0f, -entity.missile->speed});

      ++itr;
    }
  }
};
