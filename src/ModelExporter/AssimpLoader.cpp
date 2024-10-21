#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <fstream>
#include <filesystem>

#include "GameUtils.h"

#include "AssimpLoader.h"

namespace fs = std::filesystem;

string GetExtension(const string filename)
{
    string ext(filesystem::path(filename).extension().string());
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

void AssimpLoader::Cleanup()
{
    if (m_pModel)
    {
        m_pModel->Release();
        m_pModel = nullptr;
    }
    if (m_pAnim)
    {
        m_pAnim->Release();
        m_pAnim = nullptr;
    }
}

void AssimpLoader::CalcVerticeTangent(BasicVertex *pInOutVertices, UINT numVertices, const uint32_t *pIndices,
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


void AssimpLoader::ProcessNode(aiNode *node, const aiScene *scene, const Matrix &tr)
{
    Matrix m(&node->mTransformation.a1);
    m = m.Transpose() * tr;

    for (UINT i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

        IGameMesh *pObj = nullptr;
        if (!CreateGameMesh(&pObj))
        {
            __debugbreak();
        }
        pObj->Initialize(MESH_TYPE_DEFAULT);

        WCHAR wcsName[MAX_NAME] = {L'\0'};
        GameUtils::s2ws(node->mName.C_Str(), wcsName);
        pObj->SetName(wcsName);

        ProcessMesh(mesh, pObj, scene, m);

        m_objects.push_back(pObj);
    }

    for (UINT i = 0; i < node->mNumChildren; i++)
    {
        this->ProcessNode(node->mChildren[i], scene, m);
    }
}

// Mesh 하나에 FaceGroup 하나씩
void AssimpLoader::ProcessMesh(aiMesh *mesh, IGameMesh *pOutMesh, const aiScene *scene, const Matrix &tr)
{
    UINT             numVertice = mesh->mNumVertices;
    BasicVertex     *pVertice = new BasicVertex[numVertice];
    vector<UINT> indices;
    indices.reserve(mesh->mFaces[0].mNumIndices);

    for (UINT i = 0; i < mesh->mNumVertices; i++)
    {
        BasicVertex &vertex = pVertice[i];

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        vertex.normal.x = mesh->mNormals[i].x;
        if (m_isGLTF)
        {
            vertex.normal.y = mesh->mNormals[i].z;
            vertex.normal.z = -mesh->mNormals[i].y;
        }
        else
        {
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        if (m_revertNormals)
        {
            vertex.normal *= -1.0f;
        }

        vertex.normal.Normalize();

        if (mesh->mTextureCoords[0])
        {
            vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
            vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
        }
    }

    for (UINT i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (UINT j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if (numVertice == 0 || indices.size() == 0)
    {
        __debugbreak();
    }

    for (UINT i = 0; i < numVertice; i++)
    {
        Vector3 pos = pVertice[i].position;
        pVertice[i].position = Vector3::Transform(pos, tr);
    }
    CalcVerticeTangent(pVertice, numVertice, indices.data(), static_cast<UINT>(indices.size() / 3));
    
    pOutMesh->BeginCreateMesh(pVertice, numVertice, 1);
    pOutMesh->InsertFaceGroup(indices.data(), static_cast<UINT>(indices.size() / 3), mesh->mMaterialIndex);
    pOutMesh->EndCreateMesh();

    if (pVertice)
    {
        delete[] pVertice;
        pVertice = nullptr;
    }
    indices.clear();
}

void AssimpLoader::ProcessMaterials(const aiScene *scene)
{
    if (!scene->HasMaterials())
        return;

    /*string opacityTextureFilename;
    string heightTextureFilename;*/

    uint32_t     numMaterials = scene->mNumMaterials;
    aiMaterial **ppMaterials = scene->mMaterials;

    m_materials.resize(numMaterials);
    for (uint32_t i = 0; i < numMaterials; i++)
    {
        aiMaterial *pSrc = ppMaterials[i];
        Material   *pDest = &m_materials[i];
        memset(pDest, 0, sizeof(Material));

        ProcessMaterialProperties(pSrc, pDest);
        ProcessMaterialTextures(pSrc, pDest, scene);
    }
}

static Vector3 ToVector3(const aiColor3D &inColor)
{
    float r = static_cast<float>(inColor.r);
    float g = static_cast<float>(inColor.g);
    float b = static_cast<float>(inColor.b);

    return Vector3(r, g, b);
}

void AssimpLoader::ProcessMaterialProperties(const aiMaterial *pInMaterial, Material *pOutMaterial)
{
    aiColor3D colorAmbient;
    aiColor3D colorAlbedo;
    aiColor3D colorSpecular;
    aiColor3D colorEmissive;
    aiColor3D colorTransparent;
    float     opacity = 1.0f;
    float     metallic = 0;
    float     roughness = 0;

    pInMaterial->Get(AI_MATKEY_COLOR_AMBIENT, colorAmbient);
    pInMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, colorAlbedo);
    pInMaterial->Get(AI_MATKEY_COLOR_SPECULAR, colorSpecular);
    pInMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, colorEmissive);
    pInMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, colorTransparent);
    pInMaterial->Get(AI_MATKEY_OPACITY, opacity);
    pInMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
    pInMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);

    pOutMaterial->albedo = ToVector3(colorAlbedo);
    pOutMaterial->emissive = ToVector3(colorEmissive);
    pOutMaterial->metallicFactor = metallic;
    pOutMaterial->roughnessFactor = roughness;
    pOutMaterial->opacityFactor = colorTransparent.IsBlack() ? opacity : 1.0f;
    pOutMaterial->reflectionFactor = 0.0f;
}

void AssimpLoader::ProcessMaterialTextures(const aiMaterial *pInMaterial, Material *pOutMaterial, const aiScene *scene)
{
    string albedoTextureFilename = ReadTextureFilename(scene, pInMaterial, aiTextureType_BASE_COLOR);
    if (albedoTextureFilename.empty())
    {
        albedoTextureFilename = ReadTextureFilename(scene, pInMaterial, aiTextureType_DIFFUSE);
    }
    string emissiveTextureFilename = ReadTextureFilename(scene, pInMaterial, aiTextureType_EMISSIVE);
    string normalTextureFilename = ReadTextureFilename(scene, pInMaterial, aiTextureType_NORMALS);
    string metallicTextureFilename = ReadTextureFilename(scene, pInMaterial, aiTextureType_METALNESS);
    string roughnessTextureFilename = ReadTextureFilename(scene, pInMaterial, aiTextureType_DIFFUSE_ROUGHNESS);
    string aoTextureFilename = ReadTextureFilename(scene, pInMaterial, aiTextureType_AMBIENT_OCCLUSION);
    if (aoTextureFilename.empty())
    {
        aoTextureFilename = ReadTextureFilename(scene, pInMaterial, aiTextureType_LIGHTMAP);
    }

    GameUtils::s2ws(pInMaterial->GetName().C_Str(), pOutMaterial->name);
    GameUtils::s2ws(albedoTextureFilename.c_str(), pOutMaterial->albedoTextureName);
    GameUtils::s2ws(emissiveTextureFilename.c_str(), pOutMaterial->emissiveTextureName);
    GameUtils::s2ws(normalTextureFilename.c_str(), pOutMaterial->normalTextureName);
    GameUtils::s2ws(metallicTextureFilename.c_str(), pOutMaterial->metallicTextureName);
    GameUtils::s2ws(roughnessTextureFilename.c_str(), pOutMaterial->roughnessTextureName);
    GameUtils::s2ws(aoTextureFilename.c_str(), pOutMaterial->aoTextureName);
}

std::string AssimpLoader::ReadTextureFilename(const aiScene *scene, const aiMaterial *material, aiTextureType type)
{
    if (material->GetTextureCount(type) > 0)
    {
        aiString filepath;
        material->GetTexture(type, 0, &filepath);

        char basePath[MAX_PATH];
        memset(basePath, 0, sizeof(basePath));
        GameUtils::ws2s(m_basePath, basePath);

        string filename = fs::path(filepath.C_Str()).filename().string();
        string fullPath = string(basePath) + filename;


        fs::path ddsPath(fullPath);
        ddsPath.replace_extension(".dds");
        if (filesystem::exists(ddsPath))
            return ddsPath.filename().string();

        // 1. 실제로 파일이 존재하는지 확인
        if (!filesystem::exists(fullPath))
        {
            // 2. 파일이 없을 경우 혹시 fbx 자체에 Embedded인지 확인
            const aiTexture *texture = scene->GetEmbeddedTexture(filepath.C_Str());
            if (texture)
            {
                // 3. Embedded texture가 존재하고 png일 경우 저장
                if (string(texture->achFormatHint).find("png") != string::npos)
                {
                    ofstream fs(fullPath.c_str(), ios::binary | ios::out);
                    fs.write((char *)texture->pcData, texture->mWidth);
                    fs.close();
                    // 참고: compressed format일 경우 texture->mHeight가 0
                }
            }
            else
            {

                cout << fullPath << " doesn't exists. Return empty filename." << endl;
            }
        }
        else
        {
            return filename;
        }

        return filename;
    }
    else
    {
        return "";
    }
}

BOOL AssimpLoader::Initialize(IGameEngine *pGame)
{
    m_pGame = pGame;
    return TRUE;
}

BOOL AssimpLoader::Load(const WCHAR *basePath, const WCHAR *filename)
{
    Assimp::Importer importer;
    ZeroMemory(m_basePath, sizeof(m_basePath));
    ZeroMemory(m_filename, sizeof(m_filename));

    WCHAR wcsFullPath[MAX_PATH] = {L'\0'};
    char  fullPath[MAX_PATH] = {'\0'};
    wcscpy_s(wcsFullPath, basePath);
    wcscat_s(wcsFullPath, filename);
    GameUtils::ws2s(wcsFullPath, fullPath);

    /*if (wcsstr(filename, L"gltf") != NULL)
    {
        m_isGLTF = true;
    }*/

    const aiScene *pScene = importer.ReadFile(fullPath, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (!pScene)
    {
        std::cout << "Failed to read file: " << fullPath << std::endl;
        auto errorDescription = importer.GetErrorString();
        std::cout << "Assimp error: " << errorDescription << endl;
        return FALSE;
    }
    if (pScene)
    {
        wcscpy_s(m_basePath, basePath);
        wcscpy_s(m_filename, filename);

        m_pModel = m_pGame->CreateEmptyModel();

        ProcessMaterials(pScene);

        ProcessNode(pScene->mRootNode, pScene, Matrix::Identity);

        m_pModel->Initialize(m_materials.data(), m_materials.size(), m_objects.data(),
                             m_objects.size());

        return TRUE;
    }
}

BOOL AssimpLoader::LoadAnimation(const WCHAR *filename) { return FALSE; }

IGameModel *AssimpLoader::GetModel()
{ 
    if (m_pModel)
    {
        m_pModel->AddRef();
        return m_pModel;
    }
    return nullptr;
}

IGameAnimation *AssimpLoader::GetAnimation() { return nullptr; }

void AssimpLoader::ExportModel()
{
    if (m_pModel)
    {
        wchar_t fullPath[MAX_PATH];
        char    outPath[MAX_PATH];
        ZeroMemory(fullPath, sizeof(fullPath));
        ZeroMemory(outPath, sizeof(outPath));
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

void AssimpLoader::ExportAnimation() {}

AssimpLoader::~AssimpLoader() { Cleanup(); }
