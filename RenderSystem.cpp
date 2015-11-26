#include "RenderSystem.h"
#include "shaders.h"

struct RenderableEntity {
  std::string id;
  PositionComponent* position;
  ModelComponent* model;
};

template<>
struct EntityQuery<RenderableEntity> {
  typedef RenderableEntity Entity;

  static bool Query(EntityDatabase& entities, std::string id, RenderableEntity* const entity) {
    auto posItr = entities.positions.find(id);
    auto modelItr = entities.models.find(id);

    if (posItr == entities.positions.end() || modelItr == entities.models.end()) {
      return false;
    }

    entity->id = id;
    entity->position = &posItr->second;
    entity->model = &modelItr->second;
    return true;
  }
};

static glm::mat4 GetViewMatrix(EntityDatabase& entities, std::string const& id) {
  PositionComponent const& position = entities.positions.at(id);
  CameraComponent const& camera = entities.cameras.at(id);

  glm::mat4 viewMatrix = glm::lookAt(
    position.translation, // Position of the camera
    camera.at,  // Point to look towards
    camera.up  // Direction towards which the top of the camera faces
  );

  if (entities.positions.find(position.parent) != entities.positions.end()) {
    PositionComponent const& parent = entities.positions.at(position.parent);
    viewMatrix *= glm::mat4_cast(glm::inverse(parent.orientation));
  }

  PositionComponent const* current = &position;
  while (entities.positions.find(current->parent) != entities.positions.end()) {
    current = &entities.positions.at(current->parent);
    viewMatrix *= glm::translate(glm::mat4{1.0f}, -current->translation);
  }

  return viewMatrix;
}

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

RenderSystem::RenderSystem(GLFWwindow* window, glm::mat4 projectionMatrix)
  : window{window}, projectionMatrix{projectionMatrix}
{
  this->shader_id = create_program_from_files("shaders/vertex.glsl", "shaders/fragment.glsl");
  if (this->shader_id == GL_NONE) {
    // TODO: Throw an exception instead so the environment is cleaned up properly.
    exit(1);
  }
}

void RenderSystem::Render(GameState& state) {
  // Clear the previous render results
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Compute the cumulative transformation from the world basis to clip space.
  glm::mat4 const viewMatrix = GetViewMatrix(state.entities, CAMERAS[state.active_camera]);

  for (auto entity : state.entities.Query<RenderableEntity>()) {
    // Set up the shader for this instance
    {
      // Use our simple ("100% ambient light") shader.
      glUseProgram(this->shader_id);

      // Configure the render properties of this instance via shader uniforms.
      // Properties specific to each instance may include its position, animation step, etc.

      glm::mat4 const modelview = viewMatrix * GetWorldMatrix(state.entities, entity.id);
      GLint modelviewLocation = glGetUniformLocation(this->shader_id, "modelview");
      glUniformMatrix4fv(modelviewLocation, 1, GL_FALSE, glm::value_ptr(modelview));

      GLint projectionLocation = glGetUniformLocation(this->shader_id, "projection");
      glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(this->projectionMatrix));

      GLint normalMatrixLocation = glGetUniformLocation(this->shader_id, "normalMatrix");
      glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat3{glm::inverseTranspose(modelview)}));
    }

    // Render the instance's geometry
    {
      // Bind the necessary draw state for this model
      // This state was pre-configured when the Mesh was created.
      auto& mesh = entity.model->mesh;
      glBindVertexArray(mesh->vao);

      // Confirm that the shader has everything it needs to operate.
      if (!assertShaderValid(this->shader_id)) {
        // TODO: Throw an exception instead so the environment is cleaned up properly.
        exit(1);
      }

      // Issue a draw task to the GPU
      glDrawArrays(mesh->primitiveType, 0, mesh->primitiveCount);
    }
  }

  // Clean up
  glBindVertexArray(GL_NONE);

  // Copy the draw buffer to the screen
  glfwSwapBuffers(this->window);
}
