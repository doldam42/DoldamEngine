#pragma once
#include "Vector12.h"

struct Matrix12
{
    static const Matrix12 Identity;
    static const Matrix12 Zero;

    Vector12 Rows[12];

    void Transpose();

    inline const Matrix12 &operator=(const Matrix12 &rhs);

    inline void operator*=(float rhs);
    inline Vector12 &operator[](size_t idx) noexcept;

    static void Mul(const Matrix12 &m, const Vector12 &v, Vector12 *result);
    static void Mul(const Matrix12 &m1, const Matrix12 &m2, Matrix12 *result);

    Matrix12() { *this = Identity; }
    Matrix12(float rhs);
    Matrix12(const Vector12 &r1, const Vector12 &r2, const Vector12 &r3, const Vector12 &r4, const Vector12 &r5,
             const Vector12 &r6, const Vector12 &r7, const Vector12 &r8, const Vector12 &r9, const Vector12 &r10,
             const Vector12 &r11, const Vector12 &r12);
};

inline const Matrix12 &Matrix12::operator=(const Matrix12 &rhs)
{
    for (size_t i = 0; i < 12; i++)
    {
        Rows[i] = rhs.Rows[i];
    }
    return *this;
}

inline void Matrix12::operator*=(float rhs) 
{
    for (size_t i = 0; i < 12; i++)
    {
        Rows[i] *= rhs;
    }
}

inline Vector12 &Matrix12::operator[](size_t idx) noexcept
{
    assert(idx < 12);
    return Rows[idx];
}

Matrix12::Matrix12(float rhs)
{
    for (size_t i = 0; i < 12; i++)
    {
        Rows[i] = Vector12(rhs);
    }
}

inline Matrix12::Matrix12(const Vector12 &r1, const Vector12 &r2, const Vector12 &r3, const Vector12 &r4,
                          const Vector12 &r5, const Vector12 &r6, const Vector12 &r7, const Vector12 &r8,
                          const Vector12 &r9, const Vector12 &r10, const Vector12 &r11, const Vector12 &r12)
{
    Rows[0] = r1;
    Rows[1] = r2;
    Rows[2] = r3;
    Rows[3] = r4;
    Rows[4] = r5;
    Rows[5] = r6;
    Rows[6] = r7;
    Rows[7] = r8;
    Rows[8] = r9;
    Rows[9] = r10;
    Rows[10] = r11;
    Rows[11] = r12;
}
