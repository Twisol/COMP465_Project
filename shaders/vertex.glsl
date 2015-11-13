#version 330 core

uniform mat4 modelview;
uniform mat4 projection;
uniform mat3 normalMatrix;

layout(location=0) in vec3 v_position;
layout(location=1) in vec3 v_normal;
layout(location=2) in vec4 v_color;

out vec3 normal;
out vec4 color;

void main() {
  normal = normalMatrix*v_normal;
  color = vec4(normalMatrix*vec3(v_color), v_color.a);

  gl_Position = (projection*modelview)*vec4(v_position, 1.0);
}
