float3 toLinearSRGB(float3 sRGB) {
    float3 cutoff = float3(sRGB.lessThan(float3(0.04045)));
    float3 higher = pow((sRGB + float3(0.055))/float3(1.055), float3(2.4));
    float3 lower = sRGB/float3(12.92);
    return lerp(higher, lower, cutoff);
}

float3 fromLinearToSRGB(float3 linearRGB) {
    float3 cutoff = float3(linearRGB.lessThan(float3(0.0031308)));
    float3 higher = float3(1.055)*pow(linearRGB, float3(1.0/2.4)) - float3(0.055);
    float3 lower = linearRGB * float3(12.92);
    return lerp(higher, lower, cutoff);
}
