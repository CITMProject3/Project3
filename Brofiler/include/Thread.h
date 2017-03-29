#pragma once
#include "Common.h"
#include <winnt.h>

namespace Profiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns current Thread Environment Block (Extremely fast approach to get Thread Unique ID)
BRO_INLINE const void* GetThreadUniqueID()
{
	return NtCurrentTeb();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}