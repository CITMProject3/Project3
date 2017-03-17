// ScriptingProject.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
//
//#include <list>
//#include <string>
//
//#include "../Application.h"
//#include "../ModuleScripting.h"
//#include "../ModuleWindow.h"

/* Very useful links

Using dllimport and dllexport in C++ Classes
https://msdn.microsoft.com/en-us/library/81h27t8c.aspx

Take a look! Calling methods within a class?
https://www.codeproject.com/articles/9405/using-classes-exported-from-a-dll-using-loadlibrar
*/

//namespace Hello
//{
//	__declspec(dllexport) void Say() { LOG("Hello!"); }
//}
//
//namespace GoodBye
//{
//	__declspec(dllexport) void Say() { LOG("Goodbye!"); }
//}

namespace Goodbye
{
	__declspec(dllexport) void Say() { }
}

extern "C"
{
	// On this function, people must include new methods for each script...
	__declspec(dllexport) const char **GetScriptNames()
	{
		static const char *script_names[] = {
			"Test",
			"Test2",
			nullptr
		};

		return script_names;
	}

	//__declspec(dllexport) int Add(int a, int b)
	//{
	//	return(a + b);
	//}

	namespace Hello
	{
		__declspec(dllexport) void Say() {  }
	}

	//namespace Test
	//{
	//	void Test_Start(Application* engine_app, GameObject* game_object)
	//	{
	//	}

	//	void Test_Update(Application* engine_app, GameObject* game_object)
	//	{
	//		//engine_app->window->SetTitle("Hello World from Script");
	//	}
	//}

	//namespace Test2
	//{
	//	void Test2_Start(Application* engine_app, GameObject* game_object)
	//	{
	//	}

	//	void Test2_Update(Application* engine_app, GameObject* game_object)
	//	{
	//		//engine_app->window->SetTitle("Hello World from Script2");
	//	}
	//}
}