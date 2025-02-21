#pragma once
struct GUIView;
struct FileNode;
class GUIController : public IController
{
    IRenderer *m_pRenderer = nullptr;
    GUIView *m_pGUIView = nullptr;

    ITextureHandle *m_pTex = nullptr;

    void Cleanup();

  public:
    BOOL Initialize(IRenderer* pRnd, FileNode* assetDir, const WCHAR* basePath, UINT width, UINT height);

    void OnUpdateWindowSize(UINT width, UINT height);

    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;
    void Render() override;

    Vector2 GetViewportSizeRatio();

    GUIController() = default;
    ~GUIController();
};
