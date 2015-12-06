#version 330 core

uniform samplerCube cube;

in vec3 coord;
layout(location=0) out vec4 color;

void main() {
  color = texture(cube, coord);
}
