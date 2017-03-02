#include "ComponentScript.h"
#include "Application.h"
#include "ModuleScripting.h"

ComponentScript::ComponentScript(ComponentType type, GameObject* game_object, const char* path) : Component(type, game_object)
{
	this->path = path;
	App->scripting->LoadScriptLibrary(this->path.c_str(), &script);
}

ComponentScript::~ComponentScript()
{
	if (script)
		App->scripting->FreeScriptLibrary(script);
}

void ComponentScript::Update()
{
	if (!script)
	{
		App->scripting->LoadScriptLibrary(path.c_str(), &script);
	}
	else
	{
		if (!started)
		{
			if (f_start start = (f_start)GetProcAddress(script, "Start"))
			{
				start(App, GetGameObject());
			}
			else
			{
				DWORD error = GetLastError();
			}
		}
		else
		{
			if (f_update update = (f_update)GetProcAddress(script, "Update"))
			{
				update(App, GetGameObject());
			}
			else
			{
				DWORD error = GetLastError();
			}
		}
	}
}

void ComponentScript::OnInspector(bool debug)
{
}

void ComponentScript::Save(Data & file) const
{
}

void ComponentScript::Load(Data & conf)
{
}

void ComponentScript::SetPath(const char * path)
{
	if (script)
	{
		App->scripting->FreeScriptLibrary(script);
	}
	this->path = path;
}
