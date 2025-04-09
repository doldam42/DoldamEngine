#pragma once
class btHeightfieldTerrainShape;
class HeightFieldTerrain : public IHeightFieldTerrainCollider
{
    btHeightfieldTerrainShape *m_pShape = nullptr;
    btRigidBody               *m_pBody = nullptr;
  public:

    BOOL Initialize(btDiscreteDynamicsWorld * dynamicsWorld, btAlignedObjectArray<btCollisionShape *> & shapeArray, const BYTE *pImage,
                    const UINT imgWidth,
                    const UINT imgHeight, const Vector3 &scale,
                    const float minHeight, const float maxHeight);

    btHeightfieldTerrainShape *Get() const { return m_pShape; }
};
