#pragma once

#include "GameUtils.h"
#include "MathHeaders.h"

struct KEYFRAME_HEADER
{
    wchar_t  BindingJointName[MAX_NAME] = {0};
    uint32_t NumKeys = 0;
};

struct Keyframe
{
    KEYFRAME_HEADER Header;
    Matrix          pKeys[0];

    static inline Keyframe *Alloc(const KEYFRAME_HEADER &header)
    {
        int       requredSize = sizeof(Keyframe) + sizeof(Matrix) * header.NumKeys;
        Keyframe *pData = (Keyframe *)malloc(requredSize);
        if (!pData)
        {
            return nullptr;
        }
        pData->Header.NumKeys = header.NumKeys;
        wcscpy_s(pData->Header.BindingJointName, header.BindingJointName);
        return pData;
    }

    static inline void Dealloc(Keyframe *pKeyframe)
    {
        free(pKeyframe);
    }

    inline size_t GetSize()
    {
        return sizeof(Keyframe) + sizeof(Matrix) * Header.NumKeys;
    }
};

class AnimationClip
{
    size_t  m_Hash;
    wchar_t m_name[MAX_NAME];

    static const size_t MAX_KEY_COUNT = 86400;

    uint32_t   ref_count = 0;
    uint32_t   m_curKeyframeCount = 0;
    Keyframe **m_ppKeyframes = nullptr; // keyframe*[jointCount]
    uint32_t   m_jointCount = 0;

  private:
    void Cleanup();

  public:
    uint32_t AddRef();
    void     Release();
    uint32_t GetRefCount()
    {
        return ref_count;
    }
    size_t GetHash() const
    {
        return m_Hash;
    }

    void WriteFile(FILE *fp);
    void ReadFile(FILE *fp);

    void BeginCreateAnim(int jointCount);
    void InsertKeyframes(const wchar_t *bindingJointName, const Matrix *pInKeys, uint32_t numKeys);
    void EndCreateAnim();

    Keyframe *GetKeyframeByIdx(int jointIdx);
    Keyframe *GetKeyframeByName(const wchar_t *name);

    AnimationClip() = default;
    AnimationClip(const wchar_t *name)
    {
        memset(m_name, 0, sizeof(m_name));
        wcscpy_s(m_name, name);
        m_Hash = GameUtils::CreateHash(m_name, (uint32_t)wcslen(m_name));
    }
    ~AnimationClip();
};
