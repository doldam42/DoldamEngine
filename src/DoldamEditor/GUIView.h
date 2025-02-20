#pragma once

struct FileNode;
struct GUIView
{
    IRenderGUI *pGUI;

    WCHAR selectedItemName[MAX_NAME] = {L'\0'};

    static constexpr float WINDOW_WIDTH = 0.2f;
    static constexpr float WINDOW_HEIGHT = 0.7f;

    static const GUI_WINDOW_FLAGS g_windowFlags =
        GUI_WINDOW_FLAG_NO_MOVE | GUI_WINDOW_FLAG_NO_RESIZE | GUI_WINDOW_FLAG_NO_TITLE_BAR;

    // left sidebar
    Vector2 HierarchyPos = Vector2(0.0f, 0.0f);
    Vector2 HierarchySize = Vector2(WINDOW_WIDTH, WINDOW_HEIGHT);
    void    ShowHierarchy();

    // center top
    Vector2 SceneViewPos = Vector2(0.0f, 0.0f);
    Vector2 SceneViewSize = Vector2(0.0f, 0.0f);
    void    ShowSceneView();

    // right sidebar
    Vector2 InspectorPos = Vector2(1.0f - WINDOW_WIDTH, 0.0f);
    Vector2 InspectorSize = Vector2(WINDOW_WIDTH, 1.0f);
    IGameObject *pSelectedObject = nullptr;
    void    ShowInspector();

    // bottom
    Vector2 ProjectPos = Vector2(0.0f, WINDOW_HEIGHT);
    Vector2 ProjectSize = Vector2(1.0f - WINDOW_WIDTH, 1.0f - WINDOW_HEIGHT);
    WCHAR     basePath[MAX_PATH] = {L'\0'};
    FileNode *assetDir = nullptr;
    void    ShowProject();

    GUIView(IRenderGUI *gui, FileNode *assetDir_, const WCHAR* basePath_) : pGUI(gui), assetDir(assetDir_) 
    {
        wcscpy_s(basePath, MAX_PATH, basePath_);
    }
    ~GUIView()
    {
        if (pGUI)
        {
            pGUI->Release();
            pGUI = nullptr;
        }
    };
};