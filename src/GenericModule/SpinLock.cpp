#include "pch.h"

#include "SpinLock.h"

void AcquireSpinLock(volatile LONG *pCount)
{
    LONG oldCount;

lb_try:
    oldCount = InterlockedCompareExchange(pCount, 1, 0);
    if (oldCount)
    {
        for (int i = 0; i < 1024; i++)
        {
            YieldProcessor();
        }
        goto lb_try;
    }
}

void ReleaseSpinLock(volatile LONG *pCount) { InterlockedDecrement(pCount); }
