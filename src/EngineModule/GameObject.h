#pragma once
#include "MathHeaders.h"
#include "RigidBody.h"
class IRenderer;
class GameManager;
class Model;
class GameObject : public IGameObject
{
    static size_t g_id;

    size_t    m_id;
    Transform m_transform;
    Matrix    m_worldMatrix;
    BOOL      m_IsUpdated = false;
    BOOL      m_isStatic = TRUE;
    UINT      m_materialCount = 0;

    GameManager      *m_pGame = nullptr;
    IRenderer        *m_pRenderer = nullptr;
    Model            *m_pModel = nullptr;
    IRenderMaterial **m_ppMaterials = nullptr;

    ICollider *m_pCollider = nullptr;
    RigidBody *m_pRigidBody = nullptr;

  public:
    SORT_LINK m_LinkInGame;
    SORT_LINK m_LinkInWorld;
    void     *m_pSearchHandleInGame = nullptr;

    BOOL m_isVisible = TRUE;

  private:
    void CleanupMaterials();

    void Cleanup();

  public:
    void Initialize(GameManager *pGameEngine, BOOL isStatic);

    BOOL InitBoxCollider(const Vector3 &center, const Vector3 &extent) override;
    BOOL InitSphereCollider(const Vector3 &center, const float radius) override;
    BOOL InitConvexCollider() override;

    BOOL InitRigidBody(float mass, float elasticity, float friction, BOOL useGravity = TRUE,
                       BOOL isKinematic = FALSE) override;

    virtual void Update(float dt);
    void         Render();

    // Getter
    const Transform &GetTransform() { return m_transform; }
    const Matrix    &GetWorldMatrix() { return m_worldMatrix; }
    Model           *GetModel() { return m_pModel; }

    ICollider  *GetCollider() const override { return m_pCollider; }
    IRigidBody *GetRigidBody() const override { return m_pRigidBody; }

    Vector3    GetPosition() override { return m_transform.GetPosition(); }
    Vector3    GetScale() override { return m_transform.GetScale(); }
    Vector3    GetForward() override { return m_transform.GetForward(); }
    float      GetRotationX() override { return m_transform.GetRotation().ToEuler().x; }
    float      GetRotationY() override { return m_transform.GetRotation().ToEuler().y; }
    float      GetRotationZ() override { return m_transform.GetRotation().ToEuler().z; }
    Quaternion GetRotation() override { return m_transform.GetRotation(); }

    void SetModel(IGameModel *pModel) override;
    void SetPosition(float x, float y, float z) override;
    void SetScale(float x, float y, float z) override;
    void SetScale(float s) override;
    void SetRotationY(float rotY) override;
    void SetRotationX(float rotX) override;
    void SetRotationZ(float rotZ) override;
    void SetRotation(Quaternion q) override;

    void AddPosition(Vector3 deltaPos) override;

    void             SetMaterials(IRenderMaterial **ppMaterials, const UINT numMaterials) override;
    IRenderMaterial *GetMaterialAt(UINT index) override;

    virtual size_t GetID() override { return m_id; }

    GameObject();
    virtual ~GameObject();

    BOOL   IsStatic() const { return m_isStatic; }
    BOOL   HasBounds() const { return (m_pModel || m_pCollider); }
    Bounds GetBounds() const override;
    bool   IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const override;
    bool   Intersect(const Bounds &b) const override;
};
