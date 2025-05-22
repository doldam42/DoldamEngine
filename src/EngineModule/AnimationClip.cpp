#include "pch.h"

#include "GameManager.h"

#include "AnimationClip.h"

void AnimationClip::Cleanup()
{
    if (m_ppKeyframes)
    {
        for (uint32_t i = 0; i < m_jointCount; i++)
        {
            Keyframe *pKeyframe = m_ppKeyframes[i];
            DeleteKeyframe(pKeyframe);
            m_ppKeyframes[i] = nullptr;
        }
        delete[] m_ppKeyframes;
        m_ppKeyframes = nullptr;
    }
}

void AnimationClip::WriteFile(const char *filename)
{
    FILE *fp = nullptr;
    fopen_s(&fp, filename, "wb");
    if (!fp)
        __debugbreak();

    fwrite(&m_jointCount, sizeof(uint32_t), 1, fp);
    for (uint32_t i = 0; i < m_jointCount; i++)
    {
        Keyframe *pKeyframe = m_ppKeyframes[i];
        fwrite(pKeyframe, pKeyframe->GetSize(), 1, fp);
    }

    fclose(fp);
}

void AnimationClip::ReadFile(const char *filename)
{
    FILE *fp = nullptr;
    fopen_s(&fp, filename, "rb");
    if (!fp)
        __debugbreak();

    uint32_t   jointCount;
    Keyframe **ppKeyframe = nullptr;
    Matrix    *pKeys = new Matrix[MAX_KEY_COUNT];

    fread(&jointCount, sizeof(uint32_t), 1, fp);
    ppKeyframe = new Keyframe *[jointCount];

    WCHAR jointName[MAX_NAME];
    for (uint32_t i = 0; i < jointCount; i++)
    {
        UINT numKeys = 0;
        memset(jointName, 0, sizeof(jointName));
        fread(jointName, sizeof(jointName), 1, fp);
        fread(&numKeys, sizeof(UINT), 1, fp);
        fread(pKeys, sizeof(Matrix), numKeys, fp);

        ppKeyframe[i] = CreateKeyframe(jointName, pKeys, numKeys);
    }

    m_curKeyframeCount = 0;
    m_jointCount = jointCount;
    m_ppKeyframes = ppKeyframe;

    delete[] pKeys;

    fclose(fp);
}

void AnimationClip::BeginCreateAnim(int jointCount)
{
    m_ppKeyframes = new Keyframe *[jointCount];
    m_jointCount = jointCount;
    m_curKeyframeCount = 0;
}

void AnimationClip::InsertKeyframes(const wchar_t *bindingJointName, const Matrix *pInKeys, uint32_t numKeys)
{
    Keyframe *pKeyframe = CreateKeyframe(bindingJointName, pInKeys, numKeys);

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
        if (!wcscmp(pKeyframe->BindingJointName, jointName))
        {
            return pKeyframe;
        }
    }
    return nullptr;
}

void AnimationClip::SetName(const WCHAR *name)
{
    ZeroMemory(m_name, sizeof(m_name));
    wcscpy_s(m_name, name);
}

AnimationClip::~AnimationClip() { Cleanup(); }

HRESULT __stdcall AnimationClip::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall AnimationClip::AddRef(void)
{
    ref_count++;
    return ref_count;
}

ULONG __stdcall AnimationClip::Release(void)
{
    LONG returnRefCount = ref_count - 1;
    g_pGame->DeleteAnimation(this);
    return returnRefCount;
}
