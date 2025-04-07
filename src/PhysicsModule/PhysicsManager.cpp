#include "pch.h"

#include "CharacterBody.h"
#include "Collider.h"
#include "RigidBody.h"

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

IRigidBody *PhysicsManager::CreateRigidBody(IGameObject *pObj, ICollider *pCollider, float mass, float elasticity,
                                            float friction, BOOL useGravity)
{
    RigidBody *pBody = new RigidBody;
    pBody->Initialize(pObj, pCollider, mass, elasticity, friction, useGravity);

    btRigidBody *pBtBody = pBody->Get();
    m_pDynamicWorld->addRigidBody(pBtBody);
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
    m_pDynamicWorld->removeRigidBody(pBody->Get());
    delete pBody;
}

void PhysicsManager::BuildScene() {}

void PhysicsManager::BeginCollision(float dt)
{
    m_pDynamicWorld->updateAabbs();
    m_pDynamicWorld->computeOverlappingPairs();
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
        RigidBody  *pBody = (RigidBody *)body->getUserPointer();
        btTransform trans;
        if (body && body->getMotionState())
        {
            body->getMotionState()->getWorldTransform(trans);
        }
        else
        {
            trans = obj->getWorldTransform();
        }

        btVector3   &btOrigin = trans.getOrigin();
        btQuaternion btQ = trans.getRotation();

        pBody->SetPosition(Vector3(btOrigin.getX(), btOrigin.getY(), btOrigin.getZ()));
        pBody->SetRotation(Quaternion(btQ.getX(), btQ.getY(), btQ.getZ(), btQ.getW()));
    }

    return TRUE;
}

void PhysicsManager::EndCollision() {}

BOOL PhysicsManager::Raycast(const Ray &ray, float *tHit, IGameObject **ppHitted)
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
        RigidBody         *pMyBody = (RigidBody *)pBody->getUserPointer();

        *tHit = ray.tmax * cb.m_closestHitFraction;
        *ppHitted = pMyBody->GetObj();

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