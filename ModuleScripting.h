#ifndef __MODULESCRIPTING_H__
#define __MODULESCRIPTING_H__

#include "Module.h"

struct MethodInfo;
struct FieldInfo;
typedef struct _MonoType MonoType;

// Structures to save all related Class info: That includes its methods and attributes (fields)
struct ClassInfo
{
	std::string name;
	std::vector<MethodInfo*> methods;
	std::vector<FieldInfo*> fields;
};

struct MethodInfo
{
	std::string name;
	std::vector<MonoType*> types;
};

struct FieldInfo
{
	std::string name;
};

typedef struct _MonoDomain MonoDomain;
typedef struct _MonoImage MonoImage;

class ModuleScripting : public Module
{
public:
	ModuleScripting(const char* name, bool start_enabled = true);
	~ModuleScripting();

	bool Init(Data& config);
	bool Start();

	bool CleanUp();
	void SaveBeforeClosing(Data& data)const;

	void ObtainScripts(std::vector<ClassInfo*> &scripts) const;

private:
	
	MonoDomain *mono_domain = nullptr;

	std::vector<ClassInfo*> script_collection;

	void InitMonoLibrary();
	void TerminateMonoLibrary();

	void LoadClasses(MonoImage *mono_image);
};

#endif // !__MOUDLESCRIPTING_H__
