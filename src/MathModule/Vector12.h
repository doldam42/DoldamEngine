#pragma once
struct Vector12
{
    static const Vector12 Zero;
    static const Vector12 One;

    inline Vector12() = default;
    inline Vector12(const Vector12 &V);
    inline Vector12(const float *V);
    inline Vector12(const float v);
    inline Vector12(const float v1, const float v2, const float v3, const float v4, const float v5, const float v6,
                    const float v7, const float v8, const float v9, const float v10, const float v11, const float v12);

    inline Vector12 &operator=(const Vector12 &V);

    inline bool operator==(const Vector12 &V) const;

    // Assignment operators
    inline Vector12 &operator+=(const Vector12 &V) noexcept;
    inline Vector12 &operator-=(const Vector12 &V) noexcept;
    inline Vector12 &operator*=(const Vector12 &V) noexcept;
    inline Vector12 &operator*=(float S) noexcept;
    inline Vector12 &operator/=(float S) noexcept;

    inline Vector12 operator+() const noexcept { return *this; }
    inline Vector12 operator-() const noexcept;

    inline Vector12 operator+(const Vector12 &V) const noexcept;
    inline Vector12 operator-(const Vector12 &V) const noexcept;
    inline Vector12 operator*(const Vector12& V) const noexcept;
    inline Vector12 operator/(const Vector12 &V) const noexcept;

    inline float operator[](size_t idx) const noexcept;
    inline float& operator[](size_t idx) noexcept;

    float Dot(const Vector12 &V) const;

    float Data[12] = {0.0f};
};

inline Vector12::Vector12(const Vector12 &V) { memcpy_s(Data, sizeof(Data), V.Data, sizeof(V.Data)); }
inline Vector12::Vector12(const float *V) { memcpy_s(Data, sizeof(Data), V, sizeof(Data)); }

inline Vector12::Vector12(const float v)
{ 
    for (size_t i = 0; i < 12; i++)
    {
        Data[i] = v;
    }
}

inline Vector12::Vector12(const float v1, const float v2, const float v3, const float v4, const float v5,
                          const float v6, const float v7, const float v8, const float v9, const float v10,
                          const float v11, const float v12)
{
    Data[0] = v1;
    Data[1] = v2;
    Data[2] = v3;
    Data[3] = v4;
    Data[4] = v5;
    Data[5] = v6;
    Data[6] = v7;
    Data[7] = v8;
    Data[8] = v9;
    Data[9] = v10;
    Data[10] = v11;
    Data[11] = v12;
}

inline Vector12 &Vector12 ::operator=(const Vector12 &V)
{ 
    memcpy_s(Data, sizeof(Data), V.Data, sizeof(V.Data));
    return *this;
}

inline bool Vector12::operator==(const Vector12 &V) const { return memcmp(Data, V.Data, sizeof(Data)) == 0; }

inline Vector12 &Vector12::operator+=(const Vector12 &V) noexcept 
{
    __m128 l1 = _mm_load_ps(Data);
    __m128 l2 = _mm_load_ps(Data + 4);
    __m128 l3 = _mm_load_ps(Data + 8);

    __m128 r1 = _mm_load_ps(V.Data);
    __m128 r2 = _mm_load_ps(V.Data + 4);
    __m128 r3 = _mm_load_ps(V.Data + 8);

    __m128 s1 = _mm_add_ps(l1, r1);
    __m128 s2 = _mm_add_ps(l2, r2);
    __m128 s3 = _mm_add_ps(l3, r3);

    _mm_store_ps(Data, s1);
    _mm_store_ps(Data + 4, s2);
    _mm_store_ps(Data + 8, s3);

    return *this;
}

inline Vector12 &Vector12::operator-=(const Vector12 &V) noexcept
{
    __m128 l1 = _mm_load_ps(Data);
    __m128 l2 = _mm_load_ps(Data + 4);
    __m128 l3 = _mm_load_ps(Data + 8);

    __m128 r1 = _mm_load_ps(V.Data);
    __m128 r2 = _mm_load_ps(V.Data + 4);
    __m128 r3 = _mm_load_ps(V.Data + 8);

    __m128 s1 = _mm_sub_ps(l1, r1);
    __m128 s2 = _mm_sub_ps(l2, r2);
    __m128 s3 = _mm_sub_ps(l3, r3);

    _mm_store_ps(Data, s1);
    _mm_store_ps(Data + 4, s2);
    _mm_store_ps(Data + 8, s3);

    return *this;
}

inline Vector12 &Vector12::operator*=(const Vector12 &V) noexcept
{
    __m128 l1 = _mm_load_ps(Data);
    __m128 l2 = _mm_load_ps(Data + 4);
    __m128 l3 = _mm_load_ps(Data + 8);

    __m128 r1 = _mm_load_ps(V.Data);
    __m128 r2 = _mm_load_ps(V.Data + 4);
    __m128 r3 = _mm_load_ps(V.Data + 8);

    __m128 s1 = _mm_mul_ps(l1, r1);
    __m128 s2 = _mm_mul_ps(l2, r2);
    __m128 s3 = _mm_mul_ps(l3, r3);

    _mm_store_ps(Data, s1);
    _mm_store_ps(Data + 4, s2);
    _mm_store_ps(Data + 8, s3);

    return *this;
}

inline Vector12 &Vector12::operator*=(float S) noexcept
{
    __m128 l1 = _mm_load_ps(Data);
    __m128 l2 = _mm_load_ps(Data + 4);
    __m128 l3 = _mm_load_ps(Data + 8);

    __m128 r1 = _mm_set_ps1(S);
    __m128 r2 = _mm_set_ps1(S);
    __m128 r3 = _mm_set_ps1(S);

    __m128 s1 = _mm_mul_ps(l1, r1);
    __m128 s2 = _mm_mul_ps(l2, r2);
    __m128 s3 = _mm_mul_ps(l3, r3);

    _mm_store_ps(Data, s1);
    _mm_store_ps(Data + 4, s2);
    _mm_store_ps(Data + 8, s3);

    return *this;
}

inline Vector12 &Vector12::operator/=(float S) noexcept
{
    __m128 l1 = _mm_load_ps(Data);
    __m128 l2 = _mm_load_ps(Data + 4);
    __m128 l3 = _mm_load_ps(Data + 8);

    __m128 r1 = _mm_set_ps1(S);
    __m128 r2 = _mm_set_ps1(S);
    __m128 r3 = _mm_set_ps1(S);

    __m128 s1 = _mm_div_ps(l1, r1);
    __m128 s2 = _mm_div_ps(l2, r2);
    __m128 s3 = _mm_div_ps(l3, r3);

    _mm_store_ps(Data, s1);
    _mm_store_ps(Data + 4, s2);
    _mm_store_ps(Data + 8, s3);

    return *this;
}

inline Vector12 Vector12::operator-() const noexcept 
{
    const static __m128 sign_mask = _mm_set_ps1(-0.0f);

    __m128 neg_v1 = _mm_xor_ps(_mm_load_ps(Data), sign_mask);
    __m128 neg_v2 = _mm_xor_ps(_mm_load_ps(Data + 4), sign_mask);
    __m128 neg_v3 = _mm_xor_ps(_mm_load_ps(Data + 8), sign_mask);

    float data[12];
    _mm_store_ps(data, neg_v1);
    _mm_store_ps(data + 4, neg_v2);
    _mm_store_ps(data + 8, neg_v3);

    return Vector12(data);
}

inline Vector12 Vector12::operator+(const Vector12 &V) const noexcept 
{
    __m128 l1 = _mm_load_ps(Data);
    __m128 l2 = _mm_load_ps(Data + 4);
    __m128 l3 = _mm_load_ps(Data + 8);

    __m128 r1 = _mm_load_ps(V.Data);
    __m128 r2 = _mm_load_ps(V.Data + 4);
    __m128 r3 = _mm_load_ps(V.Data + 8);

    __m128 s1 = _mm_add_ps(l1, r1);
    __m128 s2 = _mm_add_ps(l2, r2);
    __m128 s3 = _mm_add_ps(l3, r3);

    float data[12];
    _mm_store_ps(data, s1);
    _mm_store_ps(data + 4, s2);
    _mm_store_ps(data + 8, s3);

    return Vector12(data);
}

inline Vector12 Vector12::operator-(const Vector12 &V) const noexcept
{
    __m128 l1 = _mm_load_ps(Data);
    __m128 l2 = _mm_load_ps(Data + 4);
    __m128 l3 = _mm_load_ps(Data + 8);

    __m128 r1 = _mm_load_ps(V.Data);
    __m128 r2 = _mm_load_ps(V.Data + 4);
    __m128 r3 = _mm_load_ps(V.Data + 8);

    __m128 s1 = _mm_sub_ps(l1, r1);
    __m128 s2 = _mm_sub_ps(l2, r2);
    __m128 s3 = _mm_sub_ps(l3, r3);

    float data[12];
    _mm_store_ps(data, s1);
    _mm_store_ps(data + 4, s2);
    _mm_store_ps(data + 8, s3);

    return Vector12(data);
}

inline Vector12 Vector12::operator*(const Vector12 &V) const noexcept
{
    __m128 l1 = _mm_load_ps(Data);
    __m128 l2 = _mm_load_ps(Data + 4);
    __m128 l3 = _mm_load_ps(Data + 8);

    __m128 r1 = _mm_load_ps(V.Data);
    __m128 r2 = _mm_load_ps(V.Data + 4);
    __m128 r3 = _mm_load_ps(V.Data + 8);

    __m128 s1 = _mm_mul_ps(l1, r1);
    __m128 s2 = _mm_mul_ps(l2, r2);
    __m128 s3 = _mm_mul_ps(l3, r3);

    float data[12];
    _mm_store_ps(data, s1);
    _mm_store_ps(data + 4, s2);
    _mm_store_ps(data + 8, s3);

    return Vector12(data);
}

inline Vector12 Vector12::operator/(const Vector12 &V) const noexcept
{
    __m128 l1 = _mm_load_ps(Data);
    __m128 l2 = _mm_load_ps(Data + 4);
    __m128 l3 = _mm_load_ps(Data + 8);

    __m128 r1 = _mm_load_ps(V.Data);
    __m128 r2 = _mm_load_ps(V.Data + 4);
    __m128 r3 = _mm_load_ps(V.Data + 8);

    __m128 s1 = _mm_div_ps(l1, r1);
    __m128 s2 = _mm_div_ps(l2, r2);
    __m128 s3 = _mm_div_ps(l3, r3);

    float data[12];
    _mm_store_ps(data, s1);
    _mm_store_ps(data + 4, s2);
    _mm_store_ps(data + 8, s3);

    return Vector12(data);
}

inline float Vector12::operator[](size_t idx) const noexcept
{
    assert(idx < 12);
    return Data[idx];
}

inline float &Vector12::operator[](size_t idx) noexcept
{
    assert(idx < 12);
    return Data[idx];
}
