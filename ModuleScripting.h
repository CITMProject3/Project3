#ifndef __MODULESCRIPTING_H__
#define __MODULESCRIPTING_H__

#include "Module.h"
#include "Globals.h"
#include <vector>
#include <string>

//#pragma comment ( lib, "Debug/Game.lib" )

class Application;
using namespace std;

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

	DWORD GetError();
	void LoadScriptsLibrary();
	void LoadScriptNames();
	vector<const char*> GetScriptNamesList()const;
	const char* GetScriptNames()const;
	void SetScriptNames(const char* names);
	void AddScriptName(const char* name);

	bool scripts_loaded;

private:
	DWORD last_error = 0;
	vector<const char*> script_names;
	int scripts_quantity; 
	const char* names;
	bool finded_script_names;
};

#endif // !__MOUDLESCRIPTING_H__
