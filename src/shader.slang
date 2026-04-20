import glsl_operators_overload;

struct VSInput {
    float3 Pos;
    float3 Color;
    float2 UV;
};

[[vk::binding(0, 0)]] Sampler2D textures[];

struct VSOutput {
    float4 Pos : SV_POSITION;
    float3 Color;
    float2 UV;
    // uint32_t InstanceIndex;
};

struct ShaderData {
    float4x4 projection;
    float4x4 view;
    float4 viewPosition;
    float4 viewDirection;
    float time;
};

// struct PushConstants {
//     ShaderData *u;
// };
// [[vk::push_constant]] uniform PushConstants pushConstant;
// property u : ShaderData {
//     get { return *pushConstant.u; }
// }

[shader("vertex")]
VSOutput main(VSInput input, uniform ImmutablePtr<ShaderData> u, uint instanceIndex : SV_VulkanInstanceID) {
// VSOutput main(VSInput input, uint instanceIndex: SV_VulkanInstanceID) {
    VSOutput output;

    output.Pos = u.projection * u.view * float4(input.Pos.xyz, 1.0);
    // output.Pos = mul(u.projection, mul(u.view, float4(input.Pos.xyz, 1.0)));
    output.Color = input.Color;
    output.UV = input.UV;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform ImmutablePtr<ShaderData> u) {
// float4 main(VSOutput input) {
    // float2 uv = fract(input.UV * 4);
    // uv.x += u.time;
    // uv.y -= abs(sin(u.time * 3));
    float3 color = textures[0].Sample(input.UV).rgb;

    // float3 color = input.Color * (sin(u.time * 3.0) * 0.5 + 0.5);

    return float4(color, 1.0);
}
