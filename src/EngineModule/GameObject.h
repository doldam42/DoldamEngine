#pragma once
#include "MathHeaders.h"

class IRenderer;
class GameManager;
class Model;
class PhysicsComponent;
class GameObject : public IGameObject
{
    Transform m_transform;
    Matrix    m_worldMatrix;

    GameManager *m_pGameEngine = nullptr;
    IRenderer  *m_pRenderer = nullptr;
    Model      *m_pModel = nullptr;

    PhysicsComponent *m_pPhysicsComponent = nullptr;

    bool m_IsUpdated = false;

  public:
    SORT_LINK m_LinkInGame;
    SORT_LINK m_LinkInWorld;

  private:
    void Cleanup();

  public:
    void Initialize(GameManager *pGameEngine);

    void InitPhysics(const Shape *pInShape, float mass, float elasticity, float friction) override;

    virtual void Update(float dt);
    void         Render();

    // Getter
    inline const Transform  &GetTransform() { return m_transform; }
    inline const Matrix     &GetWorldMatrix() { return m_worldMatrix; }
    inline Model            *GetModel() { return m_pModel; }
    inline PhysicsComponent *GetPhysicsComponent() { return m_pPhysicsComponent; }

    inline Vector3    GetPosition() override { return m_transform.GetPosition(); }
    inline Vector3    GetScale() override { return m_transform.GetScale(); }
    inline float      GetRotationX() override { return m_transform.GetRotation().ToEuler().x; }
    inline float      GetRotationY() override { return m_transform.GetRotation().ToEuler().y; }
    inline float      GetRotationZ() override { return m_transform.GetRotation().ToEuler().z; }
    inline Quaternion GetRotation() override { return m_transform.GetRotation(); }

    void SetModel(IGameModel *pModel) override;
    void SetPosition(float x, float y, float z) override;
    void SetScale(float x, float y, float z) override;
    void SetScale(float s) override;
    void SetRotationY(float rotY) override;
    void SetRotationX(float rotX) override;
    void SetRotationZ(float rotZ) override;
    void SetRotation(const Quaternion *pInQuaternion) override;

    void AddPosition(const Vector3 *pInDeltaPos) override;

    Bounds GetBounds() const;

    GameObject();
    virtual ~GameObject();
};
