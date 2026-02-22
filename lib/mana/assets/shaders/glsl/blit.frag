#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform BlitUniformBufferObject {
  vec2 screen_size;
} ubo;

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) out vec4 out_frag_color;

void main() {
  vec2 tex_coord = gl_FragCoord.xy / ubo.screen_size;
  out_frag_color = texture(tex_sampler, tex_coord);
}
