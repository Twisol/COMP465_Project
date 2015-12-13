#version 330 core

struct Light {
  vec3 position;
  vec3 color;

  float ambient;  // Approximates scattering for this light
  float attenuation;
  bool enabled;  // Whether the light should be utilized
};

const Light ruberLight = Light(vec3(0, 0, 0), vec3(1, 1, 1), 0.2, 0.000000003, true);
const float ambient = 0.3;  // Global ambient light

in vec3 position;
in vec3 normal;
in vec4 color;
in vec4 emissivity;

layout(location=0) out vec4 fragColor;

vec4 applyLighting(Light light) {
  if (!light.enabled) {
    return vec4(0, 0, 0, 1);
  }

  vec3 to_light = light.position - position;
  float len = length(to_light);
  to_light = normalize(to_light);

  float diffuseFactor = max(0, dot(normal, to_light));
  float ambientFactor = light.ambient;
  float attenuation = 1.0/(1.0 + light.attenuation*len*len);

  return (ambientFactor + diffuseFactor) * attenuation * color * vec4(light.color, 1);
}

void main() {
  vec4 accumulatedColor = vec4(0, 0, 0, 0);
  accumulatedColor += emissivity; // Emissive light for this fragment
  accumulatedColor += applyLighting(ruberLight); // Light from Ruber
  accumulatedColor += ambient*color; // Global illumination

  fragColor = accumulatedColor;
}
