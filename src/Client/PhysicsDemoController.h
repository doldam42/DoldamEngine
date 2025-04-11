#pragma once

#include "SceneRegistry.h"

class PhysicsDemoController : public IScene
{
    const static UINT MAX_BALL_COUNT = 128;

    IRenderMaterial *m_pMaterial = nullptr;

    IGameObject *m_pBoxs[6] = {nullptr};
    IGameObject *m_pBalls[MAX_BALL_COUNT] = {nullptr};

    UINT frontCursor = 0;
    UINT rearCursor = 0;
    UINT ballCount = 0;

    void Push()
    {
        rearCursor = (rearCursor + 1) % MAX_BALL_COUNT;
        ++ballCount;
    }
    IGameObject *Pop()
    {
        IGameObject *pDel = m_pBalls[frontCursor];
        frontCursor = (frontCursor + 1) % MAX_BALL_COUNT;
        --ballCount;
        return pDel;
    }
    IGameObject *Back() { return m_pBalls[rearCursor]; }

    bool IsFull() { return ballCount == MAX_BALL_COUNT; }
    bool IsEmpty() { return ballCount == 0; }

  public:
    PhysicsDemoController() = default;
    ~PhysicsDemoController() {}

    // Inherited via IScene
    void Load() override;
    void Update(float dt) override;
    void UnLoad() override;
};
