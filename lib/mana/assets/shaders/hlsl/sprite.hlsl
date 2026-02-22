cbuffer SpriteUniformBufferObject : register(b0)
{
  float4x4 model;
  float4x4 view;
  float4x4 proj;
}

struct VertexInput
{
  float3 in_position : POSITION;
  float2 in_tex_coord : TEXCOORD0;
};

struct PixelInput
{
  float4 position : SV_POSITION;
  float2 tex_coord : TEXCOORD0;
};

struct GBufferOutput
{
    float4 albedo : SV_Target0;       // Color of the pixel
    float4 normal : SV_Target1;       // Normal in view space
    float depth : SV_Target2;         // Depth (linear or non-linear, depending on your needs)
};

// Vertex Shader
PixelInput VS_main(VertexInput input)
{
  PixelInput output;
  output.position = mul(mul(mul(float4(input.in_position, 1.0f), model), view), proj);
  output.tex_coord = input.in_tex_coord;
  return output;
}

Texture2D tex_sampler1 : register(t1);
SamplerState sampler_state1 : register(s0);

// Pixel Shader
GBufferOutput PS_main(PixelInput input) : SV_Target
{
  //return tex_sampler1.Sample(sampler_state1, input.tex_coord);
  GBufferOutput output;

  // Sample the base color
  output.albedo = tex_sampler1.Sample(sampler_state1, input.tex_coord);
  
  // Stubbing out normals for now
  output.normal = float4(0.0f, 0.0f, 1.0f, 1.0f);  // Placeholder, pointing straight out of the screen in view space

  // For depth: depending on whether you want linear depth, you could extract Z/W from the position.
  // This is a non-linear depth value. If you want linear depth, you'd have to calculate it differently.
  output.depth = input.position.z / input.position.w;

  return output;
}
