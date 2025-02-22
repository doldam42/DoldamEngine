#include "GameUtils.h"
#include <fbxsdk.h>
#include <filesystem>

#include "FBXLoader.h"

UINT AddVertex(vector<BasicVertex> &vertexList, const BasicVertex *pVertex)
{
    UINT foundIndex = -1;
    UINT existVertexCount = vertexList.size();
    for (DWORD i = 0; i < existVertexCount; i++)
    {
        const BasicVertex *pExistVertex = &vertexList[i];
        if (!memcmp(pExistVertex, pVertex, sizeof(BasicVertex)))
        {
            foundIndex = i;
            goto lb_return;
        }
    }
    // 새로운 vertex추가
    foundIndex = existVertexCount;
    vertexList.push_back(*pVertex);
lb_return:
    return foundIndex;
}

UINT AddSkinnedVertex(vector<SkinnedVertex> &vertexList, const SkinnedVertex *pVertex)
{
    UINT foundIndex = -1;
    UINT existVertexCount = vertexList.size();
    for (DWORD i = 0; i < existVertexCount; i++)
    {
        const SkinnedVertex *pExistVertex = &vertexList[i];
        if (!memcmp(pExistVertex, pVertex, sizeof(SkinnedVertex)))
        {
            foundIndex = i;
            goto lb_return;
        }
    }
    // 새로운 vertex추가
    foundIndex = existVertexCount;
    vertexList.push_back(*pVertex);
lb_return:
    return foundIndex;
}

FbxAMatrix GetGeometryTransformation(FbxNode *inNode)
{
    if (!inNode)
    {
        __debugbreak();
    }
    const FbxVector4 lT = inNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = inNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = inNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(lT, lR, lS);
}

Matrix ToMatrix(const FbxAMatrix &inMatrix)
{
    float m00 = static_cast<float>(inMatrix.Get(0, 0));
    float m01 = static_cast<float>(inMatrix.Get(0, 1));
    float m02 = static_cast<float>(inMatrix.Get(0, 2));
    float m03 = static_cast<float>(inMatrix.Get(0, 3));

    float m10 = static_cast<float>(inMatrix.Get(1, 0));
    float m11 = static_cast<float>(inMatrix.Get(1, 1));
    float m12 = static_cast<float>(inMatrix.Get(1, 2));
    float m13 = static_cast<float>(inMatrix.Get(1, 3));

    float m20 = static_cast<float>(inMatrix.Get(2, 0));
    float m21 = static_cast<float>(inMatrix.Get(2, 1));
    float m22 = static_cast<float>(inMatrix.Get(2, 2));
    float m23 = static_cast<float>(inMatrix.Get(2, 3));

    float m30 = static_cast<float>(inMatrix.Get(3, 0));
    float m31 = static_cast<float>(inMatrix.Get(3, 1));
    float m32 = static_cast<float>(inMatrix.Get(3, 2));
    float m33 = static_cast<float>(inMatrix.Get(3, 3));

    return Matrix(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
}

Vector3 ToVector3(const FbxVector4 &inVector)
{
    Vector3 v;
    v.x = static_cast<float>(inVector.mData[0]);
    v.y = static_cast<float>(inVector.mData[1]);
    v.z = static_cast<float>(inVector.mData[2]);
    return v;
}

Vector3 ToVector3(const FbxDouble3 &inVector)
{
    Vector3 v;
    v.x = static_cast<float>(inVector.mData[0]);
    v.y = static_cast<float>(inVector.mData[1]);
    v.z = static_cast<float>(inVector.mData[2]);
    return v;
}

Vector2 ToVector2(const FbxVector2 &inVector)
{
    Vector2 v;
    v.x = static_cast<float>(inVector.mData[0]);
    v.y = static_cast<float>(inVector.mData[1]);
    return v;
}

Transform ToTransform(const FbxAMatrix &inMatrix)
{
    const FbxQuaternion &inQ = inMatrix.GetQ();

    Quaternion q(inQ.mData[0], inQ.mData[1], inQ.mData[2], inQ.mData[3]);
    Vector3    t(ToVector3(inMatrix.GetT()));
    Vector3    s(ToVector3(inMatrix.GetS()));
    return Transform(t, q, s);
}

void FBXLoader::Cleanup()
{
    if (m_pGeoConverter)
    {
        delete m_pGeoConverter;
        m_pGeoConverter = nullptr;
    }
    if (m_pScene)
    {
        m_pScene->Destroy();
        m_pScene = nullptr;
    }
    if (m_pManager)
    {
        m_pManager->Destroy();
        m_pManager = nullptr;
    }
    if (m_pCtrlPointList)
    {
        delete[] m_pCtrlPointList;
        m_pCtrlPointList = nullptr;
    }

    if (m_pAnim)
    {
        m_pGame->DeleteAnimation(m_pAnim);
        m_pAnim = nullptr;
    }
    if (m_pModel)
    {
        m_pModel->Release();
        m_pModel = nullptr;
    }
}

BOOL FBXLoader::Initialize(IGameManager *pGame)
{
    m_pManager = FbxManager::Create();
    if (!m_pManager)
    {
        return false;
    }

    FbxIOSettings *fbxIOSettings = FbxIOSettings::Create(m_pManager, IOSROOT);

    m_pManager->SetIOSettings(fbxIOSettings);

    m_pScene = FbxScene::Create(m_pManager, "myScene");

    m_pGeoConverter = new FbxGeometryConverter(m_pManager);

    m_pGame = pGame;

    return true;
}

BOOL FBXLoader::Load(const WCHAR *basePath, const WCHAR *filename)
{
    if (!m_pScene)
    {
        m_pScene = FbxScene::Create(m_pManager, "myScene");
    }
    WCHAR wPath[MAX_PATH] = {L'\0'};
    char  path[MAX_PATH] = {'\0'};

    ZeroMemory(m_basePath, sizeof(m_basePath));
    ZeroMemory(m_filename, sizeof(m_filename));
    wcscpy_s(m_basePath, basePath);
    wcscpy_s(m_filename, filename);
    wcscpy_s(wPath, basePath);
    wcscat_s(wPath, filename);

    GameUtils::ws2s(wPath, path);

    FbxImporter *fbxImporter = FbxImporter::Create(m_pManager, "myImporter");
    if (!fbxImporter)
    {
        __debugbreak();
        return FALSE;
    }

    if (!fbxImporter->Initialize(path, -1, m_pManager->GetIOSettings()))
    {
        __debugbreak();
        return FALSE;
    }

    if (!fbxImporter->Import(m_pScene))
    {
        __debugbreak();
        return FALSE;
    }

    if (m_pModel)
    {
        m_pModel->Release();
        m_pModel = nullptr;
    }
    m_pModel = m_pGame->CreateEmptyModel();

    FbxAxisSystem::DirectX.DeepConvertScene(m_pScene);

    ProcessDeformingJointNode(m_pScene->GetRootNode());
    ProcessSkeletonHierarchy(m_pScene->GetRootNode());
    m_jointNames.clear();

    if (m_joints.empty())
    {
        m_isSkinned = false;
    }

    ProcessMaterials();
    ProcessNode(m_pScene->GetRootNode());

    if (m_isSkinned)
    {
        m_pModel->Initialize(m_materials.data(), m_materials.size(), m_objects.data(), m_objects.size(),
                             m_joints.data(), m_joints.size());
    }
    else
    {
        m_pModel->Initialize(m_materials.data(), m_materials.size(), m_objects.data(), m_objects.size());
    }

    fbxImporter->Destroy();
    fbxImporter = nullptr;

    m_pScene->Destroy();
    m_pScene = nullptr;
    return TRUE;
}

BOOL FBXLoader::LoadAnimation(const WCHAR *filename)
{
    if (!m_pAnim)
    {
        m_pAnim = m_pGame->CreateEmptyAnimation(filename);
    }
    else
    {
        m_pGame->DeleteAnimation(m_pAnim);
        m_pAnim = m_pGame->CreateEmptyAnimation(filename);
    }

    if (!m_isSkinned)
    {
        __debugbreak();
        return FALSE;
    }

    if (m_pAnim)
    {
        m_pAnim->Release();
        m_pAnim = nullptr;
    }
    m_pAnim = m_pGame->CreateEmptyAnimation(filename);

    WCHAR wcsPath[MAX_PATH] = {0};
    char  path[MAX_PATH] = {0};
    ZeroMemory(m_filename, sizeof(m_filename));
    wcscpy_s(m_filename, wcslen(filename) + 1, filename);

    wcscpy_s(wcsPath, m_basePath);
    wcscat_s(wcsPath, filename);
    GameUtils::ws2s(wcsPath, path);

    FbxImporter *fbxImporter = FbxImporter::Create(m_pManager, "myImporter");
    if (!fbxImporter)
    {
        __debugbreak();
        return FALSE;
    }
    if (!fbxImporter->Initialize(path, -1, m_pManager->GetIOSettings()))
    {
        __debugbreak();
        return FALSE;
    }
    m_pScene = FbxScene::Create(m_pManager, "myScene");
    if (!m_pScene)
    {
        __debugbreak();
        return FALSE;
    }
    if (!fbxImporter->Import(m_pScene))
    {
        __debugbreak();
        return FALSE;
    }

    FbxAxisSystem::DirectX.DeepConvertScene(m_pScene);

    m_pAnim->BeginCreateAnim(m_joints.size());
    ProcessAnimationNode(m_pScene->GetRootNode());
    m_pAnim->EndCreateAnim();

    fbxImporter->Destroy();
    fbxImporter = nullptr;
    m_pScene->Destroy();
    m_pScene = nullptr;

    return TRUE;
}

int FBXLoader::FindMaterialIndexByName(const WCHAR *name)
{
    for (int i = 0; i < m_materials.size(); i++)
    {
        if (wcscmp(m_materials[i].name, name) == 0)
        {
            return i;
        }
    }
    throw std::exception("No material\n");
}

int FBXLoader::FindJointIndexUsingName(const WCHAR *inJointName)
{
    for (unsigned int i = 0; i < m_joints.size(); ++i)
    {
        if (wcscmp(m_joints[i].name, inJointName) == 0)
        {
            return i;
        }
    }

    return -1;
}

void FBXLoader::FindDeformingJoints(FbxNode *pNode)
{
    FbxMesh *currMesh = pNode->GetMesh();

    unsigned int numOfDeformers = currMesh->GetDeformerCount();
    for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
    {
        FbxSkin *currSkin = reinterpret_cast<FbxSkin *>(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
        if (!currSkin)
        {
            continue;
        }

        unsigned int numOfClusters = currSkin->GetClusterCount();
        for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
        {
            FbxCluster *currCluster = currSkin->GetCluster(clusterIndex);
            m_jointNames.push_back(currCluster->GetLink()->GetName());
        }
    }
}

bool FBXLoader::IsDeformingJoint(const char *inJointName)
{
    for (size_t i = 0; i < m_jointNames.size(); i++)
    {
        if (!strcmp(inJointName, m_jointNames[i]))
        {
            return true;
        }
    }
    return false;
}

void FBXLoader::ProcessDeformingJointNode(FbxNode *inNode)
{
    FbxNodeAttribute *attr = inNode->GetNodeAttribute();
    if (attr != NULL)
    {
        switch (inNode->GetNodeAttribute()->GetAttributeType())
        {
        case FbxNodeAttribute::eMesh: {
            FindDeformingJoints(inNode);
        }
        break;
        }
    }

    for (int i = 0; i < inNode->GetChildCount(); ++i)
    {
        ProcessDeformingJointNode(inNode->GetChild(i));
    }
}

void FBXLoader::ProcessNode(FbxNode *inNode)
{
    for (int childIndex = 0; childIndex < inNode->GetChildCount(); ++childIndex)
    {
        FbxNode *currNode = inNode->GetChild(childIndex);
        ProcessNodeRecursively(currNode, 0, 0, -1);
    }
}

void FBXLoader::ProcessNodeRecursively(FbxNode *inNode, int inDepth, int myIndex, int inParentIndex)
{
    FbxNodeAttribute *attr = inNode->GetNodeAttribute();
    if (attr != NULL)
    {
        switch (inNode->GetNodeAttribute()->GetAttributeType())
        {
        case FbxNodeAttribute::eMesh: {
            m_pGeoConverter->Triangulate(inNode->GetMesh(), true);

            IGameMesh *pObj = m_pGame->CreateGameMesh();
            
            if (!pObj)
            {
                __debugbreak();
            }
            if (!inNode->GetMesh()->GetDeformerCount())
            {
                pObj->Initialize(MESH_TYPE_DEFAULT);
            }
            else
            {
                pObj->Initialize(MESH_TYPE_SKINNED);
            }

            pObj->SetParentIndex(inParentIndex);
            /*Transform tm(ToVector3(inNode->LclTranslation.Get()),
                         Quaternion::CreateFromYawPitchRoll(ToVector3(inNode->LclRotation.Get())), Vector3::One);*/
            // pObj->SetTransform();

            WCHAR wcsName[MAX_NAME] = {L'\0'};
            GameUtils::s2ws(inNode->GetName(), wcsName);
            pObj->SetName(wcsName);

            if (inParentIndex != -1)
            {
                m_objects[inParentIndex]->AddChildCount();
            }

            ProcessCtrlPoints(inNode);
            ProcessJoints(inNode);
            ProcessMesh(inNode, pObj);
            m_objects.push_back(pObj);
        }
        break;
        }
    }

    for (int i = 0; i < inNode->GetChildCount(); ++i)
    {
        ProcessNodeRecursively(inNode->GetChild(i), inDepth + 1, m_objects.size(), myIndex);
    }
}

void FBXLoader::ProcessAnimationNode(FbxNode *pNode)
{
    FbxNodeAttribute *attr = pNode->GetNodeAttribute();
    if (attr != NULL)
    {
        switch (pNode->GetNodeAttribute()->GetAttributeType())
        {
        case FbxNodeAttribute::eSkeleton:
            ProcessJointAnimation(pNode);
            break;
        }
    }

    for (int i = 0; i < pNode->GetChildCount(); ++i)
    {
        ProcessAnimationNode(pNode->GetChild(i));
    }
}

void FBXLoader::ProcessSkeletonHierarchy(FbxNode *inRootNode)
{
    for (int childIndex = 0; childIndex < inRootNode->GetChildCount(); ++childIndex)
    {
        FbxNode *currNode = inRootNode->GetChild(childIndex);
        ProcessSkeletonHierarchyRecursively(currNode, 0, 0, -1);
    }
    if (!m_joints.empty())
    {
        m_joints[0].parentIndex = -1;
    }
}

void FBXLoader::ProcessSkeletonHierarchyRecursively(FbxNode *inNode, int inDepth, int myIndex, int inParentIndex)
{
    if (inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() &&
        inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton &&
        IsDeformingJoint(inNode->GetName()))
    {
        Joint currJoint;
        currJoint.parentIndex = inParentIndex;
        GameUtils::s2ws(inNode->GetName(), currJoint.name);
        m_joints.push_back(currJoint);
    }
    for (int i = 0; i < inNode->GetChildCount(); i++)
    {
        ProcessSkeletonHierarchyRecursively(inNode->GetChild(i), inDepth + 1, m_joints.size(), myIndex);
    }
}

void FBXLoader::ProcessMaterials()
{
    int materialCount = m_pScene->GetMaterialCount();
    m_materials.resize(materialCount);
    for (int i = 0; i < materialCount; i++)
    {
        FbxSurfaceMaterial *surfaceMaterial = m_pScene->GetMaterial(i);
        Material           *pMaterial = &m_materials[i];
        ProcessMaterialAttribute(surfaceMaterial, pMaterial);
        ProcessMaterialTexture(surfaceMaterial, pMaterial);
    }
}

void FBXLoader::ProcessMaterialAttribute(FbxSurfaceMaterial *inMaterial, Material *pOutMaterial)
{
    FbxDouble3 double3;
    FbxDouble  double1;

    float metallic = 0.f;
    float roughness = 0.f;
    float opacity = 1.0f;
    float reflection = 0.0f;

    Vector3 Albedo;
    Vector3 diffuse;
    Vector3 emissive;

    GameUtils::s2ws(inMaterial->GetName(), pOutMaterial->name);

    if (inMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
    {
        FbxSurfacePhong *phong = reinterpret_cast<FbxSurfacePhong *>(inMaterial);

        // Diffuse Color
        double3 = phong->Diffuse;
        diffuse.x = static_cast<float>(double3.mData[0]);
        diffuse.y = static_cast<float>(double3.mData[1]);
        diffuse.z = static_cast<float>(double3.mData[2]);

        // Emissive Color
        double3 = phong->Emissive;
        emissive.x = static_cast<float>(double3.mData[0]);
        emissive.y = static_cast<float>(double3.mData[1]);
        emissive.z = static_cast<float>(double3.mData[2]);

        // Specular
        double3 = phong->Specular;
        Vector3 specular;
        specular.x = static_cast<float>(double3.mData[0]);
        specular.y = static_cast<float>(double3.mData[1]);
        specular.z = static_cast<float>(double3.mData[2]);

        double1 = phong->SpecularFactor;
        specular *= static_cast<float>(double1);

        // Shininess
        double1 = phong->Shininess;
        float shininess = static_cast<float>(double1);

        // Reflection
        double1 = phong->ReflectionFactor;
        reflection = static_cast<float>(double1);

        // Transparency
        double1 = phong->TransparencyFactor;
        opacity = 1.0f - static_cast<float>(double1);

        roughness = CalcRoughness(specular, shininess);
        metallic = CalcMetallic(specular, diffuse);
        Albedo = CalcAlbedo(specular, diffuse, metallic);
    }
    else if (inMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
    {
        FbxSurfaceLambert *lambert = reinterpret_cast<FbxSurfaceLambert *>(inMaterial);

        // Diffuse Color
        double3 = lambert->Diffuse;
        diffuse.x = static_cast<float>(double3.mData[0]);
        diffuse.y = static_cast<float>(double3.mData[1]);
        diffuse.z = static_cast<float>(double3.mData[2]);

        // Emissive Color
        double3 = lambert->Emissive;
        emissive.x = static_cast<float>(double3.mData[0]);
        emissive.y = static_cast<float>(double3.mData[1]);
        emissive.z = static_cast<float>(double3.mData[2]);

        // Reflection
        reflection = 0.0f; // 임의 설정

        // Transparency
        double1 = lambert->TransparencyFactor;
        opacity = 1.0f - static_cast<float>(double1);

        // Specular
        Vector3 specular(0.5f, 0.5f, 0.5f);

        // metallicFactor
        metallic = 0.5f;
        roughness = CalcRoughness(specular, 1.0f);
        Albedo = CalcAlbedo(specular, diffuse, metallic);
    }

    pOutMaterial->albedo = Albedo;
    pOutMaterial->emissive = emissive;
    pOutMaterial->opacityFactor = opacity;
    pOutMaterial->reflectionFactor = reflection;
    pOutMaterial->roughnessFactor = roughness;
    pOutMaterial->metallicFactor = metallic;
}

void FBXLoader::ProcessMaterialTexture(FbxSurfaceMaterial *inMaterial, Material *pOutMaterial)
{
    unsigned int textureIndex = 0;
    FbxProperty  property;

    FBXSDK_FOR_EACH_TEXTURE(textureIndex)
    {
        property = inMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[textureIndex]);
        if (property.IsValid())
        {
            unsigned int textureCount = property.GetSrcObjectCount<FbxTexture>();
            for (unsigned int i = 0; i < textureCount; ++i)
            {
                FbxLayeredTexture *layeredTexture = property.GetSrcObject<FbxLayeredTexture>(i);
                if (layeredTexture)
                {
                    throw std::exception("Layered Texture is currently unsupported\n");
                }
                else
                {
                    FbxTexture *texture = property.GetSrcObject<FbxTexture>(i);
                    if (texture)
                    {
                        std::string     textureType = property.GetNameAsCStr();
                        FbxFileTexture *fileTexture = FbxCast<FbxFileTexture>(texture);

                        if (fileTexture)
                        {
                            std::wstring textureName =
                                std::filesystem::path(fileTexture->GetFileName()).filename().wstring();
                            if (textureType == "DiffuseColor" || textureType == "BaseColor" ||
                                textureType == "AlbedoColor")
                            {
                                wcscpy_s(pOutMaterial->albedoTextureName, textureName.c_str());
                            }
                            else if (textureType == "SpecularColor")
                            {
                                // TODO;
                            }
                            else if (textureType == "Bump" || textureType == "NormalMap")
                            {
                                wcscpy_s(pOutMaterial->normalTextureName, textureName.c_str());
                            }
                            else if (textureType == "AmbientColor")
                            {
                                wcscpy_s(pOutMaterial->aoTextureName, textureName.c_str());
                            }
                            else if (textureType == "EmmisiveColor")
                            {
                                wcscpy_s(pOutMaterial->emissiveTextureName, textureName.c_str());
                            }
                            // fbx sdk에서 지원 안하는 포맷.. 커스텀 포맷으로 import 가능
                            else if (textureType == "MetallicColor")
                            {
                                wcscpy_s(pOutMaterial->metallicTextureName, textureName.c_str());
                            }
                            else if (textureType == "RoughnessColor")
                            {
                                wcscpy_s(pOutMaterial->roughnessTextureName, textureName.c_str());
                            }
                            else if (textureType == "ShininessExponent")
                            {
                                // TODO
                            }
                        }
                    }
                }
            }
        }
    }
}

void FBXLoader::ProcessMesh(FbxNode *inNode, IGameMesh *pOutMesh)
{
    FbxMesh *currMesh = inNode->GetMesh();

    FbxLayerElementArrayTemplate<int> &materialIndices = currMesh->GetElementMaterial()->GetIndexArray();
    FbxGeometryElement::EMappingMode   materialMappingMode = currMesh->GetElementMaterial()->GetMappingMode();

    int  ctrlPointCount = currMesh->GetControlPointsCount();
    int  numFaces = inNode->GetMaterialCount();
    UINT triangleCount = currMesh->GetPolygonCount();

    vector<SkinnedVertex> skinnedVertices;
    vector<BasicVertex>   basicVertices;

    FaceGroup *pFaceGroups = new FaceGroup[numFaces];
    for (int i = 0; i < numFaces; i++)
    {
        const char *matName = inNode->GetMaterial(i)->GetName();
        pFaceGroups[i].indices.reserve(static_cast<size_t>(triangleCount * 3));
        pFaceGroups[i].materialName = string(matName);
    }

    if (m_isSkinned)
    {
        skinnedVertices.reserve(ctrlPointCount);
    }
    else
    {
        basicVertices.reserve(ctrlPointCount);
    }

    int  vertexCounter = 0;
    UINT indices[3] = {0};
    for (UINT i = 0; i < triangleCount; i++)
    {
        int materialIndex = materialIndices.GetAt(i);
        for (UINT j = 0; j < 3; j++) // 폴리곤 = 삼각형(점 3개)
        {
            DWORD      index;
            int        ctrlPointIndex = currMesh->GetPolygonVertex(i, j);
            CtrlPoint *pCtrlPoint = &m_pCtrlPointList[ctrlPointIndex];

            FbxStringList strList;
            bool          isUnmapped;
            FbxVector4    normal;
            FbxVector2    uv;
            Vector3       tangent(0.0f);

            currMesh->GetPolygonVertexNormal(i, j, normal);
            currMesh->GetUVSetNames(strList);
            currMesh->GetPolygonVertexUV(i, j, strList.GetStringAt(0), uv, isUnmapped);
            
            ReadTangent(currMesh, ctrlPointIndex, vertexCounter, &tangent);

            if (m_isSkinned)
            {
                SkinnedVertex vertex;
                CtrlPointToSkinnedVertex(pCtrlPoint, &vertex);
                vertex.normal = ToVector3(normal);

                //vertex.normal *= -1.0f;

                vertex.position = pCtrlPoint->position;
                vertex.texcoord.x = static_cast<float>(uv.mData[0]);
                vertex.texcoord.y = 1.0f - static_cast<float>(uv.mData[1]);
                vertex.tangent = tangent;
                index = AddSkinnedVertex(skinnedVertices, &vertex);
            }
            else
            {
                BasicVertex vertex;

                vertex.normal = ToVector3(normal);

                //vertex.normal *= -1.0f;

                vertex.position = pCtrlPoint->position;
                vertex.texcoord.x = static_cast<float>(uv.mData[0]);
                vertex.texcoord.y = 1.0f - static_cast<float>(uv.mData[1]);
                vertex.tangent = tangent;
                index = AddVertex(basicVertices, &vertex);
            }
            pFaceGroups[materialIndex].indices.push_back(index);
        }

        vertexCounter++;
    }

    if (m_isSkinned)
    {
        if (!hasTangent)
        {
            for (UINT i = 0; i < numFaces; i++)
            {
                FaceGroup &face = pFaceGroups[i];
                CalcVerticeTangent(skinnedVertices.data(), skinnedVertices.size(), face.indices.data(),
                                   face.indices.size() / 3);
            }
        }

        pOutMesh->BeginCreateMesh(skinnedVertices.data(), skinnedVertices.size(), numFaces);
    }
    else
    {
        if (!hasTangent)
        {
            for (UINT i = 0; i < numFaces; i++)
            {
                FaceGroup &face = pFaceGroups[i];
                CalcVerticeTangent(basicVertices.data(), basicVertices.size(), face.indices.data(),
                                   face.indices.size() / 3);
            }
        }

        pOutMesh->BeginCreateMesh(basicVertices.data(), basicVertices.size(), numFaces);
    }

    WCHAR wName[MAX_NAME];
    for (UINT i = 0; i < numFaces; i++)
    {
        ZeroMemory(wName, sizeof(wName));
        FaceGroup &face = pFaceGroups[i];
        GameUtils::s2ws(face.materialName.c_str(), wName);

        int materialIndex = FindMaterialIndexByName(wName);

        pOutMesh->InsertFaceGroup(face.indices.data(), face.indices.size() / 3, materialIndex);
    }

    if (pFaceGroups)
    {
        delete[] pFaceGroups;
        pFaceGroups = nullptr;
    }
}

void FBXLoader::ProcessCtrlPoints(FbxNode *inNode)
{
    FbxMesh *currMesh = inNode->GetMesh();
    int      ctrlPointCount = currMesh->GetControlPointsCount();
    m_pCtrlPointList = new CtrlPoint[ctrlPointCount];
    for (int i = 0; i < ctrlPointCount; i++)
    {
        FbxVector4 pos = currMesh->GetControlPointAt(i);
        m_pCtrlPointList[i].position = ToVector3(pos);
    }
}

void FBXLoader::ProcessJoints(FbxNode *inNode)
{
    WCHAR    tmpName[MAX_PATH] = {0};
    FbxMesh *currMesh = inNode->GetMesh();

    FbxAMatrix geometryTransform = GetGeometryTransformation(inNode);

    unsigned int numOfDeformers = currMesh->GetDeformerCount();
    for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
    {
        // There are many types of deformers in Maya,
        // We are using only skins, so we see if this is a skin
        FbxSkin *currSkin = reinterpret_cast<FbxSkin *>(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
        if (!currSkin)
        {
            continue;
        }

        unsigned int numOfClusters = currSkin->GetClusterCount();
        for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
        {
            FbxCluster *currCluster = currSkin->GetCluster(clusterIndex);
            memset(tmpName, 0, sizeof(tmpName));
            GameUtils::s2ws(currCluster->GetLink()->GetName(), tmpName);
            unsigned int currJointIndex = FindJointIndexUsingName(tmpName);
            FbxAMatrix   transformMatrix;
            FbxAMatrix   transformLinkMatrix;
            FbxAMatrix   globalBindposeInverseMatrix;

            currCluster->GetTransformMatrix(transformMatrix);         // The transformation of the mesh at binding time
            currCluster->GetTransformLinkMatrix(transformLinkMatrix); // The transformation of the cluster(joint) at

            globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;
            m_joints[currJointIndex].globalBindposeInverse = ToMatrix(globalBindposeInverseMatrix);

            // Associate each joint with the control points it affects
            unsigned int numOfIndices = currCluster->GetControlPointIndicesCount();

            for (unsigned int i = 0; i < numOfIndices; ++i)
            {
                int vertexIdx = currCluster->GetControlPointIndices()[i];
                m_pCtrlPointList[vertexIdx].boneIndices.push_back(currJointIndex);
                m_pCtrlPointList[vertexIdx].boneWeights.push_back(currCluster->GetControlPointWeights()[i]);
            }
        }
    }
}

void FBXLoader::ProcessJointAnimation(FbxNode *inNode)
{
    WCHAR jointName[MAX_NAME] = {L'\0'};
    GameUtils::s2ws(inNode->GetName(), jointName);
    if (FindJointIndexUsingName(jointName) == -1)
    {
        return;
    }

    FbxAnimStack *currAnimStack = m_pScene->GetSrcObject<FbxAnimStack>(0);
    FbxString     animStackName = currAnimStack->GetName();

    // GameUtils::s2ws(animStackName.Buffer(), pClip->name);

    FbxTakeInfo *takeInfo = m_pScene->GetTakeInfo(animStackName);
    FbxTime      start = takeInfo->mLocalTimeSpan.GetStart();
    FbxTime      end = takeInfo->mLocalTimeSpan.GetStop();
    uint32_t     frameCount = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
    Matrix      *pKeys = new Matrix[frameCount];

    for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); ++i)
    {
        FbxTime currTime;
        currTime.SetFrame(i, FbxTime::eFrames24);
        FbxAMatrix currentTransformOffset = inNode->EvaluateLocalTransform(currTime);
        pKeys[i - start.GetFrameCount(FbxTime::eFrames24)] = ToMatrix(currentTransformOffset);
    }

    m_pAnim->InsertKeyframes(jointName, pKeys, frameCount);
}

void FBXLoader::ReadTangent(FbxMesh *inMesh, int inCtrlPointIndex, int inVertexCounter, Vector3 *pOutTangent)
{
    if (inMesh->GetElementTangentCount() < 1)
    {
        hasTangent = false;
        return;
    }

    FbxGeometryElementTangent *vertexTangent = inMesh->GetElementTangent(0);
    switch (vertexTangent->GetMappingMode())
    {
    case FbxGeometryElement::eByControlPoint:
        switch (vertexTangent->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect: {
            pOutTangent->x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[0]);
            pOutTangent->y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[1]);
            pOutTangent->z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inCtrlPointIndex).mData[2]);
        }
        break;

        case FbxGeometryElement::eIndexToDirect: {
            int index = vertexTangent->GetIndexArray().GetAt(inCtrlPointIndex);
            pOutTangent->x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
            pOutTangent->y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
            pOutTangent->z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
        }
        break;

        default:
            throw std::exception("Invalid Reference");
        }
        break;

    case FbxGeometryElement::eByPolygonVertex:
        switch (vertexTangent->GetReferenceMode())
        {
        case FbxGeometryElement::eDirect: {
            pOutTangent->x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[0]);
            pOutTangent->y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[1]);
            pOutTangent->z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(inVertexCounter).mData[2]);
        }
        break;

        case FbxGeometryElement::eIndexToDirect: {
            int index = vertexTangent->GetIndexArray().GetAt(inVertexCounter);
            pOutTangent->x = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[0]);
            pOutTangent->y = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[1]);
            pOutTangent->z = static_cast<float>(vertexTangent->GetDirectArray().GetAt(index).mData[2]);
        }
        break;

        default:
            throw std::exception("Invalid Reference");
        }
        break;
    }
}

// Calc Metallic/Roughness
// ref: https://learn.microsoft.com/en-us/azure/remote-rendering/reference/material-mapping
float FBXLoader::CalcRoughness(const Vector3 &inSpecular, float inShininess)
{
    float specularIntensity = inSpecular.x * 0.2125f + inSpecular.y * 0.7154f + inSpecular.z * 0.0721f;

    float roughness = sqrt(2.0f / (inShininess * specularIntensity + 2.0f));
    return roughness;
}

float FBXLoader::CalcMetallic(const Vector3 &inSpecular, const Vector3 &inDiffuse)
{
    float diffuseBrightness =
        0.299f * inDiffuse.x * inDiffuse.x + 0.587f * inDiffuse.y * inDiffuse.y + 0.114f * inDiffuse.z * inDiffuse.z;

    float specularBrightness = 0.299f * inSpecular.x * inSpecular.x + 0.587f * inSpecular.y * inSpecular.y +
                               0.114f * inSpecular.z * inSpecular.z;

    float specularStrength = max(max(inSpecular.x, inSpecular.y), inSpecular.z);

    // 4% 반사의 경우
    // 아래 방정식은 금속체 표면에서만 성립.
    // 고무/플라스틱의 경우 실제와 다르게 보일 수 있음.
    float dielectricSpecularReflectance = 0.04f;
    float oneMinusSpecularStrength = 1.0f - specularStrength;

    float A = dielectricSpecularReflectance;
    float B = (diffuseBrightness * (oneMinusSpecularStrength / (1 - A)) + specularBrightness) - 2 * A;
    float C = A - specularBrightness;
    float squareRoot = sqrt(max(0.0, B * B - 4 * A * C));
    float value = (-B + squareRoot) / (2 * A);
    float metallic = std::clamp(value, 0.0f, 1.0f);

    return metallic;
}

Vector3 FBXLoader::CalcAlbedo(const Vector3 &inSpecular, const Vector3 &inDiffuse, float metallic)
{
    Vector3 dielectricColor = inDiffuse;

    float dielectricSpecularReflectance = 0.04;
    float specularStrength = max(max(inSpecular.x, inSpecular.y), inSpecular.z);

    float oneMinusSpecularStrength = 1 - specularStrength;

    dielectricColor *= (oneMinusSpecularStrength / (1.0f - dielectricSpecularReflectance) / max(1e-4, 1.0f - metallic));

    float   A = dielectricSpecularReflectance * (1.0f - metallic);
    Vector3 metalColor = Vector3(inSpecular.x - A, inSpecular.y - A, inSpecular.z - A);
    metalColor *= static_cast<float>(1.0f / max(1e-4f, metallic));
    Vector3 AlbedoRGB = Vector3::Lerp(dielectricColor, metalColor, metallic * metallic);
    AlbedoRGB.Clamp(Vector3::Zero, Vector3::One);

    return AlbedoRGB;
}

void FBXLoader::CalcVerticeTangent(BasicVertex *pInOutVertices, UINT numVertices, const uint32_t *pIndices,
                                   UINT numTriangles)
{
    Vector3 *tan1 = new Vector3[numVertices * 2];
    Vector3 *tan2 = tan1 + numVertices;
    ZeroMemory(tan1, numVertices * sizeof(Vector3) * 2);

    for (long i = 0; i < numTriangles; i++)
    {
        long i1 = pIndices[i * 3];
        long i2 = pIndices[i * 3 + 1];
        long i3 = pIndices[i * 3 + 2];

        const Vector3 v1 = pInOutVertices[i1].position;
        const Vector3 v2 = pInOutVertices[i2].position;
        const Vector3 v3 = pInOutVertices[i3].position;

        const Vector2 w1 = pInOutVertices[i1].texcoord;
        const Vector2 w2 = pInOutVertices[i2].texcoord;
        const Vector2 w3 = pInOutVertices[i3].texcoord;

        float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;

        float s1 = w2.x - w1.x;
        float s2 = w3.x - w1.x;
        float t1 = w2.y - w1.y;
        float t2 = w3.y - w1.y;

        float   r = 1.0F / (s1 * t2 - s2 * t1);
        Vector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
        Vector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;

        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;
    }

    for (long a = 0; a < numTriangles * 3; a++)
    {
        long          index = pIndices[a];
        const Vector3 n = pInOutVertices[index].normal;
        const Vector3 t = tan1[index];

        Vector3 tangent = t - n * n.Dot(t);
        tangent.Normalize();

        // Gram-Schmidt orthogonalize
        pInOutVertices[index].tangent = tangent;
    }

    delete[] tan1;
}

void FBXLoader::CalcVerticeTangent(SkinnedVertex *pInOutVertices, UINT numVertices, const uint32_t *pIndices,
                                   UINT numTriangles)
{
    Vector3 *tan1 = new Vector3[numVertices * 2];
    Vector3 *tan2 = tan1 + numVertices;
    ZeroMemory(tan1, numVertices * sizeof(Vector3) * 2);

    for (long i = 0; i < numTriangles; i++)
    {
        long i1 = pIndices[i * 3];
        long i2 = pIndices[i * 3 + 1];
        long i3 = pIndices[i * 3 + 2];

        const Vector3 v1 = pInOutVertices[i1].position;
        const Vector3 v2 = pInOutVertices[i2].position;
        const Vector3 v3 = pInOutVertices[i3].position;

        const Vector2 w1 = pInOutVertices[i1].texcoord;
        const Vector2 w2 = pInOutVertices[i2].texcoord;
        const Vector2 w3 = pInOutVertices[i3].texcoord;

        float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;

        float s1 = w2.x - w1.x;
        float s2 = w3.x - w1.x;
        float t1 = w2.y - w1.y;
        float t2 = w3.y - w1.y;

        float   r = 1.0F / (s1 * t2 - s2 * t1);
        Vector3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
        Vector3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;

        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;
    }

    for (long a = 0; a < numTriangles * 3; a++)
    {
        long          index = pIndices[a];
        const Vector3 n = pInOutVertices[index].normal;
        const Vector3 t = tan1[index];

        Vector3 tangent = t - n * n.Dot(t);
        tangent.Normalize();

        // Gram-Schmidt orthogonalize
        pInOutVertices[index].tangent = tangent;
    }
    
    delete[] tan1;
}

void FBXLoader::CtrlPointToSkinnedVertex(const CtrlPoint *pInCtrlPoint, SkinnedVertex *pOutVertex)
{
    int numIndices = pInCtrlPoint->boneWeights.size();
    int argSortedIndices[8] = {0};

    if (numIndices > 4)
    {
        for (int i = 0; i < 8; i++)
        {
            argSortedIndices[i] = i;
        }
        GameUtils::argsort(pInCtrlPoint->boneWeights.data(), argSortedIndices, numIndices);

        for (int i = 0; i < 4; i++)
        {
            pOutVertex->blendWeights[i] = pInCtrlPoint->boneWeights[argSortedIndices[i]];
            pOutVertex->boneIndices[i] = pInCtrlPoint->boneIndices[argSortedIndices[i]];
        }
        for (int i = 4; i < numIndices; i++)
        {
            pOutVertex->blendWeights[0] += pInCtrlPoint->boneWeights[argSortedIndices[i]];
        }
    }
    else
    {
        for (int i = 0; i < numIndices; i++)
        {
            pOutVertex->blendWeights[i] = pInCtrlPoint->boneWeights[i];
            pOutVertex->boneIndices[i] = pInCtrlPoint->boneIndices[i];
        }
    }
    // 합이 1이 되도록 정규화
    float sum = 0.f;
    for (int i = 0; i < 4; i++)
    {
        sum += pOutVertex->blendWeights[i];
    }
    for (int i = 0; i < 4; i++)
    {
        pOutVertex->blendWeights[i] /= sum;
    }
}

IGameModel *FBXLoader::GetModel()
{
    m_pModel->AddRef();
    return m_pModel;
}

IGameAnimation *FBXLoader::GetAnimation()
{
    m_pAnim->AddRef();
    return m_pAnim;
}

void FBXLoader::ExportAnimation()
{
    if (m_pAnim)
    {
        wchar_t fullPath[MAX_PATH] = {L'\0'};
        char    outPath[MAX_PATH] = {L'\0'};
        wcscpy_s(fullPath, m_basePath);
        wcscat_s(fullPath, m_filename);

        fs::path p(fullPath);
        p.replace_extension("dca");
        GameUtils::ws2s(p.c_str(), outPath);

        FILE *fp = nullptr;
        fopen_s(&fp, outPath, "wb");
        if (!fp)
        {
            __debugbreak();
            return;
        }
        m_pAnim->WriteFile(fp);
        fclose(fp);
    }
}

void FBXLoader::ExportModel()
{
    if (m_pModel)
    {
        wchar_t fullPath[MAX_PATH] = {L'\0'};
        char    outPath[MAX_PATH] = {L'\0'};
        wcscpy_s(fullPath, m_basePath);
        wcscat_s(fullPath, m_filename);

        fs::path p(fullPath);
        p.replace_extension("dom");
        GameUtils::ws2s(p.c_str(), outPath);

        FILE *fp = nullptr;
        fopen_s(&fp, outPath, "wb");
        if (!fp)
        {
            __debugbreak();
            return;
        }
        m_pModel->WriteFile(fp);
        fclose(fp);
    }
}

FBXLoader::~FBXLoader()
{
    Cleanup();
}

HRESULT __stdcall FBXLoader::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall FBXLoader::AddRef(void)
{
    m_refCount++;
    return m_refCount;
}

ULONG __stdcall FBXLoader::Release(void)
{
    ULONG ref_count = --m_refCount;
    if (!m_refCount)
        delete this;

    return ref_count;
}
