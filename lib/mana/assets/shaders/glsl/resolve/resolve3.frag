#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform BlitUniformBufferObject {
  vec2 screen_size;
} ubo;

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) out vec4 out_frag_color;

void main() {
  vec2 tex_coord = gl_FragCoord.xy / ubo.screen_size;

  float offset = 1.0 / (3.0 * ubo.screen_size.x);
  vec4 color = vec4(0.0);
  for(int i = -1; i <= 1; i++) {
    for(int j = -1; j <= 1; j++) {
      color += texture(tex_sampler, tex_coord + vec2(offset * i, offset * j));
    }
  }

  out_frag_color = color / 9.0;
}
