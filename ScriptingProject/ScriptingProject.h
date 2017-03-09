#ifndef __SCRIPTINGPROJECT_H__
#define __SCRIPTINGPROJECT_H__

#ifdef SCRIPTINGPROJECT_EXPORTS  
#define SCRIPTING_API __declspec(dllexport)   
#else  
#define SCRIPTING_API __declspec(dllimport)   
#endif  

using namespace std;
#include <list>
#include <string>
#include <map>

class Application;
class GameObject;

namespace ScriptNames
{
	SCRIPTING_API void GetScriptNames(Application* engine_app);
}


namespace Test
{
	string SCRIPTING_API test_title;
	int SCRIPTING_API test_int;
	int SCRIPTING_API test_int2;

	void SCRIPTING_API GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools);

	void SCRIPTING_API UpdatePublics(GameObject* game_object);

	void SCRIPTING_API Start(Application* engine_app, GameObject* game_object);

	void SCRIPTING_API Update(Application* engine_app, GameObject* game_object);
}

namespace Test2
{
	void SCRIPTING_API Start(Application* engine_app, GameObject* game_object);

	void SCRIPTING_API Update(Application* engine_app, GameObject* game_object);
}

#endif // !__SCRIPTINGPROJECT_H__