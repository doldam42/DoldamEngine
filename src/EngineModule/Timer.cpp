#include "pch.h"

#include "Timer.h"

Timer::Timer() {}

float Timer::Tick()
{
    m_frameCount++;
    ULONGLONG curTick = GetTickCount64();

    float dt = static_cast<float>(curTick - m_prevTick) / 1000.f;
    
#ifdef _DEBUG
    if (dt > 0.1f)
    {
        dt = 0.03f;
    }
    dt *= m_timeSpeed;
#endif // _DEBUG

    m_prevTick = curTick;
    m_deltaTime = dt;

    if (curTick - m_prevFrameCheckTick > 1000)
    {
        m_prevFrameCheckTick = curTick;
        m_FPS = m_frameCount;
        m_frameCount = 0;
    }

    return dt;
}
