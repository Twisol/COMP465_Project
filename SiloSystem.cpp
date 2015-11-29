#include "SiloSystem.h"
#include <glm/gtc/quaternion.hpp>

template<>
struct EntityQuery<SiloComponent> {
  typedef SiloComponent* Entity;

  static bool Query(EntityDatabase& entities, std::string id, SiloComponent** const entity) {
    auto siloItr = entities.silos.find(id);

    if (siloItr == entities.silos.end()) {
      return false;
    }

    *entity = &siloItr->second;
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

  if (state.entities.silos.at(owner).current_missile != "" || state.entities.silos.at(owner).missiles <= 0) {
    canFire = false;
  }

  if (canFire) {
    glm::mat4 worldMatrix = GetWorldMatrix(state.entities, owner);
    std::stringstream tmpMissile;
    glm::vec3 offset;
    glm::quat orientation;
    tmpMissile << "missile: " << owner << " " << state.entities.silos.at(owner).missiles;
    std::string newMissile = tmpMissile.str();
    switch (targeting) {
      case SILO_TARGETING : {
        orientation = state.entities.positions.at(owner).orientation;
        offset = state.entities.positions.at(owner).orientation * glm::vec3{0.0f, 0.0f, -75.0f};
      } break;

      case SHIP_TARGETING : {
        orientation = glm::normalize(glm::rotate(
          state.entities.positions.at(owner).orientation,
          glm::radians(90.0f),
          glm::vec3{1.0f, 0.0f, 0.0f}
        ));
        offset = state.entities.positions.at(owner).orientation * glm::vec3{0.0f, 50.0f, 0.0f};
      } break;

    }
    state.entities.silos.at(owner).current_missile = newMissile;
    state.entities.silos.at(owner).missiles -= 1;
    // instantiate new missile
    state.entities.positions.insert(std::make_pair(newMissile, PositionComponent{
      "::world",
      glm::vec3{worldMatrix * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}} + offset,
      orientation,
    }));
    state.entities.missiles.insert(std::make_pair(newMissile, MissileComponent{owner, targeting}));
    state.entities.models.insert(std::make_pair(newMissile, ModelComponent{missileMesh}));
  }
}

void Update(GameState& /*state*/, double /*delta*/) {

}
