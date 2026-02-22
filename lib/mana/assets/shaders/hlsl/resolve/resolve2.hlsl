cbuffer BlitConstantBuffer : register(b0) {
  float2 screen_size;
};

Texture2D tex_sampler1 : register(t1);
SamplerState sampler_state1 : register(s0);

float4 PS_main(float4 Pos : SV_POSITION) : SV_Target {
  float2 tex_coord = Pos.xy / screen_size;
  
  // Downsampling using a simple box filter
  float offset = 1.0 / (2.0 * screen_size.x); // Half pixel offset for sampling
  float4 color = float4(0.0, 0.0, 0.0, 0.0);
  color += tex_sampler1.Sample(sampler_state1, tex_coord + float2(-offset, -offset));
  color += tex_sampler1.Sample(sampler_state1, tex_coord + float2(offset, -offset));
  color += tex_sampler1.Sample(sampler_state1, tex_coord + float2(-offset, offset));
  color += tex_sampler1.Sample(sampler_state1, tex_coord + float2(offset, offset));

  return color / 4.0; // Average the color
}
