#pragma once
class CollisionDemoController : public IController
{
    const float  speed = 5.0f;
    float jumpSpeed = 0.0f;
    IGameObject *m_pPlayer = nullptr; 

    void Jump();

  public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;
    void Render() override;
};
