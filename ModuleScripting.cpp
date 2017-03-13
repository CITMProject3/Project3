#include "ModuleScripting.h"
#include "MonoScripts.h"

ModuleScripting::ModuleScripting(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	mono_scripts = new MonoScripts();
}

ModuleScripting::~ModuleScripting()
{ }

bool ModuleScripting::Init(Data &config)
{
	mono_scripts->Init();
	return true;
}

bool ModuleScripting::Start()
{
	return true;
}


bool ModuleScripting::CleanUp()
{
	mono_scripts->Terminate();
	delete mono_scripts;

	return true;
}

void ModuleScripting::SaveBeforeClosing(Data &data) const
{ }

bool ModuleScripting::LoadScript(ClassInfo* script_to_load)
{
	return mono_scripts->LoadScript(script_to_load);
}

void ModuleScripting::ObtainScripts(std::vector<ClassInfo*> &scripts)
{
	mono_scripts->GetScripts(scripts);
}
