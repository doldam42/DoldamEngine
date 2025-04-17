#pragma once
class CharacterController : public IController
{
    static constexpr float SPEED = 8.0f;

    IGameObject *m_pPlayer = nullptr;

    float speed = SPEED;
    float jumpSpeed = 20.0f;

  public:
    BOOL Start() override;
    void Update(float dt) override;
};
