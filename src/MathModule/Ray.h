#pragma once

struct Ray
{
    Vector3 pos;
    Vector3 dir;

    inline Ray &operator=(Ray &ref);
    inline bool operator==(Ray &ref);
};

Ray &Ray::operator=(Ray &ref)
{
    pos = ref.pos;
    dir = ref.dir;
}

bool Ray::operator==(Ray &ref) 
{
    using namespace DirectX;
    const XMVECTOR r1p = XMLoadFloat3(&pos);
    const XMVECTOR r2p = XMLoadFloat3(&ref.pos);
    const XMVECTOR r1d = XMLoadFloat3(&dir);
    const XMVECTOR r2d = XMLoadFloat3(&ref.dir);
    return XMVector3Equal(r1p, r2p) && XMVector3Equal(r1d, r2d);
}