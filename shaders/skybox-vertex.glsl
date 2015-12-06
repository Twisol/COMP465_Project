#version 330 core

uniform mat4 mvpMatrix;

layout(location=0) in vec3 v_position;
out vec3 coord;

void main() {
  coord = v_position;
  gl_Position = mvpMatrix*vec4(v_position, 1);
}
