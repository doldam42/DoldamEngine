#pragma once
class CharacterController
{
    IPhysicsManager *m_pPhysics = nullptr;

    IGameObject *m_pGameObject = nullptr;
    ICollider   *m_pCollider = nullptr;

    float m_height = 0.0f;
    float m_radius = 0.0f;

    BOOL m_isGrounded = TRUE;
    BOOL m_isActive = TRUE;

    Vector3 m_velocity;

  private:
    void ApplyGravity(float dt);
    void UpdatePosition(float dt);
    void CheckGrounded();

  public:
    BOOL Initialize(const Vector3 &startPosition, float height, float radius);

    CharacterController() = default;
    ~CharacterController();
};
