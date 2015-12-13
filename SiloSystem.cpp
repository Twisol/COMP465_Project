#include "SiloSystem.h"
#include <glm/gtc/quaternion.hpp>

struct FiringEntity {
  std::string id;
  SiloComponent* silo;
  PositionComponent* position;
};

template<>
struct EntityQuery<FiringEntity> {
  typedef FiringEntity Entity;

  static bool Query(EntityDatabase& entities, std::string id, FiringEntity* const entity) {
    auto siloItr = entities.silos.find(id);
    auto posItr = entities.positions.find(id);

    if (siloItr == entities.silos.end() || posItr == entities.positions.end()) {
      return false;
    }

    entity->id = id;
    entity->silo = &siloItr->second;
    entity->position = &posItr->second;
    return true;
  }
};

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

void SiloSystem::FireMissile(GameState& state, std::string owner, targeting_mode targeting, Mesh* missileMesh) {
  bool canFire = true;

  if (state.entities.silos.at(owner).destroyed
    || state.entities.silos.at(owner).missiles <= 0
    || state.entities.silos.at(owner).current_missile != "")
  {
    canFire = false;
  }

  if (canFire) {
    glm::mat4 worldMatrix = GetWorldMatrix(state.entities, owner);
    std::stringstream tmpMissile;
    glm::quat orientation;
    tmpMissile << "missile: " << owner << " " << state.entities.silos.at(owner).missiles;
    std::string newMissile = tmpMissile.str();
    switch (targeting) {
      case SILO_TARGETING : {
        orientation = state.entities.positions.at(owner).orientation;
      } break;

      case SHIP_TARGETING : {
        orientation = glm::normalize(glm::rotate(
          state.entities.positions.at(owner).orientation,
          glm::radians(90.0f),
          glm::vec3{1.0f, 0.0f, 0.0f}
        ));
      } break;

    }
    state.entities.silos.at(owner).current_missile = newMissile;
    state.entities.silos.at(owner).missiles -= 1;
    // instantiate new missile
    state.entities.positions.insert(std::make_pair(newMissile, PositionComponent{
      "::world",
      glm::vec3{worldMatrix * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}},
      orientation,
    }));
    state.entities.missiles.insert(std::make_pair(newMissile, MissileComponent{
      owner,
      targeting,
      state.entities.silos.at(owner).missile_range,
      state.entities.silos.at(owner).missile_speed,
    }));
    state.entities.models.insert(std::make_pair(newMissile, ModelComponent{missileMesh}));
  }
}

void SiloSystem::Update(GameState& state, double /*delta*/) {
  for (auto entity : state.entities.Query<FiringEntity>()) {
    // entities with positive ranges are enemy silos
    if (entity.silo->range > 0.0 && !state.entities.silos.at("ship").destroyed) {
      // calculate distance between current silo and warbird
      auto const silo_position = glm::vec3{GetWorldMatrix(state.entities, entity.id) * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};
      auto const ship_position = glm::vec3{GetWorldMatrix(state.entities, "ship") * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};
      double ship_distance = glm::length(silo_position - ship_position);
      // if the silo is within range, attempt to fire a missile
      if (ship_distance <= entity.silo->range) {
        FireMissile(state, entity.id, SHIP_TARGETING, this->missileMesh);
      }
    }
  }
}
