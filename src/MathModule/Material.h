#pragma once

struct Material
{
    WCHAR basePath[MAX_PATH] = {0};
    WCHAR name[MAX_NAME] = {0};
    WCHAR albedoTextureName[MAX_NAME] = {0};
    WCHAR emissiveTextureName[MAX_NAME] = {0};
    WCHAR normalTextureName[MAX_NAME] = {0};
    WCHAR aoTextureName[MAX_NAME] = {0};
    WCHAR metallicTextureName[MAX_NAME] = {0};
    WCHAR roughnessTextureName[MAX_NAME] = {0};
    WCHAR heightTextureName[MAX_NAME] = {0};

    Vector3 albedo = Vector3(1.0f);
    Vector3 emissive = Vector3(0.0f);

    float roughnessFactor = 1.f;
    float metallicFactor = 1.f;
    float opacityFactor = 1.0f;
    float reflectionFactor = 0.0f;
};
