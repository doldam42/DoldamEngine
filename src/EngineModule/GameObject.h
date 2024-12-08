#pragma once
#include "MathHeaders.h"

class IRenderer;
class GameManager;
class Model;
class GameObject : public IGameObject
{
    Transform m_transform;
    Matrix    m_worldMatrix;

    GameManager *m_pGameEngine = nullptr;
    IRenderer  *m_pRenderer = nullptr;
    Model      *m_pModel = nullptr;

    bool m_IsUpdated = false;

  public:
    SORT_LINK m_LinkInGame;

    // Physics
    Vector3    m_linearVelocity = Vector3::Zero;
    float      m_invMass = 0.0f;
    float      m_elasticity = 1.0f;
    SHAPE_TYPE m_collisionShapeType = SHAPE_TYPE_NONE;

  private:
    void Cleanup();

  public:
    void         Initialize(GameManager *pGameEngine);
    virtual void Run();
    void         Render();

    // Physics
    Vector3 GetVelocity() const override { return m_linearVelocity; }

    static void ResolveContact(GameObject *pA, GameObject *pB, const Contact &contact);

    void ApplyImpulseLinear(const Vector3 &impulse) override;
    BOOL Intersect(const GameObject *pOther, Contact *pOutContact) const;

    // Getter
    inline const Transform &GetTransform() { return m_transform; }
    inline const Matrix    &GetWorldMatrix() { return m_worldMatrix; }
    inline Model           *GetModel() { return m_pModel; }

    inline Vector3 GetPosition() override { return m_transform.GetPosition(); }
    inline Vector3 GetScale() override { return m_transform.GetScale(); }
    inline float   GetRotationX() override { return m_transform.GetRotation().ToEuler().x; }
    inline float   GetRotationY() override { return m_transform.GetRotation().ToEuler().y; }
    inline float   GetRotationZ() override { return m_transform.GetRotation().ToEuler().z; }

    inline Quaternion GetRotation() { return m_transform.GetRotation(); }

    void GetBoxInWorld(Box *pOutBox) const;
    void GetSphereInWorld(Sphere *pOutSphere) const;

    // Setter
    void SetPhysics(SHAPE_TYPE collisionType, float mass, float elasticity) override;

    void SetModel(IGameModel *pModel) override;
    void SetPosition(float x, float y, float z) override;
    void SetScale(float x, float y, float z) override;
    void SetScale(float s) override;
    void SetRotationY(float rotY) override;
    void SetRotationX(float rotX) override;
    void SetRotationZ(float rotZ) override;

    GameObject();
    virtual ~GameObject();
};
