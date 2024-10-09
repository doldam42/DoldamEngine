#pragma once

#include <fbxsdk.h>
#include <string>
#include <vector>

#include "../MathModule/MathHeaders.h"
#include "../GenericModule/GenericHeaders.h"
#include "../EngineModule/EngineInterface.h"

#include "ModelExporterInterface.h"

namespace fs = std::filesystem;
using namespace std;

class FBXLoader : public IModelExporter
{
    struct CtrlPoint
    {
        Vector3 position;

        std::vector<uint8_t> boneIndices;
        std::vector<float>   boneWeights;

        CtrlPoint()
        {
            boneIndices.reserve(4);
            boneWeights.reserve(4);
        }
    };

    struct FaceGroup
    {
        string      materialName;
        vector<UINT> indices;  
    };

    CtrlPoint *m_pCtrlPointList = nullptr;

    const size_t MAX_WORKING_VERTEX_COUNT = 262144; // 65536

    IGameEngine          *m_pGame = nullptr;
    FbxManager           *m_pManager = nullptr;
    FbxScene             *m_pScene = nullptr;
    FbxGeometryConverter *m_pGeoConverter = nullptr;

    bool hasTangent = true;

  public:
    wchar_t         m_basePath[MAX_PATH] = {0};
    wchar_t         m_filename[MAX_NAME] = {0};
    IGameModel     *m_pModel = nullptr;
    IGameAnimation *m_pAnim = nullptr;

    std::vector<IGameMesh *> m_objects;
    std::vector<Material>    m_materials;

    bool                      m_isSkinned = true;
    std::vector<const char *> m_jointNames;
    std::vector<Joint>        m_joints;

    private:
    void Cleanup();

    int  FindMaterialIndexByName(const WCHAR *name);
    int  FindJointIndexUsingName(const WCHAR *inJointName);
    void FindDeformingJoints(FbxNode *pNode);
    bool IsDeformingJoint(const char *inJointName);

    void ProcessDeformingJointNode(FbxNode *inNode);
    void ProcessNode(FbxNode *inNode);
    void ProcessNodeRecursively(FbxNode *inNode, int inDepth, int myIndex, int inParentIndex);
    void ProcessAnimationNode(FbxNode *pNode);
    void ProcessSkeletonHierarchy(FbxNode *inRootNode);
    void ProcessSkeletonHierarchyRecursively(FbxNode *inNode, int inDepth, int myIndex, int inParentIndex);
    void ProcessMaterials();
    void ProcessMaterialAttribute(FbxSurfaceMaterial *inMaterial, Material *pOutMaterial);
    void ProcessMaterialTexture(FbxSurfaceMaterial *inMaterial, Material *pOutMaterial);
    void ProcessMesh(FbxNode *inNode, IGameMesh *pOutMesh);
    void ProcessCtrlPoints(FbxNode *inNode);
    void ProcessJoints(FbxNode *inNode);
    void ProcessJointAnimation(FbxNode *inNode);

    void ReadTangent(FbxMesh *inMesh, int inCtrlPointIndex, int inVertexCounter, Vector3 *pOutTangent);

    float   CalcRoughness(const Vector3 &inSpecular, float inShininess);
    float   CalcMetallic(const Vector3 &inSpecular, const Vector3 &inDiffuse);
    Vector3 CalcAlbedo(const Vector3 &inSpecular, const Vector3 &inDiffuse, float metallic);

    void CalcVerticeTangent(BasicVertex *pInOutVertices, UINT numVertices, const uint32_t *pIndices, UINT numTriangles);
    void CalcVerticeTangent(SkinnedVertex *pInOutVertices, UINT numVertices, const uint32_t *pIndices,
                            UINT numTriangles);

    void CtrlPointToSkinnedVertex(const CtrlPoint *pInCtrlPoint, SkinnedVertex *pOutVertex);

  public:
    // Inherited via IModelExporter
    BOOL Initialize(IGameEngine *pGame) override;

    BOOL Load(const WCHAR *basePath, const WCHAR *filename) override;
    BOOL LoadAnimation(const WCHAR *filename) override;

    IGameModel *GetModel() override;
    IGameAnimation *GetAnimation() override;

    void ExportModel() override;
    void ExportAnimation() override;

    FBXLoader() = default;
    ~FBXLoader();
};
