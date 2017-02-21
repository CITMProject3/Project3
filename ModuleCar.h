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
	
	float maxSpeed = 0.5f;
	float maxAcceleration = 0.2f;
	float brakePower = 0.5f;
	float maneuverability = 6.0f;
	float maxSteer = 160.0f;
	float drag = 0.3f;

private:
	float speed = 0.0f;
	float currentSteer = 0.0f;
public:

	bool steering = false;

	bool loaded = false;
	bool firstFrameOfExecution = true;

	GameObject* kart = nullptr;
	GameObject* chasis = nullptr;
	GameObject* frontWheel = nullptr;
	GameObject* backWheel = nullptr;
	GameObject* cam = nullptr;

	ComponentCamera* camera = nullptr;

	GameObject* track = nullptr;

	ComponentTransform* kart_trs = nullptr;

	ModuleCar(const char* name, bool start_enabled = true);
	~ModuleCar();

	bool Init(Data& config);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	update_status PostUpdate();
	bool CleanUp();

	void KartLogic();

	void Steer(float amount);

	void Car_Debug_Ui();

	
};

#endif // !__MODULECAR_H__
