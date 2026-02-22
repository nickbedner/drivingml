cbuffer ModelStaticUniformBufferObject : register(b0)
{
  float4x4 model;
  float4x4 view;
  float4x4 proj;
  float3 camera_pos;
  float3 padding; // For 16-byte alignment
};

struct VertexInput
{
  float3 in_position : POSITION;
  float3 in_normal : NORMAL;
  float2 in_tex_coord : TEXCOORD0;
  float3 in_color : COLOR;
};

struct VertexOutput {
  float4 clip_position : SV_POSITION; // Must be the first and in float4 for clip space
  float3 frag_pos      : POSITION1;   // Using another position semantic for fragment's world pos
  float2 texture_coords: TEXCOORD0;   // Keep consistent ordering with Pixel Shader Input
  float3 normal        : NORMAL0;
  float3 frag_color    : COLOR0;
};

VertexOutput VS_main(VertexInput input)
{
  VertexOutput output;

  output.clip_position = mul(mul(mul(float4(input.in_position, 1.0f), model), view), proj);
  output.normal = input.in_normal;
  output.texture_coords = input.in_tex_coord;
  output.frag_color = input.in_color;
  output.frag_pos = mul(float4(input.in_position, 1.0f), model).xyz;

  return output;
}
