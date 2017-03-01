#ifndef __MODULECAR_H__
#define __MODULECAR_H__

#include "Module.h"
#include "Globals.h"
#include <vector>

class GameObject;
class ComponentTransform;

#define CAR_GRAVITY 1.5f

#define NAVMESH_LAYER 20

using namespace std;

class ModuleCar : public Module
{
public:
	//Car status info
	bool onTheGround = true;
	bool steering = false;

	//Car functionality values
	float maxSpeed = 0.5f;
	float maxAcceleration = 0.2f;
	float brakePower = 0.5f;
	float maneuverability = 6.0f;
	float maxSteer = 160.0f;
	float drag = 0.3f;
	//Time that takes a car on the air to put itself straighta again
	float recoveryTime = 2.0f;

private:
	float speed = 0.0f;
	float currentSteer = 0.0f;

	float fallSpeed = 0.0f;

	bool loaded = false;
	bool wantToLoad = false;
	bool firstFrameOfExecution = true;

	GameObject* kart = nullptr;
	GameObject* chasis = nullptr;
	GameObject* frontWheel = nullptr;
	GameObject* backWheel = nullptr;
	GameObject* cam = nullptr;

	ComponentCamera* camera = nullptr;

	std::vector<GameObject*> track;
	GameObject* light = nullptr;

	ComponentTransform* kart_trs = nullptr;

	char tmpOutput[1024];

public:
	ModuleCar(const char* name, bool start_enabled = true);
	~ModuleCar();

	bool Init(Data& config);
	bool Start();
	update_status PreUpdate();
	update_status Update();

	void Load();
private:
	void LoadNow();

	void KartLogic();

	float AccelerationInput();
	void Steer(float amount);
	void AutoSteer();

	void Car_Debug_Ui();
	void FindKartGOs();
	
};

#endif // !__MODULECAR_H__
