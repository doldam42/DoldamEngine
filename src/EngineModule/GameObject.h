#pragma once
#include "MathHeaders.h"

class IRenderer;
class GameEngine;
class Model;
class GameObject : public IGameObject
{
    Transform m_transform;
    Matrix    m_worldMatrix;

    GameEngine *m_pGameEngine = nullptr;
    IRenderer  *m_pRenderer = nullptr;
    Model      *m_pModel = nullptr;

    bool m_IsUpdated = false;

  public:
    SORT_LINK m_LinkInGame;

  private:
    void Cleanup();

  public:
    void Initialize(GameEngine *pGameEngine);
    virtual void Run();
    void Render();

    // Getter
    inline const Transform &GetTransform() { return m_transform; }
    inline const Matrix    &GetWorldMatrix() { return m_worldMatrix; }
    inline Model           *GetModel() { return m_pModel; }

    inline Vector3 GetPosition() override { return m_transform.GetPosition(); }
    inline Vector3 GetScale() override { return m_transform.GetScale(); }
    inline float   GetRotationX() override { return m_transform.GetRotation().ToEuler().x; }
    inline float   GetRotationY() override { return m_transform.GetRotation().ToEuler().y; }
    inline float   GetRotationZ() override { return m_transform.GetRotation().ToEuler().z; }

    // Setter
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
