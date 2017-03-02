#include "ModuleScripting.h"
#include "Application.h"

ModuleScripting::ModuleScripting(const char* name, bool start_enabled) : Module(name, start_enabled)
{
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

bool ModuleScripting::LoadScriptLibrary(const char* path, HINSTANCE* script)
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
}
