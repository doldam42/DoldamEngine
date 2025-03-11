#pragma once
class PhysicsDemoController : public IController
{
    IRenderMaterial *m_pMaterial;

    IGameObject     *m_pBalls[100];
  public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;

    PhysicsDemoController() = default;
    ~PhysicsDemoController() {}
};
