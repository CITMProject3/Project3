#ifndef __COMPONENT_SCRIPT_H__
#define __COMPONENT_SCRIPT_H__

#include "Component.h"
#include "Globals.h"
#include <string>

class Application;
class GameObject;

using namespace std;

typedef void(*f_start)(Application* engine_app, GameObject* game_object);
typedef void(*f_update)(Application* engine_app, GameObject* game_object);

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
	HINSTANCE script;

	string path;

	bool started = false;
};
#endif // !__COMPONENT_SCRIPT_H__
