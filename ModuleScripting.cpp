#include "ModuleScripting.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "Globals.h"

ModuleScripting::ModuleScripting(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	scripts_lib = NULL;
	finded_script_names = false; 
	scripts_quantity = -1;
	resource_created = false;
	scripts_loaded = false;
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
/*
update_status ModuleScripting::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModuleScripting::Update()
{
	return UPDATE_CONTINUE;
}

update_status ModuleScripting::PostUpdate()
{
	return UPDATE_CONTINUE;
}*/

bool ModuleScripting::CleanUp()
{
	if (script_names.size() > 0)
		script_names.clear();

	scripts_lib->Unload();
	resource_created = false;
	scripts_loaded = false;
	finded_script_names = false;
	//if (scripts_lib->lib != NULL)
	//	FreeLibrary(script);
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
	/*if (script)
	{
		FreeLibrary(script);
	}*/

	if(_DEBUG)
		scripts_lib = (ResourceScriptsLibrary*)App->resource_manager->LoadResource(App->resource_manager->FindFile("Assets/Scripts/Debug_Game.dll"), ResourceFileType::RES_SCRIPTS_LIBRARY);
	else
		scripts_lib = (ResourceScriptsLibrary*)App->resource_manager->LoadResource(App->resource_manager->FindFile("Assets/Scripts/Release_Game.dll"), ResourceFileType::RES_SCRIPTS_LIBRARY);

	if (scripts_lib != nullptr)
	{
		resource_created = true;
		if (scripts_lib->lib == NULL)
		{
			last_error = GetLastError();
			scripts_loaded = false;

			if (last_error == 127)
			{
				LOG("Can't find Game.dll");
			}
			else
				LOG("Unknown error loading Game.dll");
		}
		else
		{
			scripts_loaded = true;
			LoadScriptNames();
		}
	}
}

void ModuleScripting::LoadScriptNames()
{
	script_names.clear();
	script_names.push_back(" ");
	if (scripts_quantity != -1)
		names = "";

	if (scripts_loaded)
	{
		if (f_GetScriptNames get_script_names = (f_GetScriptNames)GetProcAddress(scripts_lib->lib, "GetScriptNames"))
		{
			finded_script_names = true;
			get_script_names(App);
		}
		else
		{
			finded_script_names = false;
			last_error = GetLastError();

			if (last_error == 126)
			{
				LOG("Can't find script names function");
			}
			else
				LOG("Unknown error loading script names");
		}
	}
}

vector<const char*> ModuleScripting::GetScriptNamesList()const
{
	return script_names;
}

const char* ModuleScripting::GetScriptNames()const
{
	return names;
}

void ModuleScripting::SetScriptNames(const char* names)
{
	this->names = names;
}

void ModuleScripting::AddScriptName(const char* name)
{
	script_names.push_back(name);
}

/*bool ModuleScripting::LoadScriptLibrary(const char* path, HINSTANCE* script)
{
	bool ret = false;
	if (*script = LoadLibrary(path))
	{
		ret = true;
	}
	else
	{
		last_error = GetLastError();
	}

	if (!ret)
		FreeLibrary(*script);

	return ret;
}

bool ModuleScripting::FreeScriptLibrary(HINSTANCE& script)
{
	bool ret = false;
	if (script)
	{
		FreeLibrary(script);
		ret = true;
	}
	return ret;
}*/
