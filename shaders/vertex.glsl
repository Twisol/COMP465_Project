#version 330 core

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projection;
uniform mat3 normalMatrix;

uniform vec4 v_emissivity;

layout(location=0) in vec3 v_position;
layout(location=1) in vec3 v_normal;
layout(location=2) in vec4 v_color;

out vec3 position;
out vec3 normal;
out vec4 color;
out vec4 emissivity;

void main() {
  position = vec3(worldMatrix * vec4(v_position, 1.0));
  normal = normalize(normalMatrix*v_normal);
  color = v_color;
  emissivity = v_emissivity;

  mat4 MVP = projection * viewMatrix * worldMatrix;
  gl_Position = MVP * vec4(v_position, 1.0);
}
