#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "../MathModule/MathHeaders.h"
#include "../Common/EngineInterface.h"

#include "../Common/ModelExporterInterface.h"


using namespace std;

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
enum aiTextureType;
class AssimpLoader : public IModelExporter
{
  private:
    IGameManager      *m_pGame = nullptr;

    ULONG m_refCount = 1;

  public:
    wchar_t         m_basePath[MAX_PATH] = {0};
    wchar_t         m_filename[MAX_NAME] = {0};
    IGameModel     *m_pModel = nullptr;
    IGameAnimation *m_pAnim = nullptr;

    std::vector<IGameMesh *> m_objects;
    std::vector<Material>    m_materials;

    bool m_isGLTF = false;
    bool m_revertNormals = false;

    const size_t MAX_WORKING_VERTEX_COUNT = 262144; // 65536

  private:
    void Cleanup();

    void CalcVerticeTangent(BasicVertex *pInOutVertices, UINT numVertices, const uint32_t *pIndices, UINT numTriangles);

    void ProcessNode(aiNode *node, const aiScene *scene, const Matrix& tr);

    void ProcessMesh(aiMesh *mesh, IGameMesh *pOutMesh, const aiScene *scene, const Matrix& tr);

    void        ProcessMaterials(const aiScene *scene);
    void        ProcessMaterialProperties(const aiMaterial *pInMaterial, Material *pOutMaterial);
    void        ProcessMaterialTextures(const aiMaterial *pInMaterial, Material *pOutMaterial, const aiScene *scene);
    std::string ReadTextureFilename(const aiScene *scene, const aiMaterial *material, aiTextureType type);

  public:
    // Inherited via IModelExporter
    BOOL            Initialize(IGameManager *pGame) override;
    BOOL            Load(const WCHAR *basePath, const WCHAR *filename) override;
    BOOL            LoadAnimation(const WCHAR *filename) override;
    IGameModel     *GetModel() override;
    IGameAnimation *GetAnimation() override;
    void            ExportModel() override;
    void            ExportAnimation() override;

    AssimpLoader() = default;
    ~AssimpLoader();

    // Inherited via IModelExporter
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;
};
