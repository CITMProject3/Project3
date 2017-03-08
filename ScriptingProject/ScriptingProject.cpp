// ScriptingProject.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <list>
#include <string>

#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ModuleWindow.h"

extern "C"
{
	namespace ScriptNames
	{
		void GetScriptNames(Application* engine_app)
		{
			engine_app->scripting->AddScriptName("Test");
			engine_app->scripting->AddScriptName("Test2");
			
			engine_app->scripting->SetScriptNames("Test\0Test2\0\0");
		}
	}


	namespace Test
	{
		void Test_Start(Application* engine_app, GameObject* game_object)
		{
		}

		void Test_Update(Application* engine_app, GameObject* game_object)
		{
			engine_app->window->SetTitle("Hello World from Script");
		}
	}

	namespace Test2
	{
		void Test2_Start(Application* engine_app, GameObject* game_object)
		{
		}

		void Test2_Update(Application* engine_app, GameObject* game_object)
		{
			engine_app->window->SetTitle("Hello World from Script2");
		}
	}
}