#pragma once

#include "BasicObject.h"
#include "CharacterObject.h"
#include "MeshObject.h"

#include "GameUtils.h"

#include "../MathModule/MathHeaders.h"
#include "../GenericModule/GenericHeaders.h"
#include "EngineInterface.h"

class IRenderer;
class IRenderMesh;
class GameObject;
struct MODEL_HEADER
{
    uint32_t ObjectCount;
    uint32_t MaterialCount;
};

class Model : public IModel
{
    static const UINT MAX_INSTANCE_COUNT = 256;
    wchar_t m_basePath[MAX_PATH] = {0};

    MODEL_HEADER m_header;

    ULONG ref_count = 0;

  public:
    BasicObject **m_ppObjs = nullptr;
    Material     *m_pMaterials = nullptr;
    SORT_LINK     m_LinkInGame;

  private:
    void Cleanup();

  public:
    void Initialize(const Material *pInMaterial, int materialCount, void **ppInObjs, int objectCount) override; 

    void WriteFile(FILE *fp) override;
    void ReadFile(FILE *fp) override;

    void InitMeshHandles(IRenderer *pRenderer);

    // Getter
    ULONG GetRefCount() const { return ref_count; }
    uint32_t GetObjectCount() const { return m_header.ObjectCount; }
    BasicObject *GetObjectByIdx(uint32_t index) const { return m_ppObjs[index]; }

    // Setter
    void SetBasePath(const WCHAR *basePath);

    void UpdateAnimation(AnimationClip *pClip, int frame);
    void Render(IRenderer *pRenderer, GameObject *pGameObj);

    Model();
    ~Model();

    // Derived from IUnknown
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;
};
