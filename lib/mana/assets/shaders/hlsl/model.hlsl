// Include the previously ported modelcommon.hlsl
#include "modelcommon.hlsl"

cbuffer ModelUniformBufferObject : register(b0) {
    matrix model;
    matrix view;
    matrix proj;
    float3 camera_pos;
};

cbuffer ModelAnimationUniformBufferObject : register(b7) {
    matrix joint_transforms[50];
};

struct VertexInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 tex_coord : TEXCOORD0;
    float3 color : COLOR0;
    int3 joints_ids : JOINTS;
    float3 weights : WEIGHTS;
    //float3 frag_pos : POSITION1;
};

struct VertexOutput {
    float4 clip_position : SV_POSITION; // Must be the first and in float4 for clip space
    float3 frag_pos      : POSITION1;   // Using another position semantic for fragment's world pos
    float2 texture_coords: TEXCOORD0;   // Keep consistent ordering with Pixel Shader Input
    float3 normal        : NORMAL0;
    float3 frag_color    : COLOR0;
};

static const int MAX_JOINTS = 50;
static const int MAX_WEIGHTS = 3;

VertexOutput VS_main(VertexInput input) {
    VertexOutput output;
    float4 total_local_pos = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 total_normal = float4(0.0f, 0.0f, 0.0f, 0.0f);

    for(int i = 0; i < MAX_WEIGHTS; i++) {
        matrix joint_transform = joint_transforms[input.joints_ids[i]];
        float4 pose_position = mul(float4(input.position, 1.0f), joint_transform);
        total_local_pos += pose_position * input.weights[i];

        float4 world_normal = mul(float4(input.normal, 0.0f), joint_transform);
        total_normal += world_normal * input.weights[i];
    }

    output.clip_position = mul(mul(mul(total_local_pos, model), view), proj);
    output.texture_coords = input.tex_coord;
    output.normal = total_normal.xyz;
    output.frag_color = input.color;
    output.frag_pos = mul(model, float4(input.position, 1.0f)).xyz;

    return output;
}

cbuffer Lighting : register(b1) {
    Light light;
};

Texture2D diffuse_map : register(t2);
Texture2D normal_map : register(t3);
Texture2D metallic_map : register(t4);
Texture2D roughness_map : register(t5);
Texture2D ao_map : register(t6);
SamplerState samplerState : register(s0);

struct PixelOutput {
    float4 out_frag_color : SV_TARGET0;
    float4 out_normal_color : SV_TARGET1;
    float out_depth : SV_Target2;         // Depth (linear or non-linear, depending on your needs)

};

float3 getNormalFromMap(float2 in_tex_coord, float3 in_frag_pos, float3 in_normal) {
    float3 tangentNormal = normal_map.Sample(samplerState, in_tex_coord).xyz * 2.0f - 1.0f;

    float3 Q1 = ddx(in_frag_pos);
    float3 Q2 = ddy(in_frag_pos);
    float2 st1 = ddx(in_tex_coord);
    float2 st2 = ddy(in_tex_coord);

    float3 N = normalize(in_normal);
    float3 T = normalize(Q1 * st2.y - Q2 * st1.y);
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);

    return normalize(mul(tangentNormal, TBN)); // Note the multiplication order switch
}

float DistributionGGX(float3 N, float3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = 3.14159265359f * denom * denom;  // Replaced PI with its constant value

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;

    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0f - F0) * pow(max(1.0f - cosTheta, 0.0f), 5.0f);
}

PixelOutput PS_main(VertexOutput input) {
    PixelOutput output;

    float3 albedo = pow(diffuse_map.Sample(samplerState, input.texture_coords).rgb, float3(2.2, 2.2, 2.2));
    float metallic = metallic_map.Sample(samplerState, input.texture_coords).r;
    float roughness = roughness_map.Sample(samplerState, input.texture_coords).r;
    float ao = ao_map.Sample(samplerState, input.texture_coords).r;

    float3 N = getNormalFromMap(input.texture_coords, input.frag_pos, input.normal);
    float3 V = normalize(camera_pos - input.frag_pos);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);

    float3 Lo = float3(0.0, 0.0, 0.0);
    {
        float3 L = normalize(light.direction - input.frag_pos);
        float3 H = normalize(V + L);
        float distance = length(light.direction - input.frag_pos);
        float attenuation = 1.0 / (distance * distance);
        float3 radiance = light.diffuse_colour * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float3 numerator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        float3 specular = numerator / denominator;

        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / 3.14159265359 + specular) * radiance * NdotL;
    }

    float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao;
    float3 color = ambient + Lo;

    color = color / (color + float3(1.0, 1.0, 1.0));
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));

    output.out_frag_color = float4(color, 1.0);

    // Writing normal
    // Ensure normal is in [0,1] range for visualization.
    // You can adjust this later if needed.
    output.out_normal_color = float4((N * 0.5 + 0.5), 1.0);

    // Writing depth
    // Assuming frag_pos.z is in view space. 
    // You can adjust this later if needed.
    output.out_depth = input.frag_pos.z;

    return output;
}
