#include "ClassInfo.h"

#include "MonoLibraries.h"

// ---- CLASS INFO ----
const char *ClassInfo::GetName() const
{
	return mono_class_get_name(mono_class);
}