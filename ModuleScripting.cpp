#include "ModuleScripting.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleEditor.h"

ModuleScripting::ModuleScripting(const char* name, bool start_enabled) : Module(name, start_enabled)
{
}

ModuleScripting::~ModuleScripting()
{
}

bool ModuleScripting::Init(Data &config)
{
	#ifdef _DEBUG
	// Loading library path whether there's one available...
	if (config.GetString("debug_scripting_library"))
		debug_lib_base_path.assign(config.GetString("debug_scripting_library"));		// Soundbanks on Library

	if (debug_lib_base_path.empty())  // Not found on Library, look for soundbanks on Assets!
		debug_lib_base_path = App->resource_manager->FindFile("Assets/Scripts/Debug_Game.dll");
	#else
	// Loading library path whether there's one available...
	if (config.GetString("release_scripting_library"))
		release_lib_base_path.assign(config.GetString("release_scripting_library"));		// Soundbanks on Library

	if (release_lib_base_path.empty())  // Not found on Library, look for soundbanks on Assets!
		release_lib_base_path = App->resource_manager->FindFile("Assets/Scripts/Release_Game.dll");
	#endif

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
	debug_lib_base_path.empty() ? data.AppendString("debug_scripting_library", "") : data.AppendString("debug_scripting_library", debug_lib_base_path.c_str());
	release_lib_base_path.empty() ? data.AppendString("release_scripting_library", "") : data.AppendString("release_scripting_library", release_lib_base_path.c_str());
}

DWORD ModuleScripting::GetError()
{
	return last_error;
}

void ModuleScripting::LoadScriptsLibrary()
{
	#ifdef _DEBUG
		scripts_lib = (ResourceScriptsLibrary*)App->resource_manager->LoadResource(debug_lib_base_path, ResourceFileType::RES_SCRIPTS_LIBRARY);
	#else
		scripts_lib = (ResourceScriptsLibrary*)App->resource_manager->LoadResource(release_lib_base_path, ResourceFileType::RES_SCRIPTS_LIBRARY);
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