// ref : https://cha930126.tistory.com/41
// �ڸ����� ��ſ� �ڵ�����
// HDR ���� ���� 1��

Texture2D HDRTex : register(t0);
StructuredBuffer<float> AverageValues1D : register(t1);
RWStructuredBuffer<float> AverageLumFinal : register(u0);

cbuffer GlobalCBForCS : register(b0)
{
    float2 g_Res;
};

groupshared float SharedPositions[1024];

// �� �����忡 ���� 4x4 �ٿ� �������� �����Ѵ�. 
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
        
        // �ȼ��� �ֵ��� ���
        avgLum = dot(downScaled, lumFactor);
        
        // ���� �޸𸮿� ��� ���
        SharedPositions[groupThreadId] = avgLum;
    }
    
    // ����ȭ �� ���� �ܰ��
    GroupMemoryBarrierWithGroupSync();
    
    return avgLum;
}

// ������ ���� ���� 4���� ������ �ٿ� ������ �Ѵ�.
float DownScale1024to4(uint dispachThreadId, uint groupThreadId,
    float avgLum)
{
    // �ٿ���� �ڵ带 Ȯ��
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

            // ��� �� ����
            avgLum = fStepAvgLum;
            SharedPositions[groupThreadId] = fStepAvgLum;
        }
        // ����ȭ �� ��������
        GroupMemoryBarrierWithGroupSync();
    }
    return avgLum;
}