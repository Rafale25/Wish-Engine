int packColor(int3 color) {
    return (color.r << 16 | color.g << 8 | color.b);
}

int packColor(float3 color) {
    int3 c = int3(color * 255);
    return int(c.r << 16 | c.g << 8 | c.b);
}

float3 unpackColor(int rgb) {
    return float3(
        (rgb >> 16) & 0xFF,
        (rgb >> 8) & 0xFF,
        (rgb) & 0xFF
    ) / 255.0;
}
