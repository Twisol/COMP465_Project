#version 330 core

struct Light {
  vec3 position;
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float attenuation;
  bool enabled;  // Whether the light should be utilized
};

const Light ruberLight = Light(
  vec3(0, 0, 0),
  vec3(0, 0, 0),

  vec3(0.2, 0.2, 0.2),
  vec3(1, 1, 1),
  vec3(0, 0, 0),

  0.000000003,
  true
);
const float ambient = 0.3;  // Global ambient light

uniform vec3 u_viewPosition;
uniform vec3 u_viewNormal;

in vec3 position;
in vec3 normal;
in vec4 color;
in vec4 emissivity;

layout(location=0) out vec4 fragColor;

vec4 applyLighting(Light light) {
  if (!light.enabled) {
    return vec4(0, 0, 0, 1);
  }

  vec3 to_light = vec3(light.position) - position;
  float len = length(to_light);
  to_light = normalize(to_light);

  vec3 diffuseFactor = max(0, dot(normal, to_light)) * light.diffuse;
  vec3 ambientFactor = light.ambient;
  vec3 specularFactor = vec3(0, 0, 0);  // TODO: Implement specular lighting!

  float attenuation = 1.0/(1.0 + light.attenuation*len*len);
  return vec4(ambientFactor + diffuseFactor, 1) * attenuation * color;
}

void main() {
  vec4 accumulatedColor = vec4(0, 0, 0, 0);
  accumulatedColor += emissivity; // Emissive light for this fragment
  accumulatedColor += applyLighting(ruberLight); // Light from Ruber
  accumulatedColor += ambient*color; // Global illumination

  fragColor = accumulatedColor;
}
