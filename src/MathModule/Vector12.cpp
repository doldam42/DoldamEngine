#include "pch.h"
#include "Vector12.h"

const Vector12 Vector12::Zero = Vector12(0.0f);
const Vector12 Vector12::One = Vector12(1.0f);

float Vector12::Dot(const Vector12& V) const
{
    __m128 l1 = _mm_load_ps(Data);
    __m128 l2 = _mm_load_ps(Data + 4);
    __m128 l3 = _mm_load_ps(Data + 8);

    __m128 r1 = _mm_load_ps(V.Data);
    __m128 r2 = _mm_load_ps(V.Data + 4);
    __m128 r3 = _mm_load_ps(V.Data + 8);

    __m128 s1 = _mm_dp_ps(l1, r1, 0xFF);
    __m128 s2 = _mm_dp_ps(l2, r2, 0xFF);
    __m128 s3 = _mm_dp_ps(l3, r3, 0xFF);

    __m128 r = _mm_add_ps(s1, s2);
    r = _mm_add_ps(r, s3);

    float result;
    _mm_store_ss(&result, r);

    return result;
}