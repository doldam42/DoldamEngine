#pragma once
struct GUIView;
class GUIController : public IController
{
    GUIView *m_pGUIView = nullptr;

    ITextureHandle *m_pTex = nullptr;

    void Cleanup();

  public:
    BOOL Initilize(IRenderGUI* pGUI, const WCHAR* assetPath);

    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;
    void Render() override;

    GUIController() = default;
    ~GUIController();
};
