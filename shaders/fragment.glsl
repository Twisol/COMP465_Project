#version 330 core

in vec3 normal;
in vec4 color;

layout(location=0) out vec4 fragColor;

void main() {
  fragColor = color;
}
