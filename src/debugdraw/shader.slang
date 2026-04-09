float4 operator*(float4x4 a, float4 b) {
    return mul(a, b);
}

float4x4 operator*(float4x4 a, float4x4 b) {
    return mul(a, b);
}

struct VSInput {
    float3 pos;
    int color;
};

struct VSOutput {
    float4 pos : SV_POSITION;
    // nointerpolation int color;
    float3 color;
};

struct ShaderData {
    float4x4 viewProjection;
};

import packing;

[shader("vertex")]
VSOutput main(VSInput input, uniform ImmutablePtr<ShaderData> u) {
    float4 pos = u.viewProjection * float4(input.pos, 1.0);

    VSOutput output;
    output.pos = pos;
    output.color = unpackColor(input.color);//input.color;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input, uniform ImmutablePtr<ShaderData> u) {
    return float4(input.color, 1.0);
    // return float4(unpackColor(input.color), 1.0);

    // FragColor = vec4(unpackColor(v_color), 1.0);
}
