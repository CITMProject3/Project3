#ifndef __MODULESCRIPTING_H__
#define __MODULESCRIPTING_H__

#include "Module.h"
#include "MonoScripts.h"

class ModuleScripting : public Module
{
public:
	ModuleScripting(const char* name, bool start_enabled = true);
	~ModuleScripting();

	bool Init(Data& config);
	bool Start();

	bool CleanUp();
	void SaveBeforeClosing(Data& data)const;

	void ObtainScriptNames(std::vector<std::string> &script_names);

private:
	
	MonoScripts mono_scripts; // Create CLEANUP!!!
};

#endif // !__MOUDLESCRIPTING_H__
