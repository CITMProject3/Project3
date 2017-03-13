#include "ModuleScripting.h"
#include "Application.h"
#include "Globals.h"

ModuleScripting::ModuleScripting(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	script = NULL;
	finded_script_names = false; 
	scripts_quantity = -1;
}

ModuleScripting::~ModuleScripting()
{
}

bool ModuleScripting::Init(Data &config)
{
	LoadScriptsLibrary();
	return true;
}

bool ModuleScripting::Start()
{
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
	if (script != NULL)
		FreeLibrary(script);
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
	if (script)
	{
		FreeLibrary(script);
	}

	if ((script = LoadLibrary("Game")) == NULL)
	{
		last_error = GetLastError();
		scripts_loaded = false;
		
		if (last_error == 127)
		{
			LOG("Can't find Game.dll", "Game.dll", last_error);
		}
		else
			LOG("Unknown error loading Game.dll", "Game.dll", last_error);
	}
	else
	{
		scripts_loaded = true;
		LoadScriptNames();
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
		if (f_GetScriptNames get_script_names = (f_GetScriptNames)GetProcAddress(App->scripting->script, "GetScriptNames"))
		{
			finded_script_names = true;
			get_script_names(App);

			/*int i = 0;
			for (vector<const char*>::const_iterator it = script_names.begin(); it != script_names.end(); it++, i++)
			{
				if (i == 0)
					names = (*it);
				else
				{
					names += "\0";
					names += (*it);
				}
			}*/

			/*const char* str;
			int i = 0;
			for (vector<const char*>::const_iterator it = script_names.begin(); it != script_names.end(); it++, i++)
			{
				if (i == 0)
					str = (*it);
				else
				{
					str += "\0";
					str += (*it);
				}
			}
			names = str.c_str();*/
		}
		else
		{
			finded_script_names = false;
			last_error = GetLastError();

			if (last_error == 126)
			{
				LOG("Can't find script names function", "Game.dll", last_error);
			}
			else
				LOG("Unknown error loading script names", "Game.dll", last_error);
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
