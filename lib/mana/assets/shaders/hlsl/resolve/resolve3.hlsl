cbuffer BlitConstantBuffer : register(b0) {
    float2 screen_size;
};

Texture2D tex_sampler1 : register(t1);
SamplerState sampler_state1 : register(s0);

float4 PS_main(float4 Pos : SV_POSITION) : SV_Target {
    float2 tex_coord = Pos.xy / screen_size;
    float offset = 1.0 / (3.0 * screen_size.x); // Adjusted to match GLSL
    float4 color = float4(0.0, 0.0, 0.0, 0.0);

    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            color += tex_sampler1.Sample(sampler_state1, tex_coord + float2(offset * i, offset * j));
        }
    }

    return color / 9.0; // Average the color
}
