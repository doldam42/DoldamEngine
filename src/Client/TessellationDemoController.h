#pragma once
class TessellationDemoController : public IController
{
    IGameObject *m_pGround = nullptr;

    void Cleanup();

  public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;

    TessellationDemoController() = default;
    ~TessellationDemoController();
};
