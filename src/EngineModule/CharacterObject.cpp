#include "pch.h"

#include "CharacterObject.h"

void CharacterObject::InitCollisionBox()
{
    if (GetVertexCount() == 0)
    {
        __debugbreak();
        return;
    }

    Vector3 minCorner;
    Vector3 maxCorner;

    Vector3 center;
    Vector3 extents;

    SkinnedVertex *pSkinnedVertex = nullptr;
    pSkinnedVertex = GetSkinnedVertices();
    minCorner = maxCorner = pSkinnedVertex[0].position;

    for (int i = 0; i < GetVertexCount(); i++)
    {
        minCorner = Vector3::Min(minCorner, pSkinnedVertex[i].position);
        maxCorner = Vector3::Max(maxCorner, pSkinnedVertex[i].position);
    }

    /*center = (minCorner + maxCorner) * 0.5f;
    extents = maxCorner - center;*/

    m_boundingBox = Box(minCorner, maxCorner);
    m_pCollisionBoxMesh = CreateWireBoxMesh(center, extents + Vector3(1e-2));
}

void CharacterObject::InitCollisionSphere()
{
    if (GetVertexCount() == 0)
    {
        __debugbreak();
        return;
    }

    SkinnedVertex *pSkinnedVertex = nullptr;
    pSkinnedVertex = GetSkinnedVertices();

    float maxRadius = 0.0f;
    for (int i = 0; i < GetVertexCount(); i++)
    {
        float radius = (m_boundingBox.GetCenter() - pSkinnedVertex[i].position).Length();
        maxRadius = (maxRadius < radius) ? radius : maxRadius;
    }
    maxRadius += 1e-2f; // 살짝 크게 설정
    m_boundingSphere = Sphere(m_boundingBox.GetCenter(), maxRadius);

    m_pCollisionSphereMesh = CreateSphereMesh(maxRadius, 8, 8);
}

void CharacterObject::Cleanup()
{
    if (m_pSkeleton)
    {
        Skeleton::Dealloc(m_pSkeleton);
        m_pSkeleton = nullptr;
    }
    if (m_pBoneMatrices)
    {
        delete[] m_pBoneMatrices;
        m_pBoneMatrices = nullptr;
    }
}

void CharacterObject::InitMeshHandle(IRenderer *pRnd, const Material *pMaterials, const wchar_t *basePath)
{
    IRenderMesh *pHandle = pRnd->CreateSkinnedObject();
    pRnd->BeginCreateMesh(pHandle, GetSkinnedVertices(), GetVertexCount(), GetFaceGroupCount(), basePath);
    for (uint32_t i = 0; i < GetFaceGroupCount(); i++)
    {
        FACE_GROUP *pFace = GetFaceGroup(i);
        Material    material = pMaterials[pFace->Header.MaterialIndex];

        pRnd->InsertFaceGroup(pHandle, pFace->pIndices, pFace->Header.IndexCount / 3, &material);
    }
    pRnd->EndCreateMesh(pHandle);
    m_pMeshHandle = pHandle;
    m_pRenderer = pRnd;

    InitCollisionBox();
    InitCollisionSphere();
}

void CharacterObject::InitSkeleton(const Joint *pJoints, int jointCount)
{
    Skeleton *pSkeleton = Skeleton::Alloc(jointCount);
    memcpy(pSkeleton->pJoints, pJoints, sizeof(Joint) * jointCount);

    m_pSkeleton = pSkeleton;
    m_pBoneMatrices = new Matrix[jointCount];
}

void CharacterObject::WriteFile(FILE *fp)
{
    MeshObject::WriteFile(fp);

    fwrite(m_pSkeleton, m_pSkeleton->GetSize(), 1, fp);
}

void CharacterObject::ReadFile(FILE *fp)
{
    uint32_t  jointCount = 0;
    Skeleton *pSkeleton = nullptr;

    MeshObject::ReadFile(fp);
    fread(&jointCount, sizeof(uint32_t), 1, fp);

    pSkeleton = Skeleton::Alloc(jointCount);
    fread(pSkeleton->pJoints, sizeof(Joint), jointCount, fp);

    m_pSkeleton = pSkeleton;
    m_pBoneMatrices = new Matrix[jointCount];
}

// 처음은 동일한 index를 공유한다는 가정하에 진행을 함. 나중에는 다르게도 해야할 듯
void CharacterObject::UpdateAnimation(AnimationClip *pClip, int frameCount)
{
    for (uint32_t boneId = 0; boneId < GetJointCount(); boneId++)
    {
        Joint    *pJoint = m_pSkeleton->pJoints + boneId;
        Keyframe *pKeyframe = pClip->GetKeyframeByIdx(boneId);

        const int    parentIdx = pJoint->parentIndex;
        const Matrix parentMatrix = parentIdx >= 0 ? m_pBoneMatrices[parentIdx] : Matrix::Identity;

        int    numKeys = pKeyframe->Header.NumKeys;
        Matrix TM = numKeys > 0 ? pKeyframe->pKeys[frameCount % numKeys] : Matrix::Identity;

        m_pBoneMatrices[boneId] = TM * parentMatrix;
    }
    for (uint32_t boneId = 0; boneId < GetJointCount(); boneId++)
    {
        Matrix &globalBindPoseInverse = m_pSkeleton->pJoints[boneId].globalBindposeInverse;
        m_pBoneMatrices[boneId] =
            m_defaultTransform.Invert() * globalBindPoseInverse * m_pBoneMatrices[boneId] * m_defaultTransform;
    }
}

Matrix *CharacterObject::GetPoseMatrices() { return m_pBoneMatrices; }

void CharacterObject::Render(IRenderer *pRnd, const Matrix &worldMat)
{
    m_pRenderer->RenderMeshObject(m_pCollisionSphereMesh, &worldMat, true);
    m_pRenderer->RenderMeshObject(m_pCollisionBoxMesh, &worldMat, true);

    m_pRenderer->RenderCharacterObject(m_pMeshHandle, &worldMat, GetPoseMatrices(), GetJointCount());
}

CharacterObject::~CharacterObject() { Cleanup(); }
