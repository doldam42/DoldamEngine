#pragma once
#include "MathHeaders.h"

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
    UINT      m_materialCount = 0;
    UINT      m_layer = 0;

    GameManager      *m_pGame = nullptr;
    IRenderer        *m_pRenderer = nullptr;
    Model            *m_pModel = nullptr;
    IRenderMaterial **m_ppMaterials = nullptr;

    ICollider  *m_pCollider = nullptr;
    IRigidBody *m_pRigidBody = nullptr;

  public:
    SORT_LINK m_LinkInGame;
    SORT_LINK m_LinkInWorld;
    void     *m_pSearchHandleInGame = nullptr;

    BOOL m_isVisible = TRUE;
    BOOL m_isActive = TRUE;

  private:
    void CleanupMaterials();

    void Cleanup();

  public:
    void Initialize(GameManager *pGameEngine);

    void SetCollider(ICollider *pCollider) override { m_pCollider = pCollider; }
    void SetRigidBody(IRigidBody *pBody) override
    {
        m_pRigidBody = pBody;
    }

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

    void SetActive(BOOL state) override
    {
        m_isActive = state;
        if (m_pRigidBody)
        {
            m_pRigidBody->SetActive(state);
        }
    }

    void AddPosition(Vector3 deltaPos) override;

    void             SetMaterials(IRenderMaterial **ppMaterials, const UINT numMaterials) override;
    IRenderMaterial *GetMaterialAt(UINT index) override;

    virtual size_t GetID() override { return m_id; }

    void SetLayer(UINT layer) override { m_layer = layer; }
    UINT GetLayer() override { return m_layer; }

    GameObject();
    virtual ~GameObject();
    BOOL   HasBounds() const { return (m_pModel || m_pCollider); }
    Bounds GetBounds() const;
    bool   IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const;
    bool   Intersect(const Bounds &b) const;
};
