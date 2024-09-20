#include "pch.h"

#include "Timer.h"

Timer::Timer() {
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&start);
}

float Timer::Tick() {
  LARGE_INTEGER current;
  QueryPerformanceCounter(&current);

  float deltaTime = static_cast<float>(current.QuadPart - start.QuadPart) /
                    frequency.QuadPart;

  start = current;

  return deltaTime;
}
