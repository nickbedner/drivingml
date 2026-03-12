#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform WaterVertexUBO {
  mat4 model;
  mat4 view;
  mat4 proj;
} ubo;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_tex_coord;

layout(location = 0) out vec2 frag_uv;
layout(location = 1) out vec3 frag_view_pos;

void main() {
  vec4 worldPos = ubo.model * vec4(in_position, 1.0);
  vec4 viewPos = ubo.view * worldPos;

  frag_view_pos = viewPos.xyz;
  frag_uv = in_tex_coord;

  gl_Position = ubo.proj * viewPos;
}