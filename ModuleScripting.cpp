#include "ModuleScripting.h"

ModuleScripting::ModuleScripting(const char* name, bool start_enabled) : Module(name, start_enabled)
{ }

ModuleScripting::~ModuleScripting()
{ }

bool ModuleScripting::Init(Data &config)
{
	mono_scripts.InitMonoLibrary();
	return true;
}

bool ModuleScripting::Start()
{
	return true;
}


bool ModuleScripting::CleanUp()
{
	mono_scripts.TerminateMonoLibrary();
	return true;
}

void ModuleScripting::SaveBeforeClosing(Data &data) const
{ }

bool ModuleScripting::LoadScript(const ClassInfo* script_to_load)
{
	return mono_scripts.LoadScript(script_to_load);
}

void ModuleScripting::ObtainScripts(std::vector<ClassInfo*> &scripts)
{
	mono_scripts.GetScripts(scripts);
}
