// ScriptingProject.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ScriptingProject.h"

#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ModuleWindow.h"
#include "../GameObject.h"
#include "../ComponentScript.h"

void ScriptNames::GetScriptNames(Application * engine_app)
{
	engine_app->scripting->AddScriptName("Test");
	engine_app->scripting->AddScriptName("Test2");

	engine_app->scripting->SetScriptNames(" \0Test\0Test2\0\0");
}

void Test::GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools)
{
	test_title = "Hello World from Script";
	test_int = 3;
	test_int2 = 5;
	

	public_chars->insert(pair<const char*, string>("Title", test_title));
}

void Test::UpdatePublics(GameObject * game_object)
{
	ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

	test_title = script->public_chars.at("Title");
}

void Test::Start(Application * engine_app, GameObject * game_object)
{
	UpdatePublics(game_object);
}

void Test::Update(Application * engine_app, GameObject * game_object)
{
	UpdatePublics(game_object);
	engine_app->window->SetTitle(test_title.c_str());
}

void Test2::Start(Application * engine_app, GameObject * game_object)
{
}

void Test2::Update(Application * engine_app, GameObject * game_object)
{
}
