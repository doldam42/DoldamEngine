#include <filesystem>

#include "AnimationClip.h"
#include "GameManager.h"
#include "GameUtils.h"
#include "MeshObject.h"
#include "Model.h"

#include "EngineInterface.h"

#include "GeometryGenerator.h"

using namespace std;

static void CalcVerticeTangent(BasicVertex *pInOutVertices, UINT numVertices, const uint32_t *pIndices,
                               UINT numTriangles)
{
    for (int i = 0; i < numVertices; i++)
    {
        BasicVertex &v = pInOutVertices[i];

        Vector3 tangent;
        Vector3 c1 = v.normal.Cross(Vector3::UnitZ);
        Vector3 c2 = v.normal.Cross(Vector3::UnitY);

        if (c1.Length() > c2.Length())
        {
            tangent = c1;
        }
        else
        {
            tangent = c2;
        }

        v.tangent = tangent;
    }
}

Model *GeometryGenerator::MakeSquare(const float scale)
{

    Vector3 positions[] = {Vector3(-1.0f, 1.0f, 0.0f) * scale, Vector3(1.0f, 1.0f, 0.0f) * scale,
                           Vector3(1.0f, -1.0f, 0.0f) * scale, Vector3(-1.0f, -1.0f, 0.0f) * scale};
    Vector3 normals[] = {Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 1.0f),
                         Vector3(0.0f, 0.0f, 1.0f)};
    Vector2 texcoords[] = {Vector2(0.0f, 0.0f), Vector2(1.0f, 0.0f), Vector2(1.0f, 1.0f), Vector2(0.0f, 1.0f)};

    uint32_t indices[] = {0, 1, 2, 0, 2, 3};

    BasicVertex pVertices[4];
    for (size_t i = 0; i < 4; i++)
    {
        BasicVertex *v = pVertices + i;
        v->position = positions[i];
        v->normal = normals[i];
        v->texcoord = texcoords[i];
    }

    CalcVerticeTangent(pVertices, 4, indices, 2);

    Model *model = new Model;

    MeshObject *pObj = new MeshObject;
    pObj->Initialize(MESH_TYPE_DEFAULT);
    pObj->SetName(L"Square");

    pObj->BeginCreateMesh(pVertices, 4, 1);
    pObj->InsertFaceGroup(indices, 2, 0);
    pObj->EndCreateMesh();

    MeshObject *ppObjs[] = {pObj};
    Material    defaultMat = Material();
    model->Initialize(&defaultMat, 1, reinterpret_cast<IGameMesh **>(ppObjs), 1);
    model->AddRef();
    return model;
}

Model *GeometryGenerator::MakeBox(const float scale)
{
    vector<Vector3> positions;
    vector<Vector3> colors;
    vector<Vector3> normals;
    vector<Vector2> texcoords; // 텍스춰 좌표

    positions.reserve(24);
    colors.reserve(24);
    normals.reserve(24);
    texcoords.reserve(24);

    // 윗면
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // 아랫면
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // 앞면
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // 뒷면
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // 왼쪽
    positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
    positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    // 오른쪽
    positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
    positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
    positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    texcoords.push_back(Vector2(0.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 0.0f));
    texcoords.push_back(Vector2(1.0f, 1.0f));
    texcoords.push_back(Vector2(0.0f, 1.0f));

    BasicVertex pVertices[24];
    for (size_t i = 0; i < 24; i++)
    {
        BasicVertex *v = pVertices + i;
        v->position = positions[i];
        v->normal = normals[i];
        v->texcoord = texcoords[i];
    }

    uint32_t indices[] = {
        0,  1,  2,  0,  2,  3,  // 윗면
        4,  5,  6,  4,  6,  7,  // 아랫면
        8,  9,  10, 8,  10, 11, // 앞면
        12, 13, 14, 12, 14, 15, // 뒷면
        16, 17, 18, 16, 18, 19, // 왼쪽
        20, 21, 22, 20, 22, 23  // 오른쪽
    };
    CalcVerticeTangent(pVertices, 4, indices, 2);
    Model *pModel = new Model;

    MeshObject *pObj = new MeshObject;
    pObj->Initialize(MESH_TYPE_DEFAULT);
    pObj->SetName(L"Box");

    pObj->BeginCreateMesh(pVertices, 24, 1);
    pObj->InsertFaceGroup(indices, 12, 0); // 폴리곤 개수
    pObj->EndCreateMesh();

    MeshObject *ppObjs[] = {pObj};
    Material    defaultMat = Material();
    pModel->Initialize(&defaultMat, 1, reinterpret_cast<IGameMesh **>(ppObjs), 1);
    pModel->AddRef();
    return pModel;
}

Model *GeometryGenerator::MakeWireBox(const Vector3 center, const Vector3 extends)
{
    vector<Vector3> positions(24);
    vector<Vector3> normals(24);

    // 윗면
    positions.push_back(center + Vector3(-1.0f, 1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, 1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, 1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, 1.0f, -1.0f) * extends);
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, 1.0f, 0.0f));

    // 아랫면
    positions.push_back(center + Vector3(-1.0f, -1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, -1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, -1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, -1.0f, 1.0f) * extends);
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
    normals.push_back(Vector3(0.0f, -1.0f, 0.0f));

    // 앞면
    positions.push_back(center + Vector3(-1.0f, -1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, 1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, 1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, -1.0f, -1.0f) * extends);
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

    // 뒷면
    positions.push_back(center + Vector3(-1.0f, -1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, -1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, 1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, 1.0f, 1.0f) * extends);
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
    normals.push_back(Vector3(0.0f, 0.0f, 1.0f));

    // 왼쪽
    positions.push_back(center + Vector3(-1.0f, -1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, 1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, 1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(-1.0f, -1.0f, -1.0f) * extends);
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));

    // 오른쪽
    positions.push_back(center + Vector3(1.0f, -1.0f, 1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, -1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, 1.0f, -1.0f) * extends);
    positions.push_back(center + Vector3(1.0f, 1.0f, 1.0f) * extends);
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
    normals.push_back(Vector3(1.0f, 0.0f, 0.0f));

    BasicVertex pVertices[24];
    for (size_t i = 0; i < 24; i++)
    {
        BasicVertex *v = pVertices + i;
        v->position = positions[i];
        v->normal = normals[i];
        v->texcoord = Vector2(0.0f);
    }

    uint32_t indices[] = {
        0,  1,  2,  0,  2,  3,  // 윗면
        4,  5,  6,  4,  6,  7,  // 아랫면
        8,  9,  10, 8,  10, 11, // 앞면
        12, 13, 14, 12, 14, 15, // 뒷면
        16, 17, 18, 16, 18, 19, // 왼쪽
        20, 21, 22, 20, 22, 23  // 오른쪽
    };
    CalcVerticeTangent(pVertices, 24, indices, 12);
    Model *pModel = new Model;

    MeshObject *pObj = new MeshObject;
    pObj->Initialize(MESH_TYPE_DEFAULT);
    pObj->SetName(L"WireBox");

    pObj->BeginCreateMesh(pVertices, 24, 1);
    pObj->InsertFaceGroup(indices, 12, 0);
    pObj->EndCreateMesh();

    MeshObject *ppObjs[] = {pObj};
    Material    defaultMat = Material();
    pModel->Initialize(&defaultMat, 1, reinterpret_cast<IGameMesh **>(ppObjs), 1);
    pModel->AddRef();
    return pModel;
}

//
// MeshData GeometryGenerator::MakeCylinder(const float bottomRadius,
//                                         const float topRadius, float height,
//                                         int numSlices) {
//
//  // Texture 좌표계때문에 (numSlices + 1) x 2 개의 버텍스 사용
//
//  const float dTheta = -XM_2PI / float(numSlices);
//
//  MeshData meshData;
//
//  vector<BasicVertex> &vertices = meshData.vertices;
//
//  // 옆면의 바닥 버텍스들 (인덱스 0 이상 numSlices 미만)
//  for (int i = 0; i <= numSlices; i++) {
//    BasicVertex v;
//    v.position = Vector3::Transform(Vector3(bottomRadius, -0.5f * height, 0.0f),
//                                    Matrix::CreateRotationY(dTheta * float(i)));
//
//    // std::cout << v.position.x << " " << v.position.z << std::endl;
//
//    v.normal = v.position - Vector3(0.0f, -0.5f * height, 0.0f);
//    v.normal.Normalize();
//    v.texcoord = Vector2(float(i) / numSlices, 1.0f);
//
//    vertices.push_back(v);
//  }
//
//  // 옆면의 맨 위 버텍스들 (인덱스 numSlices 이상 2 * numSlices 미만)
//  for (int i = 0; i <= numSlices; i++) {
//    BasicVertex v;
//    v.position = Vector3::Transform(Vector3(topRadius, 0.5f * height, 0.0f),
//                                    Matrix::CreateRotationY(dTheta * float(i)));
//    v.normal = v.position - Vector3(0.0f, 0.5f * height, 0.0f);
//    v.normal.Normalize();
//    v.texcoord = Vector2(float(i) / numSlices, 0.0f);
//
//    vertices.push_back(v);
//  }
//
//  vector<uint32_t> &indices = meshData.indices;
//
//  for (int i = 0; i < numSlices; i++) {
//    indices.push_back(i);
//    indices.push_back(i + numSlices + 1);
//    indices.push_back(i + 1 + numSlices + 1);
//
//    indices.push_back(i);
//    indices.push_back(i + 1 + numSlices + 1);
//    indices.push_back(i + 1);
//  }
//
//  return meshData;
//}
//

Model *GeometryGenerator::MakeSphere(const float radius, const int numslices, const int numstacks)
{

    // 참고: opengl sphere
    // http://www.songho.ca/opengl/gl_sphere.html
    // texture 좌표계때문에 (numslices + 1) 개의 버텍스 사용 (마지막에 닫아주는
    // 버텍스가 중복) stack은 y 위쪽 방향으로 쌓아가는 방식

    const float dtheta = -XM_2PI / float(numslices);
    const float dphi = -XM_2PI / float(numstacks);

    vector<BasicVertex> vertices;
    vertices.reserve(numstacks * numslices);

    for (int j = 0; j <= numstacks; j++)
    {

        // 스택에 쌓일 수록 시작점을 x-y 평면에서 회전 시켜서 위로 올리는 구조
        Vector3 stackstartpoint = Vector3::Transform(Vector3(0.0f, -radius, 0.0f), Matrix::CreateRotationZ(dphi * j));

        for (int i = 0; i <= numslices; i++)
        {
            BasicVertex v;

            // 시작점을 x-z 평면에서 회전시키면서 원을 만드는 구조
            v.position = Vector3::Transform(stackstartpoint, Matrix::CreateRotationY(dtheta * float(i)));

            v.normal = v.position; // 원점이 구의 중심
            v.normal.Normalize();
            v.texcoord = Vector2(float(i) / numslices, 1.0f - float(j) / numstacks);

            vertices.push_back(v);
        }
    }

    // cout << vertices.size() << endl;

    vector<uint32_t> indices;
    indices.reserve(numstacks * numslices);
    for (int j = 0; j < numstacks; j++)
    {

        const int offset = (numslices + 1) * j;

        for (int i = 0; i < numslices; i++)
        {

            indices.push_back(offset + i);
            indices.push_back(offset + i + numslices + 1);
            indices.push_back(offset + i + 1 + numslices + 1);

            indices.push_back(offset + i);
            indices.push_back(offset + i + 1 + numslices + 1);
            indices.push_back(offset + i + 1);
        }
    }

    // cout << indices.size() << endl;
    // for (int i = 0; i < indices.size(); i++) {
    //     cout << indices[i] << " ";
    // }
    // cout << endl;

    CalcVerticeTangent(vertices.data(), vertices.size(), indices.data(), indices.size() / 3);
    Model *pModel = new Model;

    MeshObject *pObj = new MeshObject;
    pObj->Initialize(MESH_TYPE_DEFAULT);
    pObj->SetName(L"Sphere");

    pObj->BeginCreateMesh(vertices.data(), vertices.size(), 1);
    pObj->InsertFaceGroup(indices.data(), indices.size() / 3, 0);
    pObj->EndCreateMesh();

    MeshObject *ppObjs[] = {pObj};
    Material    defaultMat = Material();
    pModel->Initialize(&defaultMat, 1, reinterpret_cast<IGameMesh **>(ppObjs), 1);
    pModel->AddRef();
    return pModel;
}

// MeshData GeometryGenerator::MakeIcosahedron() {
//
//  // Luna DX12 교재 참고
//  // 등20면체
//  // https://mathworld.wolfram.com/Isohedron.html
//
//  const float X = 0.525731f;
//  const float Z = 0.850651f;
//
//  MeshData newMesh;
//
//  vector<Vector3> pos = {
//      Vector3(-X, 0.0f, Z), Vector3(X, 0.0f, Z),   Vector3(-X, 0.0f, -Z),
//      Vector3(X, 0.0f, -Z), Vector3(0.0f, Z, X),   Vector3(0.0f, Z, -X),
//      Vector3(0.0f, -Z, X), Vector3(0.0f, -Z, -X), Vector3(Z, X, 0.0f),
//      Vector3(-Z, X, 0.0f), Vector3(Z, -X, 0.0f),  Vector3(-Z, -X, 0.0f)};
//
//  for (size_t i = 0; i < pos.size(); i++) {
//    BasicVertex v;
//    v.position = pos[i];
//    v.normal = v.position;
//    v.normal.Normalize();
//
//    newMesh.vertices.push_back(v);
//  }
//
//  newMesh.indices = {1,  4,  0, 4,  9, 0, 4, 5,  9, 8, 5, 4,  1,  8, 4,
//                     1,  10, 8, 10, 3, 8, 8, 3,  5, 3, 2, 5,  3,  7, 2,
//                     3,  10, 7, 10, 6, 7, 6, 11, 7, 6, 0, 11, 6,  1, 0,
//                     10, 1,  6, 11, 0, 9, 2, 11, 9, 5, 2, 9,  11, 2, 7};
//
//  return newMesh;
//}
//
// MeshData GeometryGenerator::MakeTetrahedron() {
//
//  // Regular Tetrahedron
//  // https://mathworld.wolfram.com/RegularTetrahedron.html
//
//  const float a = 1.0f;
//  const float x = sqrt(3.0f) / 3.0f * a;
//  const float d = sqrt(3.0f) / 6.0f * a; // = x / 2
//  const float h = sqrt(6.0f) / 3.0f * a;
//
//  vector<Vector3> points = {{0.0f, x, 0.0f},
//                            {-0.5f * a, -d, 0.0f},
//                            {+0.5f * a, -d, 0.0f},
//                            {0.0f, 0.0f, h}};
//
//  Vector3 center = Vector3(0.0f);
//
//  for (int i = 0; i < 4; i++) {
//    center += points[i];
//  }
//  center /= 4.0f;
//
//  for (int i = 0; i < 4; i++) {
//    points[i] -= center;
//  }
//
//  MeshData meshData;
//
//  for (int i = 0; i < points.size(); i++) {
//
//    BasicVertex v;
//    v.position = points[i];
//    v.normal = v.position; // 중심이 원점
//    v.normal.Normalize();
//
//    meshData.vertices.push_back(v);
//  }
//
//  meshData.indices = {0, 1, 2, 3, 2, 1, 0, 3, 1, 0, 2, 3};
//
//  return meshData;
//}
// MeshData GeometryGenerator::SubdivideToSphere(const float radius,
//                                              MeshData    meshData) {
//
//  using namespace DirectX;
//  using DirectX::SimpleMath::Matrix;
//  using DirectX::SimpleMath::Vector3;
//
//  // 원점이 중심이라고 가정
//  for (auto &v : meshData.vertices) {
//    v.position = v.normal * radius;
//  }
//
//  // 구의 표면으로 옮기고 노멀과 texture 좌표 계산
//  auto ProjectBasicVertex = [&](BasicVertex &v) {
//    v.normal = v.position;
//    v.normal.Normalize();
//    v.position = v.normal * radius;
//
//    // 주의: 텍스춰가 이음매에서 깨집니다.
//    // atan vs atan2
//    // https://stackoverflow.com/questions/283406/what-is-the-difference-between-atan-and-atan2-in-c
//    // const float theta = atan2f(v.position.z, v.position.x);
//    // const float phi = acosf(v.position.y / radius);
//    // v.texcoord.x = theta / XM_2PI;
//    // v.texcoord.y = phi / XM_PI;
//  };
//
//  auto UpdateFaceNormal = [](BasicVertex &v0, BasicVertex &v1,
//                             BasicVertex &v2) {
//    auto faceNormal =
//        (v1.position - v0.position).Cross(v2.position - v0.position);
//    faceNormal.Normalize();
//    v0.normal = faceNormal;
//    v1.normal = faceNormal;
//    v2.normal = faceNormal;
//  };
//
//  // 버텍스가 중복되는 구조로 구현
//  MeshData newMesh;
//  uint32_t count = 0;
//  for (size_t i = 0; i < meshData.indices.size(); i += 3) {
//    size_t i0 = meshData.indices[i];
//    size_t i1 = meshData.indices[i + 1];
//    size_t i2 = meshData.indices[i + 2];
//
//    BasicVertex v0 = meshData.vertices[i0];
//    BasicVertex v1 = meshData.vertices[i1];
//    BasicVertex v2 = meshData.vertices[i2];
//
//    BasicVertex v3;
//    v3.position = (v0.position + v2.position) * 0.5f;
//    v3.texcoord = (v0.texcoord + v2.texcoord) * 0.5f;
//    ProjectBasicVertex(v3);
//
//    BasicVertex v4;
//    v4.position = (v0.position + v1.position) * 0.5f;
//    v4.texcoord = (v0.texcoord + v1.texcoord) * 0.5f;
//    ProjectBasicVertex(v4);
//
//    BasicVertex v5;
//    v5.position = (v1.position + v2.position) * 0.5f;
//    v5.texcoord = (v1.texcoord + v2.texcoord) * 0.5f;
//    ProjectBasicVertex(v5);
//
//    // UpdateFaceNormal(v4, v1, v5);
//    // UpdateFaceNormal(v0, v4, v3);
//    // UpdateFaceNormal(v3, v4, v5);
//    // UpdateFaceNormal(v3, v5, v2);
//
//    newMesh.vertices.push_back(v4);
//    newMesh.vertices.push_back(v1);
//    newMesh.vertices.push_back(v5);
//
//    newMesh.vertices.push_back(v0);
//    newMesh.vertices.push_back(v4);
//    newMesh.vertices.push_back(v3);
//
//    newMesh.vertices.push_back(v3);
//    newMesh.vertices.push_back(v4);
//    newMesh.vertices.push_back(v5);
//
//    newMesh.vertices.push_back(v3);
//    newMesh.vertices.push_back(v5);
//    newMesh.vertices.push_back(v2);
//
//    for (uint32_t j = 0; j < 12; j++) {
//      newMesh.indices.push_back(j + count);
//    }
//    count += 12;
//  }
//
//  return newMesh;
//}

Model *GeometryGenerator::ReadFromFile(const wchar_t *basePath, const wchar_t *filename)
{
    namespace fs = std::filesystem;

    wchar_t wcsPath[MAX_PATH];
    char    path[MAX_PATH];
    memset(wcsPath, L'\0', sizeof(wcsPath));
    memset(path, L'\0', sizeof(path));

    wcscpy_s(wcsPath, basePath);
    wcscat_s(wcsPath, filename);

    GameUtils::ws2s(wcsPath, path);

    FILE *fp = nullptr;
    fopen_s(&fp, path, "rb");
    if (!fp)
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return nullptr;
    }

    Model *pModel = new Model;
    pModel->ReadFile(fp);
    fclose(fp);

    pModel->SetBasePath(basePath);
    Normalize(Vector3(0.f, 0.f, 0.f), 1, pModel);

    return pModel;
}

AnimationClip *GeometryGenerator::ReadAnimationFromFile(const wchar_t *basePath, const wchar_t *filename)
{
    wchar_t wcsPath[MAX_PATH];
    char    path[MAX_PATH];
    memset(wcsPath, L'\0', sizeof(wcsPath));
    memset(path, L'\0', sizeof(path));

    wcscpy_s(wcsPath, basePath);
    wcscat_s(wcsPath, filename);

    GameUtils::ws2s(wcsPath, path);

    FILE *fp = nullptr;

    fopen_s(&fp, path, "rb");
    if (!fp)
    {
        __debugbreak();
        return nullptr;
    }

    AnimationClip *pAnim = new AnimationClip(filename);
    pAnim->ReadFile(fp);
    fclose(fp);

    pAnim->ref_count = 1;
    return pAnim;
}

void GeometryGenerator::Normalize(const Vector3 center, const float longestLength, Model *pInOutModel)
{
    // 모델의 중심을 원점으로 옮기고 크기를 [-1,1]^3으로 스케일

    // Normalize vertices
    //
    // 만약 Normal Model이면
    Vector3 vmin(1000, 1000, 1000);
    Vector3 vmax(-1000, -1000, -1000);
    for (uint32_t i = 0; i < pInOutModel->GetObjectCount(); i++)
    {
        MeshObject *pObj = pInOutModel->GetObjectByIdx(i);
        if (pObj->IsSkinned())
        {
            const SkinnedVertex *pVertice = pObj->GetSkinnedVertices();
            for (uint32_t vertexId = 0; vertexId < pObj->GetVertexCount(); vertexId++)
            {
                SkinnedVertex v = pVertice[vertexId];
                vmin.x = XMMin(vmin.x, v.position.x);
                vmin.y = XMMin(vmin.y, v.position.y);
                vmin.z = XMMin(vmin.z, v.position.z);
                vmax.x = XMMax(vmax.x, v.position.x);
                vmax.y = XMMax(vmax.y, v.position.y);
                vmax.z = XMMax(vmax.z, v.position.z);
            }
        }
        else
        {
            const BasicVertex *pVertice = pObj->GetBasicVertices();
            for (uint32_t vertexId = 0; vertexId < pObj->GetVertexCount(); vertexId++)
            {
                BasicVertex v = pVertice[vertexId];
                vmin.x = XMMin(vmin.x, v.position.x);
                vmin.y = XMMin(vmin.y, v.position.y);
                vmin.z = XMMin(vmin.z, v.position.z);
                vmax.x = XMMax(vmax.x, v.position.x);
                vmax.y = XMMax(vmax.y, v.position.y);
                vmax.z = XMMax(vmax.z, v.position.z);
            }
        }
    }

    float   dx = vmax.x - vmin.x, dy = vmax.y - vmin.y, dz = vmax.z - vmin.z;
    float   scale = longestLength / XMMax(XMMax(dx, dy), dz);
    Vector3 translation = -(vmin + vmax) * 0.5f + center;

    for (uint32_t i = 0; i < pInOutModel->GetObjectCount(); i++)
    {
        MeshObject *pObj = pInOutModel->GetObjectByIdx(i);
        Transform   tm = *pObj->GetLocalTransform();
        tm.SetPosition((tm.GetPosition() + translation) * scale);
        tm.SetScale(Vector3::One);
        pObj->SetTransform(&tm);

        if (pObj->IsSkinned())
        {
            SkinnedVertex *pVertice = pObj->GetSkinnedVertices();
            for (uint32_t vertexId = 0; vertexId < pObj->GetVertexCount(); vertexId++)
            {
                SkinnedVertex *v = pVertice + vertexId;
                v->position = (v->position + translation) * scale;
            }
            // pChar->SetDefaultTransform(Matrix::CreateTranslation(translation) * Matrix::CreateScale(scale));
        }
        else
        {
            MeshObject  *pMesh = (MeshObject *)pObj;
            BasicVertex *pVertice = pMesh->GetBasicVertices();
            for (uint32_t vertexId = 0; vertexId < pMesh->GetVertexCount(); vertexId++)
            {
                BasicVertex *v = pVertice + vertexId;
                v->position = (v->position + translation) * scale;
            }
        }
    }

    UINT   jointCount = pInOutModel->GetJointCount();
    Matrix tm = Matrix::CreateTranslation(translation) * Matrix::CreateScale(scale);
    for (UINT i = 0; i < jointCount; i++)
    {
        Joint *pJoint = pInOutModel->GetJointByIdx(i);
        pJoint->globalBindposeInverse = tm.Invert() * pJoint->globalBindposeInverse;
    }
}
