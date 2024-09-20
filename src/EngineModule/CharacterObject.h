#pragma once

#include "AnimationClip.h"
#include "MeshObject.h"

struct Skeleton
{
    uint32_t JointCount;
    Joint    pJoints[0];

    static inline Skeleton *Alloc(uint32_t jointCount)
    {
        size_t    requiredSize = sizeof(Skeleton) + sizeof(Joint) * jointCount;
        Skeleton *pSkeleton = (Skeleton *)malloc(requiredSize);

        if (!pSkeleton)
            return nullptr;

        pSkeleton->JointCount = jointCount;
        return pSkeleton;
    }

    static inline void Dealloc(Skeleton *pInSkeleton) { free(pInSkeleton); }

    size_t GetSize() const { return sizeof(Skeleton) + sizeof(Joint) * JointCount; }
};

class CharacterObject : public MeshObject
{
    Skeleton *m_pSkeleton = nullptr;
    Matrix   *m_pBoneMatrices = nullptr;
    Matrix    m_defaultTransform = Matrix::Identity;

  private:
    virtual void InitCollisionBox() override;
    virtual void InitCollisionSphere() override;

    void Cleanup();

  public:
    virtual void InitMeshHandle(IRenderer *pRnd, const Material *pMaterials, const wchar_t *basePath) override;

    void InitSkeleton(const Joint *pJoints, int jointCount);

    void WriteFile(FILE *fp) override;
    void ReadFile(FILE *fp) override;

    void     UpdateAnimation(AnimationClip *pClip, int frameCount);
    // Todo: 이름은 추후 더 고민해서 수정할 것
    Matrix  *GetPoseMatrices();
    uint32_t GetJointCount() const { return m_pSkeleton->JointCount; }
    void     SetDefaultTransform(const Matrix &m) { m_defaultTransform = m; }

    virtual void Render(IRenderer *pRnd, const Matrix &worldMat) override;

    CharacterObject() = delete;
    CharacterObject(const OBJECT_HEADER &header) : MeshObject::MeshObject(header) {}
    ~CharacterObject() override;
};
