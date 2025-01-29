// ref : https://cha930126.tistory.com/41
// 코린이의 즐거운 코딩세상
// HDR 구현 과정 1부

Texture2D HDRTex : register(t0);
StructuredBuffer<float> AverageValues1D : register(t1);
RWStructuredBuffer<float> AverageLumFinal : register(u0);

cbuffer GlobalCBForCS : register(b0)
{
    float2 g_Res;
};

groupshared float SharedPositions[1024];

// 각 스레드에 대해 4x4 다운 스케일을 수행한다. 
float DownScale4x4(uint2 CurPixel, uint groupThreadId)
{
    float4 lumFactor = float4(1, 1, 1, 1);
    float avgLum = 0.0;
    
    if (CurPixel.y < g_Res.y)
    {
        int3 fullResPos = int3(CurPixel * 4, 0);
        float4 downScaled = float4(0, 0, 0, 0);
        
        [unroll]
        for (int i = 0; i < 4; i++)
        {
            [unroll]
            for (int j = 0; j < 4; j++)
            {
                downScaled += HDRTex.Load(fullResPos, int2(j, i));
            }
        }
        downScaled /= 16;
        
        // 픽셀별 휘도값 계산
        avgLum = dot(downScaled, lumFactor);
        
        // 공유 메모리에 결과 기록
        SharedPositions[groupThreadId] = avgLum;
    }
    
    // 동기화 후 다음 단계로
    GroupMemoryBarrierWithGroupSync();
    
    return avgLum;
}

// 위에서 구한 값을 4개의 값으로 다운 스케일 한다.
float DownScale1024to4(uint dispachThreadId, uint groupThreadId,
    float avgLum)
{
    // 다운스케일 코드를 확장
    for (uint iGroupSize = 4, iStep1 = 1, iStep2 = 2, iStep3 = 3;
        iGroupSize < 1024;
        iGroupSize *= 4, iStep1 *= 4, iStep2 *= 4, iStep3 *= 4)
    {
        if (groupThreadId % iGroupSize == 0)
        {
            float fStepAvgLum = avgLum;

            fStepAvgLum += dispachThreadId + iStep1 < g_Domain ?
                SharedPositions[groupThreadId + iStep1] : avgLum;

            fStepAvgLum += dispachThreadId + iStep2 < g_Domain ?
                SharedPositions[groupThreadId + iStep2] : avgLum;

            fStepAvgLum += dispachThreadId + iStep3 < g_Domain ?
              SharedPositions[groupThreadId + iStep3] : avgLum;

            // 결과 값 저장
            avgLum = fStepAvgLum;
            SharedPositions[groupThreadId] = fStepAvgLum;
        }
        // 동기화 후 다음으로
        GroupMemoryBarrierWithGroupSync();
    }
    return avgLum;
}