cbuffer BlitConstantBuffer : register(b0) {
  float2 screen_size;
};

Texture2D tex_sampler1 : register(t1);
SamplerState sampler_state1 : register(s0);

float4 PS_main(float4 Pos : SV_POSITION) : SV_Target {
  float2 tex_coord = Pos.xy / screen_size;
  return tex_sampler1.Sample(sampler_state1, tex_coord);
}
