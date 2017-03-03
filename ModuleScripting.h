#ifndef __MODULESCRIPTING_H__
#define __MODULESCRIPTING_H__

#include "Module.h"
#include "Globals.h"

class ModuleScripting : public Module
{
public:
	ModuleScripting(const char* name, bool start_enabled = true);
	~ModuleScripting();

	bool Init(Data& config);
	bool Start();

	//update_status PreUpdate();
	//update_status Update();
	//update_status PostUpdate();

	bool CleanUp();
	void SaveBeforeClosing(Data& data)const;

	//bool LoadScriptLibrary(const char* path, HINSTANCE* script);
	//bool FreeScriptLibrary(HINSTANCE& script);

	bool scripts_loaded;

private:
	DWORD last_error = 0;

public:
	HINSTANCE script;
};

#endif // !__MOUDLESCRIPTING_H__
