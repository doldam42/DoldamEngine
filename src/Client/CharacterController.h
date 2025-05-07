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
    Vector3 m_foot;

  private:
    void ApplyGravity(float dt);
    void CheckGrounded();

  public:
    BOOL Initialize(const Vector3 &startPosition, float height, float radius);

    void Update(float dt);
    void Move(const Vector3 &dir);
    void Stop();
    void Jump(float jumpSpeed);
    IGameObject *GetGameObject() { return m_pGameObject; }

    BOOL OnGround() { return m_isGrounded; }

    CharacterController() = default;
    ~CharacterController() {}
};
