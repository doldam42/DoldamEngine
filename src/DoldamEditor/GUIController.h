#pragma once
class GUIController : public IController
{
    IRenderGUI *m_pGUI = nullptr;

    void Cleanup();
  public:
    BOOL Initilize(IRenderGUI* pGUI);

    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;
    void Render() override;

    GUIController() = default;
    ~GUIController();
};
