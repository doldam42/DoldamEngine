#pragma once

#include "../GenericModule/LinkedList.h"
#include "EngineInterface.h"
#include "BaseObject.h"
#include "MeshObject.h"

class GameObject;
class Model : public IGameModel
{
    static const UINT MAX_INSTANCE_COUNT = 256;

    IGameEngine *m_pGame = nullptr;
    IRenderer   *m_pRenderer = nullptr;

    WCHAR m_basePath[MAX_PATH] = {L'\0'};
    ULONG ref_count = 0;

    // File I/O
    UINT m_objectCount;
    UINT m_materialCount;

    Material    *m_pMaterials = nullptr;
    MeshObject **m_ppMeshObjects = nullptr;

  public:
    SORT_LINK m_LinkInGame;

  private:
    void Cleanup();

  public:
    void Initialize(const Material *pInMaterial, int materialCount, void **ppInObjs, int objectCount) override;
    void InitMeshHandles(IRenderer *pRenderer);

    void ReadFile(FILE *fp);
    void WriteFile(FILE *fp);

    void Render(GameObject *pGameObj);

    // Getter 
    inline UINT GetObjectCount() const { return m_objectCount; }
    inline MeshObject *GetObjectByIdx(UINT index) const { return m_ppMeshObjects[index]; }

    // Setter
    void SetBasePath(const WCHAR *basePath);

    // Inherited via IModel
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;

    Model() = default;
    ~Model();
};
