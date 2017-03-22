#ifndef __MODULESCENEINTRO_H__
#define __MODULESCENEINTRO_H__

#include "Module.h"
#include "Globals.h"
#include <list>

#include "MathGeoLib\include\MathGeoLib.h"
#include "MathGeoLib\include\MathBuildConfig.h"

struct Mesh;
class GameObject;
class ComponentTransform;
class ComponentCamera;

using namespace std;

class ModuleSceneIntro : public Module
{
public:
	ModuleSceneIntro(const char* name, bool start_enabled = true);
	~ModuleSceneIntro();

	bool Init(Data& config);

	bool Start();
	update_status Update();
	bool CleanUp();

private:

	GameObject* FindGameObject(GameObject* start, const string& name)const;

private:
	//Hacks to make the water work. This should go inside a script.
	ComponentTransform* maincam = nullptr;
	ComponentTransform* reflection_tra = nullptr;
	bool water_hack_enabled = false;
};
#endif // !__MODULESCENEINTRO_H__
