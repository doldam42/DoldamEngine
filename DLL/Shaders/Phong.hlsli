#ifndef __PHONG_HLSLI__
#define __PHONG_HLSLI__

#include "Common.hlsli"

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal,
                   float3 toEye, float3 ambient, float3 diffuse, float3 specular, float shininess)
{
    float3 halfway = normalize(toEye + lightVec);
    float hdotn = dot(halfway, normal);
    specular *= pow(max(hdotn, 0.0f), shininess);

    return ambient + (diffuse + specular) * lightStrength;
}

float3 ComputeDirectionalLight(Light L, float3 normal,
                                float3 toEye, float3 ambient, float3 diffuse, float3 specular, float shininess)
{
    float3 lightVec = -L.direction;

    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.radiance * ndotl;

    // Luna DX12 å������ Specular ��꿡��
    // Lambert's law�� ����� lightStrength�� ����մϴ�.
    return BlinnPhong(lightStrength, lightVec, normal, toEye, ambient, diffuse, specular, shininess);
}

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 ComputePointLight(Light L, float3 pos, float3 normal,
                          float3 toEye, float3 ambient, float3 diffuse, float3 specular, float shininess)
{
    float3 lightVec = L.position - pos;

    // ���̵��� �������� ��������� �Ÿ� ���
    float d = length(lightVec);

    // �ʹ� �ָ� ������ ������� ����
    if (d > L.fallOffEnd)
    {
        return float3(0.0, 0.0, 0.0);
    }
    else
    {
        lightVec /= d;

        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.radiance * ndotl;

        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= att;

        return BlinnPhong(lightStrength, lightVec, normal, toEye, ambient, diffuse, specular, shininess);
    }
}

float3 ComputeSpotLight(Light L, float3 pos, float3 normal,
                         float3 toEye, float3 ambient, float3 diffuse, float3 specular, float shininess)
{
    float3 lightVec = L.position - pos;

    // ���̵��� �������� ��������� �Ÿ� ���
    float d = length(lightVec);

    // �ʹ� �ָ� ������ ������� ����
    if (d > L.fallOffEnd)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        lightVec /= d;

        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.radiance * ndotl;

        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= att;

        float spotFactor = pow(max(-dot(lightVec, L.direction), 0.0f), L.spotPower);
        lightStrength *= spotFactor;

        return BlinnPhong(lightStrength, lightVec, normal, toEye, ambient, diffuse, specular, shininess);
    }
    
    // if�� else�� ���� ��� ��� �߻�
    // warning X4000: use of potentially uninitialized variable
}

#endif 