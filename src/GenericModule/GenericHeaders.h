#pragma once

#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cstring>

// VolumeÀ» °¡Áø
interface IBoundable { virtual Bounds GetBounds() const = 0; };

#include "EventListener.h"
#include "HashTable.h"
#include "KDTree.h"
#include "LinkedList.h"
#include "MemoryPool.h"
#include "ProcessInfo.h"
#include "SpinLock.h"
#include "StringUtil.h"
