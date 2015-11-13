#version 330 core

uniform mat4 modelview;
uniform mat4 projection;

layout(location=0) in vec3 v_position;
layout(location=1) in vec3 v_normal;
layout(location=2) in vec4 v_color;

out vec3 normal;
out vec4 color;

void main() {
  normal = v_normal;
  color = v_color;

  gl_Position = (projection*modelview)*vec4(v_position, 1.0);
}
