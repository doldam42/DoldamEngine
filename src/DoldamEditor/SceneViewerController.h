#pragma once
class SceneViewerController : public IController
{
  private:
    IGameObject *m_pGround = nullptr;
    IGameObject *m_pBox = nullptr;

    void Cleanup();

  public:
    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;

    SceneViewerController() = default;
    ~SceneViewerController();

    // Inherited via IController
    void Render() override;
};
