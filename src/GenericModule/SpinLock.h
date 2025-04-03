#pragma once

#include <Windows.h>

void AcquireSpinLock(volatile LONG *pCount);
void ReleaseSpinLock(volatile LONG *pCount);