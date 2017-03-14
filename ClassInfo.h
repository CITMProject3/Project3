#ifndef __CLASSINFO_H__
#define __CLASSINFO_H__

#include <vector>

typedef struct _MonoClass MonoClass;
typedef struct _MonoObject MonoObject;
class MethodInfo;
class FieldInfo;

// Structures to save all related Class info : That includes its methods and attributes(fields)
class ClassInfo
{

public:
	MonoClass *mono_class = nullptr;
	MonoObject *instanced_class = nullptr;

	std::vector<MethodInfo*> methods;
	std::vector<FieldInfo*> fields;

	// Tools
	const char *GetName() const;
};

#endif __CLASSINFO_H__
