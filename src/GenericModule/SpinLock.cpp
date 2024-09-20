#include "pch.h"

#include "SpinLock.h"

void SpinLock::lock()
{
    while (InterlockedExchange(&lockValue, 1) == 1)
    {
        while (lockValue == 1);
    }
}

void SpinLock::unlock()
{
    InterlockedExchange(&lockValue, 0);
}
