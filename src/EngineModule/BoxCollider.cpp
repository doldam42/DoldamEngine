#include "pch.h"

#include "GameObject.h"

#include "BoxCollider.h"

BOOL BoxCollider::Initialize(GameObject *pObj, const Vector3 &center, const Vector3 &extent) 
{ 
	m_pGameObject = pObj;

	m_bounds = Bounds(center - extent, center + extent);

	return TRUE;
}
Bounds BoxCollider::GetBounds() const { return m_bounds; }

Bounds BoxCollider::GetWorldBounds() const 
{
    Matrix m = m_pGameObject->GetWorldMatrix();

    Bounds box;
    m_bounds.Transform(&box, m);
    return box;
}

Matrix BoxCollider::InertiaTensor() const 
{
	// Inertia tensor for box centered around zero
    const float dx = m_bounds.WidthX();
    const float dy = m_bounds.WidthY();
    const float dz = m_bounds.WidthZ();

	Matrix tensor;
    tensor._11 = (dy * dy + dz * dz) / 12.0f;
    tensor._22 = (dx * dx + dz * dz) / 12.0f;
    tensor._33 = (dx * dx + dy * dy) / 12.0f;

	// Now We need to use the parallel axis theorem to get the inertia tensor for a box
	// that is not centered around the origin

	Vector3 cm = m_bounds.Center();

	const Vector3 R = -cm;
    const float   R2 = R.LengthSquared();

	Matrix patTensor(
		R2 - R.x * R.x, R.x * R.y, R.x * R.z, 0.0f, 
		R.y * R.x, R2 - R.y * R.y, R.y * R.z, 0.0f, 
		R.z * R.x, R.z * R.y, R2 - R.z * R.z, 0.0f, 
		0.0f, 0.0f, 0.0f, 0.0f
	);

	tensor += patTensor;
    return tensor;
}
