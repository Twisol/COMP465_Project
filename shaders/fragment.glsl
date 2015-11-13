#version 330 core

in vec3 position;
in vec3 normal;
in vec4 color;

layout(location=0) out vec4 fragColor;

void main() {
  float percentLit = clamp(dot(normal, -normalize(position)), 0, 1);

  fragColor = vec4(vec3(color) * percentLit, color.w);
}
