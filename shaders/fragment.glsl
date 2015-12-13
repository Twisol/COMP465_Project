#version 330 core

// Represents all of the parameters for a single light source
struct Light {
  vec3 position;
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float attenuation;
  bool enabled;  // Whether the light should be utilized
};

// The three light sources we process in this shader
uniform Light u_ruberLight;
uniform Light u_globalLight;
uniform Light u_headLight;

// The position of the viewer in world space
uniform vec3 u_viewPosition;
uniform vec3 u_viewNormal;

// The position/normal of the fragment in world space
in vec3 position;
in vec3 normal;
// The material properties of the fragment
in vec4 color;
uniform vec4 u_emissivity;

layout(location=0) out vec4 fragColor;

// Applies a point/directional light to the current fragment.
vec4 applyLighting(Light light) {
  if (!light.enabled) {
    return vec4(0, 0, 0, 1);
  }

  vec3 to_light;
  if (length(light.direction) == 0) {
    to_light = light.position - position;
  } else {
    to_light = -light.direction;
  }

  float dist_light = length(to_light);
  to_light = normalize(to_light);

  vec3 to_eye = u_viewPosition - position;
  float dist_eye = length(to_eye);
  to_eye = normalize(to_eye);

  vec3 ambientFactor = light.ambient;
  vec3 diffuseFactor = vec3(0, 0, 0);
  vec3 specularFactor = vec3(0, 0, 0);
  if (dot(light.direction, u_viewPosition - light.position) <= 0) {
    diffuseFactor = light.diffuse * max(0, dot(normal, to_light));
    specularFactor = light.specular * pow(max(0, dot(to_eye, reflect(-to_light, normal))), 16);
  }

  float attenuation = 1.0/(1.0 + light.attenuation*dist_light*dist_light);
  return vec4(ambientFactor + diffuseFactor + specularFactor, 1) * attenuation * color;
}

void main() {
  vec4 accumulatedColor = vec4(0, 0, 0, 0);
  accumulatedColor += u_emissivity; // Emissive light for this fragment
  accumulatedColor += applyLighting(u_ruberLight); // Light from Ruber
  accumulatedColor += applyLighting(u_globalLight); // Global illumination
  accumulatedColor += applyLighting(u_headLight); // Directional illumination

  fragColor = accumulatedColor;
}
