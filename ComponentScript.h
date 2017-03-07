#ifndef __COMPONENT_SCRIPT_H__
#define __COMPONENT_SCRIPT_H__

#include "Component.h"
#include "Globals.h"
#include <string>

class Application;
class GameObject;

using namespace std;

typedef void(*f_Start)(Application* engine_app, GameObject* game_object);
typedef void(*f_Update)(Application* engine_app, GameObject* game_object);

class ComponentScript : public Component
{
public:
	ComponentScript(ComponentType type, GameObject* game_object, const char* path);
	ComponentScript(ComponentType type, GameObject* game_object);
	~ComponentScript();

	void Update();

	void OnInspector(bool debug);

	void Save(Data& file)const;
	void Load(Data& conf);

	void SetPath(const char* path);

private:
	string path;

	bool started = false;
	bool finded_start;
	bool finded_update;
	int script_num;

	DWORD error;
};
#endif // !__COMPONENT_SCRIPT_H__
