#pragma once

#include <Windows.h>

class SpinLock
{
  private:
    volatile ULONG lockValue = 0;

  public:
    void lock();
    void unlock();
};
