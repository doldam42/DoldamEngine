#pragma once

class PhysicsManager : public IPhysicsManager
{
    const static UINT MAX_BODY_COUNT = 1024;

  private:
    ULONG m_refCount = 1;

    btDefaultCollisionConfiguration     *m_pCollisionConfiguration = nullptr;
    btCollisionDispatcher               *m_pDispatcher = nullptr;
    btBroadphaseInterface               *m_pOverlappingPairCache = nullptr;
    btSequentialImpulseConstraintSolver *m_pSolver = nullptr;
    btDiscreteDynamicsWorld             *m_pDynamicWorld = nullptr;
    btAlignedObjectArray<btCollisionShape *> m_collisionShapes;

    void Cleanup();

  public:
    BOOL Initialize() override;

    // Inherited via IPhysicsManager
    ICollider  *CreateSphereCollider(const float radius) override;
    ICollider  *CreateBoxCollider(const Vector3 &halfExtents) override;
    ICollider  *CreateCapsuleCollider(const float radius, const float height) override;
    ICollider  *CreateConvexCollider(const Vector3 *points, const int numPoints) override;

    IRigidBody *CreateRigidBody(ICollider *pCollider, const Vector3 &pos, float mass, float elasticity, float friction,
                                BOOL useGravity) override;
    ICharacterBody *CreateCharacterBody(const Vector3 &startPosition, const float radius,
                                        const float height) override;

    void DeleteCollider(ICollider *pDel);
    void DeleteRigidBody(IRigidBody *pDel);

    IHeightFieldTerrainCollider *CreateHeightFieldTerrain(const BYTE *pImage, const UINT imgWidth, const UINT imgHeight,
                                                  const Vector3 &scale, const float minHeight,
                                                  const float maxHeight) override;

    void        BuildScene() override;
    void        BeginCollision(float dt) override;
    BOOL        CollisionTestAll(float dt) override;
    void        EndCollision() override;
    BOOL        Raycast(const Ray &ray, float *tHit, IRigidBody **ppHitted) override;

    // Inherited via IPhysicsManager
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;

    PhysicsManager() = default;
    ~PhysicsManager();
};

extern PhysicsManager *g_pPhysics;
