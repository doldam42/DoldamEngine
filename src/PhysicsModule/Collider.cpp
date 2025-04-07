#include "pch.h"
#include "Collider.h"

Collider *Collider::CreateBoxCollider(const Vector3 &halfExtents) 
{ 
    Collider *pNew = new Collider;
    pNew->m_pShape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
    return pNew;
}

Collider *Collider::CreateSphereCollider(const float radius)
{
    Collider *pNew = new Collider;
    pNew->m_pShape = new btSphereShape(btScalar(radius));
    return pNew;
}

Collider *Collider::CreateCapsuleCollider(const float radius, const float height) 
{ 
    Collider *pNew = new Collider;
    pNew->m_pShape = new btCapsuleShape(btScalar(radius), btScalar(height));
    return pNew;
}

COLLIDER_TYPE Collider::GetType() const { return COLLIDER_TYPE_BOX; }

void Collider::Update() {}

Vector3 Collider::GetCenter() const { return Vector3(); }

Vector3 Collider::GetWorldCenter() const { return Vector3(); }

Bounds Collider::GetBounds() const { return Bounds(); }

Bounds Collider::GetWorldBounds() const { return Bounds(); }

Matrix Collider::InertiaTensor() const { return Matrix(); }

BOOL Collider::Intersect(ICollider *pOther) const { return 0; }

BOOL Collider::IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const { return 0; }

BOOL Collider::Intersect(const Bounds &b) const { return 0; }

Vector3 Collider::Support(const Vector3 dir, const Vector3 pos, const Quaternion orient, const float bias)
{
    return Vector3();
}

float Collider::FastestLinearSpeed(const Vector3 angularVelocity, const Vector3 dir) const { return 0.0f; }
