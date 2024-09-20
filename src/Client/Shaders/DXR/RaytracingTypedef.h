#ifndef RAYTRACING_TYPEDEF_HLSL
#define RAYTRACING_TYPEDEF_HLSL

#define UINT unsigned int
#define BOOL int

namespace MaterialType
{
enum Type
{
    Default,
    Matte,  // Lambertian scattering
    Mirror, // Specular reflector that isn't modified by the Fresnel equations.
    AnalyticalCheckerboardTexture
};
}

#endif
