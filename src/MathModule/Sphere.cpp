#include "pch.h"
#include "Sphere.h"

void Sphere::LocalToWorld(Sphere *pOutSphere, const Matrix &worldTM) const
{
    using namespace DirectX;

    FXMMATRIX M = worldTM;
    XMVECTOR  vCenter = Center;

    XMVECTOR C = XMVector3Transform(vCenter, M);

    XMVECTOR dX = XMVector3Dot(M.r[0], M.r[0]);
    XMVECTOR dY = XMVector3Dot(M.r[1], M.r[1]);
    XMVECTOR dZ = XMVector3Dot(M.r[2], M.r[2]);

    XMVECTOR d = XMVectorMax(dX, XMVectorMax(dY, dZ));

    // Store the center sphere.
    XMStoreFloat3(&pOutSphere->Center, C);

    // Scale the radius of the pshere.
    float Scale = sqrtf(XMVectorGetX(d));
    pOutSphere->Radius = Radius * Scale;
}

Matrix Sphere::InertiaTensor() const 
{ 
    Matrix tensor;
    tensor.m[0][0] = 2.0f * Radius * Radius / 5.0f;
    tensor.m[1][1] = 2.0f * Radius * Radius / 5.0f;
    tensor.m[2][2] = 2.0f * Radius * Radius / 5.0f;
    return tensor;
}

bool Sphere::Intersect(const Box &inBox) const
{
    Contact c;
    return CheckCollision(inBox, *this, &c);
}