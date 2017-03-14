#ifndef __METHODINFO_H__
#define __METHODINFO_H__

#include <string>

typedef struct _MonoMethod MonoMethod;

class MethodInfo
{
public:

	MonoMethod *mono_method = nullptr;
	std::string name;
};

#endif __METHODINFO_H__
