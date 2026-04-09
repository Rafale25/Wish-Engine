import packing;
import glsl_operators_overload;

struct VSInput {
    float3 pos;
    int color;
};

struct VSOutput {
    float4 pos : SV_POSITION;
    nointerpolation int color;
};

struct ShaderData {
    float4x4 viewProjection;
};

[shader("vertex")]
VSOutput main(VSInput input, uniform ImmutablePtr<ShaderData> u) {
    float4 pos = u.viewProjection * float4(input.pos, 1.0);

    VSOutput output;
    output.pos = pos;
    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform ImmutablePtr<ShaderData> u) {
    return float4(unpackColor(input.color), 1.0);
}
