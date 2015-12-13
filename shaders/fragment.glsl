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


uniform Light u_ruberLight;
uniform Light u_globalLight;
uniform Light u_headLight;

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

  vec3 to_light = light.position - position;
  float dist_light = length(to_light);
  to_light = normalize(to_light);

  vec3 to_eye = u_viewPosition - position;
  float dist_eye = length(to_eye);
  to_eye = normalize(to_eye);

  vec3 ambientFactor = light.ambient;
  vec3 diffuseFactor = max(0, dot(normal, to_light)) * light.diffuse;
  vec3 specularFactor = vec3(0, 0, 0);  // TODO: Implement specular lighting!

  float attenuation = 1.0/(1.0 + light.attenuation*dist_light*dist_light);
  return vec4(ambientFactor + diffuseFactor + specularFactor, 1) * attenuation * color;
}

void main() {
  vec4 accumulatedColor = vec4(0, 0, 0, 0);
  accumulatedColor += emissivity; // Emissive light for this fragment
  accumulatedColor += applyLighting(u_ruberLight); // Light from Ruber
  accumulatedColor += applyLighting(u_globalLight); // Global illumination
  accumulatedColor += applyLighting(u_headLight); // Directional illumination

  fragColor = accumulatedColor;
}
