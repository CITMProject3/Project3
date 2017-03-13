#ifndef __MODULESCRIPTING_H__
#define __MODULESCRIPTING_H__

#include "Module.h"

class MonoScripts;
class ClassInfo;

class ModuleScripting : public Module
{
public:
	ModuleScripting(const char* name, bool start_enabled = true);
	~ModuleScripting();

	bool Init(Data& config);
	bool Start();

	bool CleanUp();
	void SaveBeforeClosing(Data& data)const;

	void ObtainScripts(std::vector<ClassInfo*> &scripts);
	bool LoadScript(ClassInfo* script_to_load);

private:
	
	MonoScripts *mono_scripts;
};

#endif // !__MOUDLESCRIPTING_H__
