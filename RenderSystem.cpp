#include "RenderSystem.h"
#include "shaders.h"
#include "Texture.h"

// Query result object for interfacing with the EntityDatabase
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

// Computes the view matrix from the world to the given entity.
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

// Computes the model matrix from the given entity to the world.
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
  // Prepare the mainline rendering shader
  this->shader_id = create_program_from_files("shaders/vertex.glsl", "shaders/fragment.glsl");
  if (this->shader_id == GL_NONE) {
    // TODO: Throw an exception instead so the environment is cleaned up properly.
    exit(1);
  }

  // Prepare the skybox rendering shader
  this->skybox_shader_id = create_program_from_files("shaders/skybox-vertex.glsl", "shaders/skybox-fragment.glsl");
  if (this->skybox_shader_id == GL_NONE) {
    // TODO: Throw an exception instead so the environment is cleaned up properly.
    exit(1);
  }

  // Load the box geometry for our skybox to be drawn on
  this->skyboxMesh = loadMeshFromFile("models/skybox.tri");

  {
    // starfield texture management
    static int const CUBE_MAP_DIM = 908; // our .RAW file tiles are 908 pixels on each side
    unsigned char* texData[6]; // array to hold our texture data

    // load our texture data array with six square texture tiles
    texData[0] = loadRawData("images/starfield_1.raw", CUBE_MAP_DIM, CUBE_MAP_DIM);
    texData[1] = loadRawData("images/starfield_2.raw", CUBE_MAP_DIM, CUBE_MAP_DIM);
    texData[2] = loadRawData("images/starfield_3.raw", CUBE_MAP_DIM, CUBE_MAP_DIM);
    texData[3] = loadRawData("images/starfield_4.raw", CUBE_MAP_DIM, CUBE_MAP_DIM);
    texData[4] = loadRawData("images/starfield_5.raw", CUBE_MAP_DIM, CUBE_MAP_DIM);
    texData[5] = loadRawData("images/starfield_6.raw", CUBE_MAP_DIM, CUBE_MAP_DIM);

    // create starfield texture cube map
    this->cubeMap = makeCubeMap(texData, CUBE_MAP_DIM);

    // release our .RAW file temp memory
    for (int i = 0; i < 6; i++) {
      free(texData[i]);
    }
  }
}

// A mimic of the struct defined in our GLSL fragment shader.
// Represents all of the parameters of a light in our lighting model.
struct Light {
  glm::vec3 position;
  glm::vec3 direction;  // (0, 0, 0) means point light; otherwise means directional

  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  // Specular sharpness/power is fixed in the shader

  float attenuation;
  bool enabled;  // Whether the light should be utilized
};

static Light GetGlobalLight(GameState& state) {
  return Light{
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),

    glm::vec3(0.3f, 0.3f, 0.3f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),

    0.0f,
    state.is_lit_global,
  };
}

static Light GetRuberLight(GameState& state) {
  return Light{
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),

    glm::vec3(0.2f, 0.2f, 0.2f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),

    0.000000003f,
    state.is_lit_ruber,
  };
}

static Light GetHeadLight(GameState& state) {
  glm::mat4 const viewMatrix = GetViewMatrix(state.entities, CAMERAS[state.active_camera]);
  glm::mat4 const inverseViewMatrix = glm::inverse(viewMatrix);

  return Light{
    glm::vec3(inverseViewMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)),
    glm::vec3(inverseViewMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)),

    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.4f, 0.4f, 0.4f),
    glm::vec3(0.8f, 0.8f, 0.8f),

    0.0f,
    state.is_lit_headlight,
  };
}

void RenderSystem::Render(GameState& state) {
  // Clear the previous render results
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Compute the cumulative transformation from the world basis to clip space.
  glm::mat4 const viewMatrix = GetViewMatrix(state.entities, CAMERAS[state.active_camera]);

  // Draw the skybox!
  {
    glUseProgram(this->skybox_shader_id);

    glm::mat4 mvpMatrix =
      ( this->projectionMatrix
      * glm::mat4{glm::mat3{viewMatrix}}
      );
    GLint const mvpMatrixLocation = glGetUniformLocation(this->skybox_shader_id, "mvpMatrix");
    glUniformMatrix4fv(mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));

    GLint const cubeLocation = glGetUniformLocation(this->skybox_shader_id, "cube");
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(cubeLocation, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubeMap);

    glDepthMask(GL_FALSE);
    glFrontFace(GL_CW);

    // Issue a draw task to the GPU
    glBindVertexArray(this->skyboxMesh.vao);
    glDrawArrays(this->skyboxMesh.primitiveType, 0, this->skyboxMesh.primitiveCount);

    glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
    glDepthMask(GL_TRUE);
    glFrontFace(GL_CCW);
  }

  // Draw all the other entities
  for (auto entity : state.entities.Query<RenderableEntity>()) {
    // Set up the shader for this instance
    {
      // Use our simple ("100% ambient light") shader.
      glUseProgram(this->shader_id);

      // Configure the render properties of this instance via shader uniforms.
      // Properties specific to each instance may include its position, animation step, etc.

      glm::mat4 const worldMatrix = GetWorldMatrix(state.entities, entity.id);
      GLint const worldMatrixLocation = glGetUniformLocation(this->shader_id, "worldMatrix");
      glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, glm::value_ptr(worldMatrix));

      GLint const normalMatrixLocation = glGetUniformLocation(this->shader_id, "normalMatrix");
      glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat3{glm::inverseTranspose(worldMatrix)}));

      glm::mat4 const mvpMatrix = this->projectionMatrix * viewMatrix * worldMatrix;
      GLint const mvpMatrixLocation = glGetUniformLocation(this->shader_id, "mvpMatrix");
      glUniformMatrix4fv(mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));

      GLint const emissivityLocation = glGetUniformLocation(this->shader_id, "u_emissivity");
      if (entity.id == "Ruber") {
        glUniform4f(emissivityLocation, 0.87f, 0.47f, 0.0f, 1.0f);
      } else {
        glUniform4f(emissivityLocation, 0.0f, 0.0f, 0.0f, 1.0f);
      }

      glm::mat4 inverseViewMatrix = glm::inverse(viewMatrix);
      GLint const viewPositionLocation = glGetUniformLocation(this->shader_id, "u_viewPosition");
      glUniform3fv(viewPositionLocation, 1, glm::value_ptr(glm::vec3{inverseViewMatrix * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}}));

      GLint const viewNormalLocation = glGetUniformLocation(this->shader_id, "u_viewNormal");
      glUniform3fv(viewNormalLocation, 1, glm::value_ptr(glm::vec3{inverseViewMatrix * glm::vec4{0.0f, 0.0f, -1.0f, 0.0f}}));

      { // Specify Ruber light
        Light light = GetRuberLight(state);
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_ruberLight.position"), 1, glm::value_ptr(light.position));
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_ruberLight.direction"), 1, glm::value_ptr(light.direction));
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_ruberLight.ambient"), 1, glm::value_ptr(light.ambient));
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_ruberLight.diffuse"), 1, glm::value_ptr(light.diffuse));
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_ruberLight.specular"), 1, glm::value_ptr(light.specular));

        glUniform1f(glGetUniformLocation(this->shader_id, "u_ruberLight.attenuation"), light.attenuation);
        glUniform1i(glGetUniformLocation(this->shader_id, "u_ruberLight.enabled"), light.enabled);
      }

      { // Specify Global light
        Light light = GetGlobalLight(state);
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_globalLight.position"), 1, glm::value_ptr(light.position));
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_globalLight.direction"), 1, glm::value_ptr(light.direction));
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_globalLight.ambient"), 1, glm::value_ptr(light.ambient));
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_globalLight.diffuse"), 1, glm::value_ptr(light.diffuse));
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_globalLight.specular"), 1, glm::value_ptr(light.specular));

        glUniform1f(glGetUniformLocation(this->shader_id, "u_globalLight.attenuation"), light.attenuation);
        glUniform1i(glGetUniformLocation(this->shader_id, "u_globalLight.enabled"), light.enabled);
      }

      { // Specify Headlight
        Light light = GetHeadLight(state);
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_headLight.position"), 1, glm::value_ptr(light.position));
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_headLight.direction"), 1, glm::value_ptr(light.direction));
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_headLight.ambient"), 1, glm::value_ptr(light.ambient));
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_headLight.diffuse"), 1, glm::value_ptr(light.diffuse));
        glUniform3fv(glGetUniformLocation(this->shader_id, "u_headLight.specular"), 1, glm::value_ptr(light.specular));

        glUniform1f(glGetUniformLocation(this->shader_id, "u_headLight.attenuation"), light.attenuation);
        glUniform1i(glGetUniformLocation(this->shader_id, "u_headLight.enabled"), light.enabled);
      }
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
