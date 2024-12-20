#pragma once

#include "EngineInterface.h"

#include "CameraController.h"
#include "GameUtils.h"
#include "InputManager.h"

class AnimationClip;
class Character;
class ControllerManager;
class GameObject;
class Model;
class PhysicsManager;
class World;
class GameManager : public IGameManager
{
  public:
    static Model *SquareMesh;
    static Model *BoxMesh;
    static Model *SphereMesh;

    static const UINT MAX_WORLD_OBJECT_COUNT = 1024;

    // For Debugging
    UINT m_culledObjectCountForDebug = 0;
    
  private:
    static UINT initRefCount;

    // Timer
    ULONGLONG m_prevUpdateTick = 0;
    ULONGLONG m_prevFrameCheckTick = 0;

    UINT m_FPS = 0;
    UINT m_frameCount = 0;

    float m_deltaTime = 0.0f;
    // Timer-For Debug
    float m_loadingTime = 0.0f;
    bool  m_isPaused = false;
    float m_timeSpeed = 1.0f;

    PhysicsManager   *m_pPhysicsManager = nullptr;
    CameraController *m_pMainCamera = nullptr;
    IRenderer        *m_pRenderer = nullptr;
    InputManager     *m_pInputManager = nullptr;

    // GameObjects
    SORT_LINK *m_pGameObjLinkHead = nullptr;
    SORT_LINK *m_pGameObjLinkTail = nullptr;

    World *m_pWorld = nullptr;

    // Model
    SORT_LINK *m_pModelLinkHead = nullptr;
    SORT_LINK *m_pModelLinkTail = nullptr;

    // Sprite
    SORT_LINK *m_pSpriteLinkHead = nullptr;
    SORT_LINK *m_pSpriteLinkTail = nullptr;

    // Animation
    HashTable *m_pAnimationHashTable = nullptr;

    // Controller Manager
    ControllerManager *m_pControllerManager = nullptr;

    ILightHandle *m_pLight = nullptr;

    UINT m_commandListCount = 0;
    UINT m_renderThreadCount = 0;

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
    void OnMouseWheel(float deltaWheel) override;

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

    void Register(IController *pController) override;

    BOOL LoadResources();

    void ProcessInput();
    void UpdatePhysics(float dt);

    void PreUpdate(ULONGLONG curTick);
    void Update(ULONGLONG curTick);
    void LateUpdate(ULONGLONG curTick);

    void Start() override;
    void Update() override;

    void BuildScene() override;

    float DeltaTime() const override { return m_deltaTime; }
    UINT  FPS() const override { return m_FPS; }

    void  SetTimeSpeed(float speed) override { m_timeSpeed = speed; }
    void TogglePause() override { m_isPaused = !m_isPaused; }

    void Render() override;

    void ToggleCamera();

    void    SetCameraFollowTarget(IGameObject *pObj) override { m_pMainCamera->SetFollowTarget((GameObject *)pObj); }
    Vector3 GetCameraPos() override { return m_pMainCamera->Eye(); }
    Vector3 GetCameraLookAt() override { return m_pMainCamera->LookAt(); }

    inline IRenderer        *GetRenderer() const { return m_pRenderer; }
    inline IInputManager    *GetInputManager() const { return m_pInputManager; }
    inline CameraController *GetCamera() const { return m_pMainCamera; }

    GameManager() = default;
    ~GameManager();
};

extern GameManager *g_pGame;