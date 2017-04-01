#ifndef __COMPONENT_SCRIPT_H__
#define __COMPONENT_SCRIPT_H__

#include "Component.h"
#include "Globals.h"
#include <string>
#include <map>

class Application;
class GameObject;
struct PhysBody3D;

using namespace std;

typedef void(*f_Start)(GameObject* game_object);
typedef void(*f_Update)(GameObject* game_object);
typedef void(*f_OnCollision)(PhysBody3D* col);

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
	void SetGOVar(GameObject* game_object);

	void OnCollision(PhysBody3D* col);

private:
	string path;

	bool started = false;
	bool finded_start;
	bool finded_update;
	int script_num;
	string filter = "";

	DWORD error;

	map<const char*, unsigned int> tmp_public_gos_uuint;
	bool public_gos_to_set;

public:
	map<const char*, string> public_chars;
	map<const char*, int> public_ints;
	map<const char*, float> public_floats;
	map<const char*, bool> public_bools;
	map<const char*, GameObject*> public_gos;
};
#endif // !__COMPONENT_SCRIPT_H__
