import glsl_operators_overload;

struct VSInput {
    float3 Pos;
};

struct VSOutput {
    float4 Pos : SV_POSITION;
    float3 UVW;
};

struct UBO {
    float4x4 projection;
    float4x4 view;
};

struct ShaderData {
    float4x4 projection;
    float4x4 view;
    float4 viewPosition;
    float4 viewDirection;
    float time;
};

[[vk::binding(0, 0)]] SamplerCube samplerCubemap;
// SamplerCube samplerCubeMap;

[shader("vertex")]
VSOutput main(VSInput input, uniform ImmutablePtr<ShaderData> u) {
    VSOutput output;
    output.UVW = input.Pos;
    // Convert cubemap coordinates into Vulkan coordinate space
    output.UVW.xy *= -1.0;
    // Remove translation from view matrix
    float4x4 viewMat = u.view;
    viewMat[0][3] = 0.0;
    viewMat[1][3] = 0.0;
    viewMat[2][3] = 0.0;
    output.Pos = u.projection * viewMat * float4(input.Pos.xyz, 1.0);
    return output;
}

[shader("fragment")]
float4 main(VSOutput input) {
    return samplerCubeMap.Sample(input.UVW);
}
