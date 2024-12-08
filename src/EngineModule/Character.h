#pragma once

#include "EngineInterface.h"
#include "GameObject.h"

class Character : public IGameCharacter, public GameObject
{
  private:
    UINT            m_maxClipCount = 0;
    UINT            m_clipCount = 0;
    UINT            m_frameCount = 0;
    UINT            m_clipId = 0;
    AnimationClip **m_ppAnimationClips = nullptr;

  private:
    void Cleanup();

  public:

    void Initialize(GameManager *pGameEngine, UINT maxClipCount);
    void Update(float dt) override;

    void InsertAnimation(IGameAnimation *pClip) override;
    void SetCurrentAnimationByName(const WCHAR *pAnimationName);

    Character() = default;
    ~Character() override;

    // Inherited via GameObject
    void InitPhysics(const Shape *pInShape, float mass, float elasticity, float friction) override;

    Vector3 GetPosition() override;
    Vector3 GetScale() override;
    float   GetRotationX() override;
    float   GetRotationY() override;
    float   GetRotationZ() override;
    Quaternion GetRotation() override;

    void    SetModel(IGameModel *pModel) override;
    void    SetPosition(float x, float y, float z) override;
    void    SetScale(float x, float y, float z) override;
    void    SetScale(float s) override;
    void    SetRotationX(float rotX) override;
    void    SetRotationY(float rotY) override;
    void    SetRotationZ(float rotZ) override;
    void    SetRotation(const Quaternion *pInQuaternion) override;

    void AddPosition(const Vector3 *pInDeltaPos) override;

    Bounds GetBounds() const override;
};
