#version 450
#extension GL_ARB_separate_shader_objects : enable

struct Light {
  vec4 direction; // w = rim light intensity
  vec4 ambient_color;
  vec4 diffuse_color;
  vec4 specular_color;
};

layout(binding = 0) uniform model_uniform_buffer_object {
  mat4 model;
  mat4 view;
  mat4 proj;
  vec3 camera_pos;
} ubo;

layout(binding = 1) uniform lighting_uniform {
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

const float pi = 3.14159265359;

vec3 get_normal_from_map() {
  vec3 tangent_normal = texture(normal_map, in_tex_coord).xyz * 2.0 - 1.0;

  vec3 q1 = dFdx(in_frag_pos);
  vec3 q2 = dFdy(in_frag_pos);
  vec2 st1 = dFdx(in_tex_coord);
  vec2 st2 = dFdy(in_tex_coord);

  vec3 n = normalize(in_normal);
  vec3 t = normalize(q1 * st2.t - q2 * st1.t);
  vec3 b = normalize(q2 * st1.s - q1 * st2.s);

  mat3 tbn = mat3(t, b, n);
  return normalize(tbn * tangent_normal);
}

float distribution_ggx(vec3 n, vec3 h, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float n_dot_h = max(dot(n, h), 0.0);
  float n_dot_h2 = n_dot_h * n_dot_h;

  float denom = (n_dot_h2 * (a2 - 1.0) + 1.0);
  return a2 / max(pi * denom * denom, 0.0001);
}

float geometry_schlick_ggx(float n_dot_v, float roughness) {
  float r = roughness + 1.0;
  float k = (r * r) / 8.0;
  return n_dot_v / max(n_dot_v * (1.0 - k) + k, 0.0001);
}

float geometry_smith(vec3 n, vec3 v, vec3 l, float roughness) {
  float n_dot_v = max(dot(n, v), 0.0);
  float n_dot_l = max(dot(n, l), 0.0);
  float ggx_v = geometry_schlick_ggx(n_dot_v, roughness);
  float ggx_l = geometry_schlick_ggx(n_dot_l, roughness);
  return ggx_v * ggx_l;
}

vec3 fresnel_schlick(float cos_theta, vec3 f0) {
  return f0 + (1.0 - f0) * pow(1.0 - cos_theta, 5.0);
}

void main() {
  vec3 albedo = pow(texture(diffuse_map, in_tex_coord).rgb, vec3(2.2));
  float metallic = clamp(texture(metallic_map, in_tex_coord).r, 0.0, 1.0);
  float roughness = clamp(texture(roughness_map, in_tex_coord).r, 0.045, 1.0);
  float ao = clamp(texture(ao_map, in_tex_coord).r, 0.0, 1.0);

  vec3 n = get_normal_from_map();
  vec3 v = normalize(ubo.camera_pos - in_frag_pos);

  // If light.direction.xyz is the direction sunlight travels,
  // then fragment-to-light is the opposite.
  vec3 l = normalize(-lighting.light.direction.xyz);
  vec3 h = normalize(v + l);

  float n_dot_l = max(dot(n, l), 0.0);
  float n_dot_v = max(dot(n, v), 0.0);

  vec3 f0 = mix(vec3(0.04), albedo, metallic);

  float ndf = distribution_ggx(n, h, roughness);
  float g = geometry_smith(n, v, l, roughness);
  vec3 f = fresnel_schlick(max(dot(h, v), 0.0), f0);

  vec3 k_s = f;
  vec3 k_d = (vec3(1.0) - k_s) * (1.0 - metallic);

  vec3 numerator = ndf * g * f;
  float denominator = max(4.0 * n_dot_v * n_dot_l, 0.0001);
  vec3 specular = numerator / denominator;

  vec3 diffuse_radiance = lighting.light.diffuse_color.rgb;
  vec3 specular_radiance = lighting.light.specular_color.rgb;

  vec3 diffuse_term = (k_d * albedo / pi) * diffuse_radiance * n_dot_l;
  vec3 specular_term = specular * specular_radiance * n_dot_l;
  vec3 ambient = lighting.light.ambient_color.rgb * albedo * ao;

  vec3 color = ambient + diffuse_term + specular_term;

  float rim_strength = max(lighting.light.direction.w, 0.0);

  float view_rim = 1.0 - max(dot(n, v), 0.0);
  view_rim = smoothstep(0.45, 0.85, view_rim);

  float shadow_side = 1.0 - max(dot(n, l), 0.0);
  shadow_side = smoothstep(0.0, 0.6, shadow_side);

  float back_lit = max(dot(-v, l), 0.0);
  back_lit = smoothstep(0.0, 0.5, back_lit);

  float rim = view_rim * shadow_side * back_lit * rim_strength;

  vec3 rim_color = mix(lighting.light.diffuse_color.rgb, lighting.light.specular_color.rgb, 0.5);
  color += rim * rim_color;

  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0 / 2.2));

  out_frag_color = vec4(color, 1.0);
  out_normal_color = vec4(normalize(n) * 0.5 + 0.5, 1.0);
}
