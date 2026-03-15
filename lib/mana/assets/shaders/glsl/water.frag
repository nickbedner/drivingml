#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform WaterFragUBO {
  vec4 params0; // x = time
  vec4 params1; // x = mip bias(fine adjust for resolution scaling)
} wp;

layout(set = 0, binding = 2) uniform sampler2D wave_tex;

layout(location = 0) in vec2 frag_uv;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;

void main() {
  float time = wp.params0.x;
  // TODO: Will likely need to change base on size of quad
  float tiling = 128.0;

  vec2 baseUV = frag_uv * tiling;

  vec2 uv1 = baseUV + vec2(time * 0.05, time * 0.05);
  vec2 uv2 = baseUV + vec2(0.0, time * 0.05);

  // Pass it in from CPU
  float bias = wp.params1.x;

  float s1 = texture(wave_tex, uv1, bias).r;
  float s2 = texture(wave_tex, uv2, bias - 0.75).r;

  float v = s1 + s2 * 2;

  // Adjust overall intensity
  v *= 0.5;

 // Wrap instead of clamp
  v = fract(v);

  // Final alpha test
  if(v > 0.13 && v < 0.92)
    discard;

  out_color = vec4(v, v, v, v);
  out_normal = vec4(0.0);
}