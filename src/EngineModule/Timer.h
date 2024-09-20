#pragma once

class Timer {
private:
  LARGE_INTEGER frequency;
  LARGE_INTEGER start;

public:
  Timer();
  float Tick();
};