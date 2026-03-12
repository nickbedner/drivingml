#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform WaterFragUBO {
  vec4 params0; // x = time
  vec4 params1; // x = mipBias (fine adjust)
} wp;

layout(set = 0, binding = 2) uniform sampler2D wave_tex;

layout(location = 0) in vec2 frag_uv;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;

void main() {
  float time = wp.params0.x;
  float tiling = 32.0;

  vec2 baseUV = frag_uv * tiling;

  vec2 uv1 = baseUV + vec2(time * 0.05, time * 0.03);
  vec2 uv2 = baseUV + vec2(-time * 0.08, time * 0.02);

  float s1 = textureLod(wave_tex, uv1, 2.0).r;
  float s2 = textureLod(wave_tex, uv2, 2.0).r;

  float v = s1 + s2 * 2.0;

  v /= 2.0;
  v = 1.0 - v;

  out_color = vec4(1.0, 1.0, 1.0, v);
  out_normal = vec4(0.0);
}