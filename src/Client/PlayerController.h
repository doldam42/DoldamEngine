#pragma once
class PlayerController : public IController
{
    static constexpr float SPEED = 10.0f;

    IGameObject *m_pPlayer = nullptr;
    ICharacterBody *m_pPlayerBody = nullptr;

    float speed = SPEED;
    float jumpSpeed = 0.0f;
  public:
    BOOL Start() override;
    void Update(float dt) override;
};
