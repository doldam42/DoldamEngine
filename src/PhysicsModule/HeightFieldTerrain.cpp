#include "pch.h"

#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "HeightFieldTerrain.h"

BOOL HeightFieldTerrain::Initialize(btDiscreteDynamicsWorld                  *dynamicsWorld,
                                    btAlignedObjectArray<btCollisionShape *> &shapeArray, const BYTE *pImage,
                                    const UINT imgWidth, const UINT imgHeight, const Vector3 &scale,
                                    const float minHeight, const float maxHeight)
{
    float heightScale = (maxHeight - minHeight) / 255.0f;

    m_pShape = new btHeightfieldTerrainShape(imgWidth, imgHeight, pImage, heightScale, minHeight, maxHeight, 1,
                                             PHY_UCHAR, false);
    m_pShape->setLocalScaling(btVector3(scale.x / imgWidth, 1.0, scale.z / imgHeight));
    m_pShape->buildAccelerator();

    btTransform startTransform;
    startTransform.setIdentity();

    btDefaultMotionState                    *myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(btScalar(0.0f), myMotionState, m_pShape);
    m_pBody = new btRigidBody(rbInfo);
    
    shapeArray.push_back(m_pShape);
    dynamicsWorld->addRigidBody(m_pBody);
    return TRUE;
}
