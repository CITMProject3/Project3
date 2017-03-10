#include "ModuleScripting.h"
#include "Application.h"
#include "Globals.h"
#include "ModuleFileSystem.h"
#include "ScriptingProject/GameScripts.h"
#include <iostream>

ModuleScripting::ModuleScripting(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	finded_script_names = false; 
	scripts_quantity = -1;
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
	Hello script;
	script.Say();
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
	if (App->file_system->Exists("Game.dll"))
	{
		scripts_loaded = false;
		
		LOG("Can't find Game.dll", "Game.dll", last_error);
	}
	else
	{
		scripts_loaded = true;
		//LoadScriptNames();
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
		//ScriptNames::GetScriptNames(App);
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
