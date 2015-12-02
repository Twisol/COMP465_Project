#version 330 core

struct Light {
  vec3 position;
  vec3 color;

  float ambient;
};

const Light ruberLight = Light(vec3(0, 0, 0), vec3(1, 1, 1), 0.3);

in vec3 position;
in vec3 normal;
in vec4 color;
in vec4 emissivity;

layout(location=0) out vec4 fragColor;

vec4 applyLighting(Light light) {
  vec3 to_light = light.position - position;
  float len = length(to_light);
  to_light = normalize(to_light);

  float diffuseFactor = max(0, dot(normal, to_light));
  float ambientFactor = light.ambient;

  return (ambientFactor + diffuseFactor) * color * vec4(light.color, 1);
}

void main() {
  fragColor = emissivity + applyLighting(ruberLight);
}
