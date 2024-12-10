#pragma once

class Timer
{
    ULONGLONG m_prevTick = 0;
    ULONGLONG m_prevUpdateTick = 0;
    ULONGLONG m_prevFrameCheckTick = 0;

    UINT m_FPS = 0;
    UINT m_frameCount = 0;
    
    float m_deltaTime = 0.0f;
    float m_timeSpeed = 1.0f;

    BOOL m_isPaused = false;

  public:
    Timer();
    BOOL  IsPaused() const { return m_isPaused; }
    float DeltaTime() const { return m_deltaTime; }
    UINT  FPS() const { return m_FPS; }
    float Tick();
};