float4 VS_main(uint VertexID : SV_VertexID) : SV_POSITION {
  float2 tex_coord = float2((VertexID << 1) & 2, VertexID & 2);
  return float4(tex_coord * 2.0f - 1.0f, 0.0f, 1.0f);
}
