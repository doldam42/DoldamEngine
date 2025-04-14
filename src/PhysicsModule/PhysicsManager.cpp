#include "pch.h"

#include "CharacterBody.h"
#include "Collider.h"
#include "RigidBody.h"
#include "HeightFieldTerrain.h"

#include "PhysicsManager.h"

PhysicsManager *g_pPhysics = nullptr;

void PhysicsManager::Cleanup()
{
    int i;
    // remove the rigidbodies from the dynamics world and delete them
    for (i = m_pDynamicWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject *obj = m_pDynamicWorld->getCollisionObjectArray()[i];
        btRigidBody       *body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            delete body->getMotionState();
        }
        m_pDynamicWorld->removeCollisionObject(obj);
        delete obj;
    }

    // delete dynamics world
    delete m_pDynamicWorld;

    // delete solver
    delete m_pSolver;

    // delete broadphase
    delete m_pOverlappingPairCache;

    // delete dispatcher
    delete m_pDispatcher;

    delete m_pCollisionConfiguration;
}

BOOL PhysicsManager::Initialize()
{
    /// collision configuration contains default setup for memory, collision setup. Advanced users can create their own
    /// configuration.
    btDefaultCollisionConfiguration *collisionConfiguration = new btDefaultCollisionConfiguration();

    /// use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see
    /// Extras/BulletMultiThreaded)
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfiguration);
    
    /// btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    btBroadphaseInterface *overlappingPairCache = new btDbvtBroadphase();
    overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

    /// the default constraint solver. For parallel processing you can use a different solver (see
    /// Extras/BulletMultiThreaded)
    btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver;

    btDiscreteDynamicsWorld *dynamicsWorld =
        new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

    dynamicsWorld->setGravity(btVector3(0, -10, 0));

    g_pPhysics = this;

    m_pCollisionConfiguration = collisionConfiguration;
    m_pDispatcher = dispatcher;
    m_pOverlappingPairCache = overlappingPairCache;
    m_pSolver = solver;
    m_pDynamicWorld = dynamicsWorld;

    return TRUE;
}

ICollider *PhysicsManager::CreateSphereCollider(const float radius)
{
    Collider *pNew = Collider::CreateSphereCollider(radius);
    m_collisionShapes.push_back(pNew->Get());
    return pNew;
}

ICollider *PhysicsManager::CreateBoxCollider(const Vector3 &halfExtents)
{
    Collider *pNew = Collider::CreateBoxCollider(halfExtents);
    m_collisionShapes.push_back(pNew->Get());
    return pNew;
}

ICollider *PhysicsManager::CreateCapsuleCollider(const float radius, const float height)
{
    Collider *pNew = Collider::CreateCapsuleCollider(radius, height);
    m_collisionShapes.push_back(pNew->Get());
    return pNew;
}

ICollider *PhysicsManager::CreateConvexCollider(const Vector3 *points, const int numPoints) 
{
    Collider *pNew = Collider::CreateConvexCollider(points, numPoints);
    m_collisionShapes.push_back(pNew->Get());
    return pNew;
}

IRigidBody *PhysicsManager::CreateRigidBody(ICollider *pCollider, const Vector3 &pos, float mass, float elasticity,
                                            float friction, BOOL useGravity)
{
    Collider *pBase = (Collider *)pCollider;

    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));

    btVector3 localInertia;
    localInertia.setZero();
    if (mass > 0.0f)
        pBase->m_pShape->calculateLocalInertia(mass, localInertia);

    btDefaultMotionState                    *myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(btScalar(mass), myMotionState, pBase->m_pShape, localInertia);

    RigidBody *pBody = new RigidBody(rbInfo);
    pBody->setFriction(friction);
    pBody->setDamping(1.0f - elasticity, 1.0f - elasticity);

    m_pDynamicWorld->addRigidBody(pBody);
    
    return pBody;
}

ICharacterBody *PhysicsManager::CreateCharacterBody(const Vector3 &startPosition, const float radius,
                                                    const float height)
{
    CharacterBody *pNew = new CharacterBody;
    pNew->Initialize(m_pDynamicWorld, startPosition, radius, height);
    return pNew;
}

void PhysicsManager::DeleteCollider(ICollider *pDel)
{
    Collider *pCollider = (Collider *)pDel;
    if (pCollider)
    {
        delete pDel;
        pDel = nullptr;
    }
}

void PhysicsManager::DeleteRigidBody(IRigidBody *pDel)
{
    RigidBody *pBody = (RigidBody *)pDel;
    m_pDynamicWorld->removeRigidBody(pBody);
    delete pBody;
}

IHeightFieldTerrainCollider *PhysicsManager::CreateHeightFieldTerrain(const BYTE *pImage, const UINT imgWidth,
                                                              const UINT imgHeight, const Vector3 &scale,
                                                              const float minHeight, const float maxHeight)
{
    HeightFieldTerrain *pNew = new HeightFieldTerrain;
    pNew->Initialize(m_pDynamicWorld, m_collisionShapes, pImage, imgWidth, imgHeight, scale, minHeight, maxHeight);
    return pNew;
}

void PhysicsManager::RemoveFromWorld(btRigidBody *pBody) { m_pDynamicWorld->removeRigidBody(pBody); }

void PhysicsManager::AddToWorld(btRigidBody *pBody) { m_pDynamicWorld->addRigidBody(pBody); }

void PhysicsManager::BuildScene() {}

void PhysicsManager::BeginCollision(float dt)
{
    m_pDynamicWorld->stepSimulation(dt, 10);
}

BOOL PhysicsManager::CollisionTestAll(float dt)
{
    btCollisionObjectArray &arr = m_pDynamicWorld->getCollisionObjectArray();
    for (int j = m_pDynamicWorld->getNumCollisionObjects() - 1; j >= 0; j--)
    {
        btCollisionObject *obj = arr[j];
        btRigidBody       *body = btRigidBody::upcast(obj);
        if (!body)
            continue;
        RigidBody  *pBody = (RigidBody *)body;
        btTransform trans;
        if (body && body->getMotionState())
        {
            body->getMotionState()->getWorldTransform(trans);
        }
        else
        {
            trans = obj->getWorldTransform();
        }

        if (pBody)
        {
            btVector3   &btOrigin = trans.getOrigin();
            btQuaternion btQ = trans.getRotation();
            
            pBody->SetPositionInternal(Vector3(btOrigin.getX(), btOrigin.getY(), btOrigin.getZ()));
            pBody->SetRotationInternal(Quaternion(btQ.getX(), btQ.getY(), btQ.getZ(), btQ.getW()));
        }
    }

    return TRUE;
}

void PhysicsManager::EndCollision() {}

BOOL PhysicsManager::Raycast(const Ray &ray, float *tHit, IRigidBody **ppHitted)
{
    Vector3   from = ray.position;
    Vector3   to = from + ray.direction * ray.tmax;
    btVector3 btFrom(from.x, from.y, from.z);
    btVector3 btTo(to.x, to.y, to.z);

    btCollisionWorld::ClosestRayResultCallback cb(btFrom, btTo);
    /*cb.m_collisionFilterGroup = btBroadphaseProxy::DefaultFilter;
    cb.m_collisionFilterMask = btBroadphaseProxy::AllFilter;
    cb.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;*/

    m_pDynamicWorld->rayTest(btFrom, btTo, cb);
    if (cb.hasHit())
    {
        const btRigidBody *pBody = btRigidBody::upcast(cb.m_collisionObject);
        RigidBody         *pMyBody = (RigidBody *)pBody;

        *tHit = ray.tmax * cb.m_closestHitFraction;
        *ppHitted = pMyBody;

        return TRUE;
    }
    return FALSE;
}

HRESULT __stdcall PhysicsManager::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall PhysicsManager::AddRef(void)
{
    m_refCount++;
    return m_refCount;
}

ULONG __stdcall PhysicsManager::Release(void)
{
    ULONG ref_count = --m_refCount;
    if (!m_refCount)
        delete this;

    return ref_count;
}

PhysicsManager::~PhysicsManager()
{
    Cleanup();
}