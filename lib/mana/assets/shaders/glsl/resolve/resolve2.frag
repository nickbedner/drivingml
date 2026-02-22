#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform BlitUniformBufferObject {
  vec2 screen_size;
} ubo;

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) out vec4 out_frag_color;

void main() {
  vec2 tex_coord = gl_FragCoord.xy / ubo.screen_size;

  // Downsampling using a simple box filter
  float offset = 1.0 / (2.0 * ubo.screen_size.x); // Half pixel offset for sampling
  vec4 color = vec4(0.0);
  color += texture(tex_sampler, tex_coord + vec2(-offset, -offset));
  color += texture(tex_sampler, tex_coord + vec2(offset, -offset));
  color += texture(tex_sampler, tex_coord + vec2(-offset, offset));
  color += texture(tex_sampler, tex_coord + vec2(offset, offset));

  out_frag_color = color / 4.0; // Average the color
}
