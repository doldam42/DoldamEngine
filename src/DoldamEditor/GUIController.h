#pragma once
class GUIController : public IController
{
    const GUI_WINDOW_FLAGS m_windowFlags =
        GUI_WINDOW_FLAG_NO_MOVE | GUI_WINDOW_FLAG_NO_RESIZE | GUI_WINDOW_FLAG_NO_TITLE_BAR;

    const float WINDOW_WIDTH = 0.2f;
    const float WINDOW_HEIGHT = 0.7;

    // left sidebar
    const Vector2 HierarchyPos = Vector2(0.0f, 0.0f);
    const Vector2 HierarchySize = Vector2(WINDOW_WIDTH, WINDOW_HEIGHT);

    // center top
    const Vector2 SceneViewPos = Vector2(0.0f, 0.0f);
    const Vector2 SceneViewSize = Vector2(0.0f, 0.0f);

    // right sidebar
    const Vector2 InspectorPos = Vector2(1.0f - WINDOW_WIDTH, 0.0f);
    const Vector2 InspectorSize = Vector2(WINDOW_WIDTH, 1.0f);

    // bottom
    const Vector2 ProjectPos = Vector2(0.0f, WINDOW_HEIGHT);
    const Vector2 ProjectSize = Vector2(1.0f - WINDOW_WIDTH, 1.0f);

    IRenderGUI *m_pGUI = nullptr;

    ITextureHandle *m_pTex = nullptr;

    void ShowSceneView();
    void ShowHierarchy();
    void ShowInspector();
    void ShowProject();

    void Cleanup();

  public:
    BOOL Initilize(IRenderGUI *pGUI);

    // Inherited via IController
    BOOL Start() override;
    void Update(float dt) override;
    void Render() override;

    GUIController() = default;
    ~GUIController();
};
