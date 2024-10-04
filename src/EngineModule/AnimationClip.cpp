#include "pch.h"

#include "GameEngine.h"

#include "AnimationClip.h"

void AnimationClip::Cleanup()
{
    if (m_ppKeyframes)
    {
        for (uint32_t i = 0; i < m_jointCount; i++)
        {
            Keyframe *pKeyframe = m_ppKeyframes[i];
            Keyframe::Dealloc(pKeyframe);
            m_ppKeyframes[i] = nullptr;
        }
        delete[] m_ppKeyframes;
        m_ppKeyframes = nullptr;
    }
}

void AnimationClip::WriteFile(FILE *fp)
{
    fwrite(&m_jointCount, sizeof(uint32_t), 1, fp);
    for (uint32_t i = 0; i < m_jointCount; i++)
    {
        Keyframe *pKeyframe = m_ppKeyframes[i];
        fwrite(pKeyframe, pKeyframe->GetSize(), 1, fp);
    }
}

void AnimationClip::ReadFile(FILE *fp)
{
    uint32_t   jointCount;
    Keyframe **ppKeyframe = nullptr;
    Matrix    *pKeys = new Matrix[MAX_KEY_COUNT];

    fread(&jointCount, sizeof(uint32_t), 1, fp);
    ppKeyframe = new Keyframe *[jointCount];
    for (uint32_t i = 0; i < jointCount; i++)
    {
        KEYFRAME_HEADER header;

        fread(&header, sizeof(KEYFRAME_HEADER), 1, fp);
        fread(pKeys, sizeof(Matrix), header.NumKeys, fp);

        ppKeyframe[i] = Keyframe::Alloc(header);
        memcpy(ppKeyframe[i]->pKeys, pKeys, sizeof(Matrix) * header.NumKeys);
    }

    m_curKeyframeCount = 0;
    m_jointCount = jointCount;
    m_ppKeyframes = ppKeyframe;

    delete[] pKeys;
}

void AnimationClip::BeginCreateAnim(int jointCount)
{
    m_ppKeyframes = new Keyframe *[jointCount];
    m_jointCount = jointCount;
    m_curKeyframeCount = 0;
}

void AnimationClip::InsertKeyframes(const wchar_t *bindingJointName, const Matrix *pInKeys, uint32_t numKeys)
{
    KEYFRAME_HEADER header;
    wcscpy_s(header.BindingJointName, bindingJointName);
    header.NumKeys = numKeys;

    Keyframe *pKeyframe = Keyframe::Alloc(header);
    memcpy(pKeyframe->pKeys, pInKeys, sizeof(Matrix) * numKeys);

    m_ppKeyframes[m_curKeyframeCount] = pKeyframe;
    m_curKeyframeCount++;
}

void AnimationClip::EndCreateAnim() {}

Keyframe *AnimationClip::GetKeyframeByIdx(int jointIdx)
{
    if (jointIdx < 0 || jointIdx >= m_jointCount)
        return nullptr;
    return m_ppKeyframes[jointIdx];
}

Keyframe *AnimationClip::GetKeyframeByName(const wchar_t *jointName)
{
    for (uint32_t i = 0; i < m_jointCount; i++)
    {
        Keyframe *pKeyframe = m_ppKeyframes[i];
        // Joint를 찾으면
        if (!wcscmp(pKeyframe->Header.BindingJointName, jointName))
        {
            return pKeyframe;
        }
    }
    return nullptr;
}

void AnimationClip::SetName(const WCHAR *name) 
{
    memset(m_name, L'\0', sizeof(m_name));
    wcscpy_s(m_name, name);
}

AnimationClip::~AnimationClip() { Cleanup(); }

HRESULT __stdcall AnimationClip::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall AnimationClip::AddRef(void)
{
    ref_count++;
    return ref_count;
}

ULONG __stdcall AnimationClip::Release(void) { 
    LONG returnRefCount = ref_count - 1;
    g_pGame->DeleteAnimation(this);
    return returnRefCount;
}
