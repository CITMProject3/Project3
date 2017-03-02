// ScriptSample.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "../Application.h"

extern "C" void Start(Application* engine_app, GameObject* game_object)
{
}

extern "C" void Update(Application* engine_app, GameObject* game_object)
{
	engine_app->window->SetTitle("Hello World from Script");
}