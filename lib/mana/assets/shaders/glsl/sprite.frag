#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2DArray tex_sampler;

layout(push_constant) uniform SpritePush {
  int frame_layer;
} pc;

layout(location = 0) in vec2 in_tex_coord;

layout(location = 0) out vec4 out_frag_color;
layout(location = 1) out vec4 out_normal_color;

void main() {
  out_frag_color = texture(tex_sampler, vec3(in_tex_coord, float(pc.frame_layer)));
  // Stubbed normals for now
  out_normal_color = vec4(0.0, 0.0, 1.0, 1.0);
}
