#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "modelcommon.glsl"

layout(binding = 0) uniform ModelUniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 proj;
  vec3 camera_pos;
} ubo;
layout(binding = 1) uniform Lighting {
  Light light;
} lighting;
layout(binding = 2) uniform sampler2D diffuse_map;
layout(binding = 3) uniform sampler2D normal_map;
layout(binding = 4) uniform sampler2D metallic_map;
layout(binding = 5) uniform sampler2D roughness_map;
layout(binding = 6) uniform sampler2D ao_map;

layout(location = 0) in vec2 in_tex_coord;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;
layout(location = 3) in vec3 in_frag_pos;

layout(location = 0) out vec4 out_frag_color;
layout(location = 1) out vec4 out_normal_color;

const float PI = 3.14159265359;

vec3 getNormalFromMap() {
  vec3 tangentNormal = texture(normal_map, in_tex_coord).xyz * 2.0 - 1.0;

  vec3 Q1 = dFdx(in_frag_pos);
  vec3 Q2 = dFdy(in_frag_pos);
  vec2 st1 = dFdx(in_tex_coord);
  vec2 st2 = dFdy(in_tex_coord);

  vec3 N = normalize(in_normal);
  vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
  vec3 B = -normalize(cross(N, T));
  mat3 TBN = mat3(T, B, N);

  return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float nom = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;

  float nom = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

void main() {
  vec3 albedo = pow(texture(diffuse_map, in_tex_coord).rgb, vec3(2.2));
  float metallic = texture(metallic_map, in_tex_coord).r;
  float roughness = texture(roughness_map, in_tex_coord).r;
  float ao = texture(ao_map, in_tex_coord).r;

  vec3 N = getNormalFromMap();
  vec3 V = normalize(ubo.camera_pos - in_frag_pos);

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  vec3 Lo = vec3(0.0);
  {
    vec3 L = normalize(-lighting.light.direction);
    vec3 H = normalize(V + L);
    vec3 radiance = lighting.light.diffuse_colour;

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);

    Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
  }

  vec3 ambient = vec3(0.03) * albedo * ao;

  vec3 color = ambient + Lo;

  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0 / 2.2));

  out_frag_color = vec4(color, 1.0);
}
