#pragma once
#include "MathHeaders.h"
#include "RigidBody.h"
class IRenderer;
class GameManager;
class Model;
class GameObject : public IGameObject
{
    static size_t g_id;
    size_t        m_id;
    Transform     m_transform;
    Matrix        m_worldMatrix;

    GameManager      *m_pGameEngine = nullptr;
    IRenderer        *m_pRenderer = nullptr;
    Model            *m_pModel = nullptr;
    IRenderMaterial **m_ppMaterials = nullptr;
    UINT              m_materialCount = 0;

    ICollider *m_pCollider = nullptr;
    RigidBody *m_pRigidBody = nullptr;

    bool m_IsUpdated = false;

  public:
    SORT_LINK m_LinkInGame;
    SORT_LINK m_LinkInWorld;
    void     *m_pSearchHandleInGame = nullptr;

  private:
    void CleanupMaterials();

    void Cleanup();

  public:
    void Initialize(GameManager *pGameEngine);

    void InitBoxCollider(const Vector3& center, const Vector3& extent) override;
    void InitSphereCollider(const Vector3& center, const float radius) override;
    void InitRigidBody(SHAPE_TYPE shapeType, float mass, float elasticity, float friction) override;

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
    void SetRotation(const Quaternion *pInQuaternion) override;

    void AddPosition(const Vector3 *pInDeltaPos) override;

    void             SetMaterials(IRenderMaterial **ppMaterials, const UINT numMaterials) override;
    IRenderMaterial *GetMaterialAt(UINT index) override;

    virtual size_t GetID() override { return m_id; }

    GameObject();
    virtual ~GameObject();

    // Inherited via IBoundedObject
    Bounds GetBounds() const override;
    bool   Intersect(const Ray &ray, float *hitt0, float *hitt1) const override;
    bool   Intersect(const Bounds b) const override;
};
