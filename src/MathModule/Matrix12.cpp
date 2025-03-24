#include "pch.h"
#include "Matrix12.h"

const Matrix12 Matrix12::Zero = Matrix12(0.0f);
const Matrix12 Matrix12::Identity =
    Matrix12(Vector12(1.0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
             Vector12(0.0, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
             Vector12(0.0, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
             Vector12(0.0, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
             Vector12(0.0, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
             Vector12(0.0, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
             Vector12(0.0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
             Vector12(0.0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
             Vector12(0.0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f),
             Vector12(0.0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f),
             Vector12(0.0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f),
             Vector12(0.0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f));


void Matrix12::Transpose()
{ 
    for (int i = 0; i < 12; i++)
    {
        for (int j = i; j < 12; j++)
        {
            std::swap(Rows[i][j], Rows[j][i]);
        }
    }
}

void Matrix12::Mul(const Matrix12 &m, const Vector12 &v, Vector12 *result)
{
    for (int i = 0; i < 12; i++)
    {
        result->Data[i] = m.Rows[i].Dot(v);
    }
}
void Matrix12::Mul(const Matrix12 &m1, const Matrix12 &m2, Matrix12 *result) 
{
    ZeroMemory(result->Rows, sizeof(result->Rows));
    for (int i = 0; i < 12; i++)
    {
        for (int j = i; j < 12; j++)
        {
            result->Rows[i][j] += m1.Rows[i][j] * m2.Rows[j][i];
        }
    }
}