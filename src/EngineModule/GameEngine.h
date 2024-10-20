#pragma once

#include "CameraController.h"
#include "GameUtils.h"
#include "InputManager.h"
#include "Timer.h"

#include "../GenericModule/GenericHeaders.h"
#include "../MathModule/MathHeaders.h"
#include "../RenderModule/RendererInterface.h"

#include "EngineInterface.h"

class Model;
class AnimationClip;
class GameObject;
class Character;
class GameEngine : public IGameEngine
{
  public:
    static Model *SquareMesh;
    static Model *BoxMesh;
    static Model *SphereMesh;

    // For Debugging
    UINT m_culledObjectCountForDebug = 0;

  private:
    static UINT initRefCount;

    CameraController *m_pMainCamera = nullptr;
    IRenderer        *m_pRenderer = nullptr;
    InputManager     *m_pInputManager = nullptr;
    Timer            *m_pTimer = nullptr;
    Timer            *m_pPerformanceTimer = nullptr;

    // GameObjects
    SORT_LINK *m_pGameObjLinkHead = nullptr;
    SORT_LINK *m_pGameObjLinkTail = nullptr;

    // Model
    SORT_LINK *m_pModelLinkHead = nullptr;
    SORT_LINK *m_pModelLinkTail = nullptr;

    // Sprite
    SORT_LINK *m_pSpriteLinkHead = nullptr;
    SORT_LINK *m_pSpriteLinkTail = nullptr;

    // Animation
    HashTable *m_pAnimationHashTable = nullptr;

    ILightHandle *m_pLight = nullptr;

    UINT      m_commandListCount = 0;
    UINT      m_renderThreadCount = 0;
    ULONGLONG m_prevUpdateTick = 0;

    Character *m_pMainCharacter = nullptr;

    BoundingFrustum m_boundingFrustum;

    HWND m_hWnd = nullptr;

    bool m_isInitialized = false;
    bool m_activateCamera = true;

    // For Debugging
    IRenderSprite *m_pShadowMapSprite = nullptr;

  private:
    void LoadPrimitiveMeshes();
    void DeletePrimitiveMeshes();

    void Cleanup();

  public:
    // Derived from IRenderer
    BOOL Initialize(HWND hWnd) override;

    void OnKeyDown(UINT nChar, UINT uiScanCode) override;
    void OnKeyUp(UINT nChar, UINT uiScanCode) override;
    void OnMouseMove(int mouseX, int mouseY) override;
    BOOL OnUpdateWindowSize(UINT width, UINT height) override;

    IGameCharacter *CreateCharacter() override;
    IGameObject    *CreateGameObject() override;
    void            DeleteGameObject(IGameObject *pGameObj) override;
    void            DeleteAllGameObject() override;

    IGameModel *GetPrimitiveModel(PRIMITIVE_MODEL_TYPE type) override;
    IGameModel *CreateModelFromFile(const WCHAR *basePath, const WCHAR *filename) override;
    IGameModel *CreateEmptyModel() override;
    void        DeleteModel(IGameModel *pModel) override;
    void        DeleteAllModel() override;

    IGameSprite *CreateSpriteFromFile(const WCHAR *basePath, const WCHAR *filename, UINT width, UINT height) override;
    IGameSprite *CreateDynamicSprite(UINT width, UINT height) override;
    void         DeleteSprite(IGameSprite *pSprite) override;
    void         DeleteAllSprite() override;

    IGameAnimation *CreateAnimationFromFile(const WCHAR *basePath, const WCHAR *filename) override;
    IGameAnimation *CreateEmptyAnimation() override;
    IGameAnimation *GetAnimationByName(const WCHAR *name) override;
    void            DeleteAnimation(IGameAnimation *pInAnim) override;
    void            DeleteAllAnimation() override;

    void LoadResources();

    void ProcessInput();

    void PreUpdate(ULONGLONG curTick) override;
    void Update(ULONGLONG curTick) override;
    void LateUpdate(ULONGLONG curTick) override;

    void Render() override;

    void ToggleCamera();

    Vector3 GetCameraPos() override { return m_pMainCamera->Eye(); }
    Vector3 GetCameraLookAt() override { return m_pMainCamera->LookAt(); }

    inline IRenderer        *GetRenderer() const { return m_pRenderer; }
    inline IInputManager    *GetInputManager() const { return m_pInputManager; }
    inline CameraController *GetCamera() const { return m_pMainCamera; }
    inline Timer            *GetPerformanceTimer() const { return m_pPerformanceTimer; }

    GameEngine() = default;
    ~GameEngine();
};

extern GameEngine *g_pGame;