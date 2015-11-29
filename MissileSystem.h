#pragma once

#include "GameState.h"

#include <glm/gtc/matrix_access.hpp>
#include <cmath>

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
      // Aim towards the target, if any
        if (entity.missile->target != "") {
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
            // TODO: fix this
            std::cout << "Dot: " << glm::dot(missile_at, target_direction) << std::endl;
            std::cout << "|Cross|: " << glm::length(rotation_axis) << std::endl;
            if (acosf(glm::dot(missile_at, target_direction)) <= 1) {
              entity.position->orientation =
                glm::normalize(glm::rotate(
                  entity.position->orientation,
                  acosf(glm::dot(missile_at, target_direction)),
                  rotation_axis
                ));
            }
          } else {
            entity.missile->target = "";
          }
        }
      }

      ++itr;
    }
  }
};
