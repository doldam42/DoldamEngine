#pragma once

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

    void SetCollider(ICollider *pCollider) override;
    void SetRigidBody(IRigidBody *pBody) override;

    Vector3 GetPosition() override;
    Vector3 GetScale() override;
    Vector3 GetForward() override;

    float GetRotationX() override;
    float GetRotationY() override;
    float GetRotationZ() override;

    Quaternion GetRotation() override;

    void SetModel(IGameModel *pModel) override;
    void SetPosition(float x, float y, float z) override;
    void SetScale(float x, float y, float z) override;
    void SetScale(float s) override;
    void SetRotationX(float rotX) override;
    void SetRotationY(float rotY) override;
    void SetRotationZ(float rotZ) override;
    void SetRotation(Quaternion q) override;

    void SetActive(BOOL state) override;

    void AddPosition(Vector3 deltaPos) override;

    void SetMaterials(IRenderMaterial **ppMaterials, const UINT numMaterials) override;
    IRenderMaterial *GetMaterialAt(UINT index) override;

    size_t GetID() override;

    Bounds GetBounds() const;
    bool   IntersectRay(const Ray &ray, float *hitt0, float *hitt1) const;
    bool   Intersect(const Bounds& b) const;

    // Inherited via IGameCharacter
    ICollider *GetCollider() const override;
    IRigidBody *GetRigidBody() const override;
};
