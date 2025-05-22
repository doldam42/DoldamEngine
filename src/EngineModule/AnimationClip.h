#pragma once

#include "../Common/EngineInterface.h"
#include "GameUtils.h"

class AnimationClip : public IGameAnimation
{
    wchar_t m_name[MAX_NAME] = {L'\0'};

    size_t m_id;

    static const size_t MAX_KEY_COUNT = 86400;

    uint32_t   m_curKeyframeCount = 0;
    uint32_t   m_jointCount = 0;
    // keyframe*[jointCount]
    Keyframe **m_ppKeyframes = nullptr;

  public:
    ULONG ref_count = 0;
    void *m_pSearchHandleInGame = nullptr;

  private:
    void Cleanup();

  public:
    uint32_t GetRefCount() const { return ref_count; }

    void WriteFile(const char *filename) override;
    void ReadFile(const char *filename) override;

    void BeginCreateAnim(int jointCount);
    void InsertKeyframes(const wchar_t *bindingJointName, const Matrix *pInKeys, uint32_t numKeys);
    void EndCreateAnim();

    Keyframe *GetKeyframeByIdx(int jointIdx);
    Keyframe *GetKeyframeByName(const wchar_t *jointName);

    // Setter
    void         SetName(const WCHAR *name) override;
    const WCHAR *GetName() { return m_name; }

    AnimationClip() = default;
    AnimationClip(const WCHAR *name) { SetName(name); }
    ~AnimationClip();

    size_t GetID() override { return m_id; }

    // Inherited via IAnimationClip
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) override;
    ULONG __stdcall AddRef(void) override;
    ULONG __stdcall Release(void) override;
};
