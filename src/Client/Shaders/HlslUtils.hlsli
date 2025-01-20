bool IsBlack(float3 color)
{
    return !any(color);
}