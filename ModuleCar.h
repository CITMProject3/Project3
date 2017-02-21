#ifndef __MODULECAR_H__
#define __MODULECAR_H__

#include "Module.h"
#include "Globals.h"
#include <vector>

class GameObject;
class ComponentTransform;

//This should be now in my branch, so probably no one will see this. Just testing :)

using namespace std;

class ModuleCar : public Module
{
public:
	
	float speed = 10.0f;
	float rotateSpeed = 120.0f;

	bool loaded = false;
	GameObject* kart = nullptr;
	GameObject* chasis = nullptr;
	GameObject* frontWheel = nullptr;
	GameObject* backWheel = nullptr;
	GameObject* cam = nullptr;

	ComponentTransform* kart_trs = nullptr;

	ModuleCar(const char* name, bool start_enabled = true);
	~ModuleCar();

	bool Init(Data& config);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	
};

#endif // !__MODULECAR_H__
