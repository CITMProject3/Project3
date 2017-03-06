#include "ModuleScripting.h"
#include "Application.h"

ModuleScripting::ModuleScripting(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	script == NULL;
	finded_script_names = false;
}

ModuleScripting::~ModuleScripting()
{
	if(script)
		FreeLibrary(script);
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
	if ((script = LoadLibrary("Game")) == NULL)
	{
		last_error = GetLastError();
		scripts_loaded = false;

		if (last_error == 127)
			OutputDebugString("Can't find Game.dll");
		else
			OutputDebugString("Unknown error loading Game.dll");
	}
	else
	{
		scripts_loaded = true;
		LoadScriptNames();
	}
}

void ModuleScripting::LoadScriptNames()
{
	scripts_names.clear();

	if (scripts_loaded)
	{
		if (f_GetScriptNames get_script_names = (f_GetScriptNames)GetProcAddress(App->scripting->script, "GetScriptNames"))
		{
			finded_script_names = true;
			scripts_names = get_script_names();
		}
		else
		{
			finded_script_names = false;
			last_error = GetLastError();

			if (last_error == 126)
				OutputDebugString("Can't find script names function");
			else
				OutputDebugString("Unknown error loading script names");
		}
	}
}

list<const char*> ModuleScripting::GetScriptNames()const
{
	return scripts_names;
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
