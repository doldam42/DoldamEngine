#pragma once

enum GUI_REQUEST_TYPE
{
    GUI_REQUEST_TYPE_NONE = 0,
    GUI_REQUEST_TYPE_CREATE_MODEL,
    GUI_REQUEST_TYPE_CREATE_ANIMATION,
    GUI_REQUEST_TYPE_CREATE_GAMEOBJECT,
};

struct FileNode;
struct GUIView
{
    static constexpr float WINDOW_WIDTH = 0.2f;
    static constexpr float WINDOW_HEIGHT = 0.7f;

    static constexpr float SCENE_VIEW_WIDTH = 1.0f - 2 * WINDOW_WIDTH;
    static constexpr float SCENE_VIEW_HEIGHT = WINDOW_HEIGHT;

    static const GUI_WINDOW_FLAGS g_windowFlags =
        GUI_WINDOW_FLAG_NO_MOVE | GUI_WINDOW_FLAG_NO_RESIZE | GUI_WINDOW_FLAG_NO_TITLE_BAR;

    IRenderGUI *pGUI;

    UINT width = 0;
    UINT height = 0;

    // left sidebar
    Vector2 HierarchyPos = Vector2(0.0f, 0.0f);
    Vector2 HierarchySize = Vector2(WINDOW_WIDTH, WINDOW_HEIGHT);
    void    ShowHierarchy();

    // center top
    ITextureHandle *pSceneViewTex = nullptr;
    Vector2         SceneViewPos = Vector2(WINDOW_WIDTH, 0.0f);
    Vector2         SceneViewSize = Vector2(1.0f - 2 * WINDOW_WIDTH, WINDOW_HEIGHT);
    void            ShowSceneView();

    // right sidebar
    Vector2      InspectorPos = Vector2(1.0f - WINDOW_WIDTH, 0.0f);
    Vector2      InspectorSize = Vector2(WINDOW_WIDTH, 1.0f);
    IGameObject *pSelectedObject = nullptr;
    void         ShowInspector();

    // bottom
    Vector2   ProjectPos = Vector2(0.0f, WINDOW_HEIGHT);
    Vector2   ProjectSize = Vector2(1.0f - WINDOW_WIDTH, 1.0f - WINDOW_HEIGHT);
    FileNode *assetDir = nullptr;
    void      ShowProject();

    bool isItemSelected = false;
    bool shouldCreateItem = false;

    GUI_REQUEST_TYPE currentRequestType = GUI_REQUEST_TYPE_NONE;

    std::vector<IGameModel *>  models;
    std::vector<IGameObject *> objects;

    WCHAR assetPath[MAX_PATH] = {L'\0'};
    WCHAR basePath[MAX_PATH] = {L'\0'};
    WCHAR filename[MAX_NAME] = {L'\0'};

    GUIView(IRenderGUI *gui, ITextureHandle *pSceneViewTex_, FileNode *assetDir_, const WCHAR *assetPath_, UINT width_,
            UINT height_)
        : pGUI(gui), pSceneViewTex(pSceneViewTex_), assetDir(assetDir_), width(width_), height(height_)
    {
        wcscpy_s(assetPath, MAX_PATH, assetPath_);
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