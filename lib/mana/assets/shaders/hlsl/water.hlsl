cbuffer WaterVertexUBO : register(b0)
{
  float4x4 model;
  float4x4 view;
  float4x4 proj;
};

cbuffer WaterFragUBO : register(b1)
{
  float4 params0; // x = time
  float4 params1; // x = mipBias
};

Texture2D wave_tex : register(t2);
SamplerState wave_sampler : register(s0);

struct VertexInput
{
  float3 in_position : POSITION;
  float2 in_tex_coord : TEXCOORD0;
};

struct PixelInput
{
  float4 position      : SV_POSITION;
  float2 frag_uv       : TEXCOORD0;
  float3 frag_view_pos : TEXCOORD1;
};

struct GBufferOutput
{
  float4 out_color  : SV_Target0;
  float4 out_normal : SV_Target1;
  float  out_depth  : SV_Target2;
};


PixelInput VS_main(VertexInput input)
{
  PixelInput output;

  float4 worldPos = mul(float4(input.in_position, 1.0f), model);
  float4 viewPos  = mul(worldPos, view);

  output.frag_view_pos = viewPos.xyz;
  output.frag_uv = input.in_tex_coord;
  output.position = mul(viewPos, proj);

  return output;
}

GBufferOutput PS_main(PixelInput input)
{
    GBufferOutput output;

    float time = params0.x;
    float tiling = 128.0f;

    float2 baseUV = input.frag_uv * tiling;

    float2 uv1 = baseUV + float2(time * 0.05f, time * 0.05f);
    float2 uv2 = baseUV + float2(0.0f, time * 0.05f);

    float bias = params1.x;

    float s1 = wave_tex.SampleBias(wave_sampler, uv1, bias).r;
    float s2 = wave_tex.SampleBias(wave_sampler, uv2, bias - 0.75f).r;

    float v = s1 + s2 * 2.0f;
    v *= 0.5f;
    v = frac(v);

    if (v > 0.13f && v < 0.92f)
        discard;

    output.out_color  = float4(v, v, v, v);
    output.out_normal = float4(0.0f, 0.0f, 0.0f, 0.0f);

    // match your sprite shader depth encoding
    output.out_depth = input.position.z / input.position.w;

    return output;
}
