float4 operator*(float4x4 a, float4 b) {
    return mul(a, b);
}

struct VSInput {
    float3 Pos;
    float3 Color;
    float2 UV;
};

// Sampler2D textures[];

struct ShaderData {
    float4x4 projection;
    float4x4 view;
    float4x4 model[3];
    float4 lightPos;
    uint32_t selected;
    float time;
};

struct VSOutput {
    float4 Pos : SV_POSITION;
    float3 Color;
    float2 UV;
    // uint32_t InstanceIndex;
};

[shader("vertex")]
VSOutput main(VSInput input, uniform ShaderData *u, uint instanceIndex : SV_VulkanInstanceID) {
    VSOutput output;

    output.Pos = float4(input.Pos.xyz, 1.0);
    output.Color = input.Color;
    output.UV = input.UV;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform ShaderData *u) {
    // float2 uv = fract(input.UV * 4);
    // uv.x += u.time;
    // uv.y -= abs(sin(u.time * 3));
    // float3 color = textures[0].Sample(uv).rgb;

    return float4(input.Color, 1.0);
}
