#include "ModuleScripting.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleEditor.h"

//#include "Globals.h"

ModuleScripting::ModuleScripting(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	scripts_lib = NULL;
	resource_created = false;
	scripts_loaded = false;
	setting_go_var = false;
	set_go_var_now = nullptr;
}

ModuleScripting::~ModuleScripting()
{
}

bool ModuleScripting::Init(Data &config)
{
	return true;
}

bool ModuleScripting::Start()
{
	LoadScriptsLibrary();
	return true;
}

bool ModuleScripting::CleanUp()
{
	if(scripts_lib)
		scripts_lib->Unload();
	resource_created = false;
	scripts_loaded = false;

	return true;
}

void ModuleScripting::SaveBeforeClosing(Data &data) const
{
}

DWORD ModuleScripting::GetError()
{
	return last_error;
}

void ModuleScripting::LoadScriptsLibrary()
{
	#ifdef _DEBUG
		scripts_lib = (ResourceScriptsLibrary*)App->resource_manager->LoadResource(App->resource_manager->FindFile("Assets/Scripts/Debug_Game.dll"), ResourceFileType::RES_SCRIPTS_LIBRARY);
	#else
		scripts_lib = (ResourceScriptsLibrary*)App->resource_manager->LoadResource(App->resource_manager->FindFile("Assets/Scripts/Release_Game.dll"), ResourceFileType::RES_SCRIPTS_LIBRARY);
	#endif

	if (scripts_lib != nullptr)
	{
		resource_created = true;
		if (scripts_lib->lib == NULL)
		{
			last_error = GetLastError();
			scripts_loaded = false;

			if (last_error == 127)
			{
				LOG("[ERROR] Can't find Game.dll");
				App->editor->DisplayWarning(WarningType::W_ERROR, "Can't find Game.dll");
			}
			else
			{
				LOG("[ERROR] Unknown error loading Game.dll");
				App->editor->DisplayWarning(WarningType::W_ERROR, "Unknown error loading Game.dll");
			}			
		}
		else
		{
			scripts_loaded = true;
		}
	}
}

vector<const char*> ModuleScripting::GetScriptNamesList()const
{
	return scripts_lib->script_names;
}

void ModuleScripting::GetPublics(const char* script_name, map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_floats, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
{
	scripts_lib->GetPublicVars(script_name, public_chars, public_ints, public_floats, public_bools, public_gos);
}