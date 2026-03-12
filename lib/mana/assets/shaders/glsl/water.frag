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
  float tiling = 128.0;

  vec2 baseUV = frag_uv * tiling;

  vec2 uv1 = baseUV + vec2(time * 0.05, time * 0.05);
  vec2 uv2 = baseUV + vec2(0.0, time * 0.05);

  //float s1 = texture(wave_tex, uv1, 0.0).r;
  //float s2 = texture(wave_tex, uv2, -0.75).r;
//
  //float v = s1 + s2 * 2.0;
//
  //v /= 3.0;
  ////v = 1.0 - v;

//////////////////////////
//  float s1 = textureLod(wave_tex, uv1, 2.0).r;
//  s1 *= s1;
//  float s2 = textureLod(wave_tex, uv2, 2.0).r;
//  s2 *= s2;
//  float v = s1 + (s2 * (1.0 - s1)) * 1.33333; // blend the two layers together, giving more weight to the second layer where the first layer is weaker
// //float v = s1 + s2 * 2.0;
// //v /= 2.0;
//  v = mod(v, 1.0);   // wrap instead of clamp
//
//  //out_color = vec4(1.0, 1.0, 1.0, v);
//  out_color = vec4(v, v, v, 1.0);
//  out_normal = vec4(0.0);
//
///////////////////////

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