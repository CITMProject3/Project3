#ifndef __MODULECAR_H__
#define __MODULECAR_H__

#include "Module.h"
#include "Globals.h"
#include <vector>

#define MAX_ACCELERATION 1000.0f
#define TURN_DEGREES 15.0f * DEGTORAD
#define BRAKE_POWER 500.0f

struct PhysVehicle3D;
class GameObject;
class ComponentTransform;

using namespace std;

class ModuleCar : public Module
{
public:
	ModuleCar(const char* name, bool start_enabled = true);
	~ModuleCar();

	bool Init(Data& config);
	bool Start();
	update_status PreUpdate();
	update_status Update();

	GameObject* chasis = nullptr;

	void AddCar();

private:
	// Vehicle control
	PhysVehicle3D* vehicle = nullptr;
	float turn;
	float acceleration;
	float brake;
	
};

#endif // !__MODULECAR_H__
