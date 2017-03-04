#ifndef _COMPONENT_CAR_H_
#define _COMPONENT_CAR_H_

#include "Component.h"
#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"

struct VehicleInfo;
struct PhysVehicle3D;

class ComponentCar : public Component
{
public:

	//
	ComponentCar(GameObject* GO);
	~ComponentCar();

	//Inherited Component Methods
	void Update();

	void OnTransformModified();
	void Save(Data& file) const;
	void Load(Data& config);

	void OnInspector(bool debug);

	//Input handler during Game (import in the future to scripting)
	void HandlePlayerInput();

private:
	void CreateCar();
	void UpdateGO();

	//Render how the car will be. No need for the bullet car to be created, it's just a simulation
	void RenderWithoutCar();

public:
	float3 chasis_size;
	float3 chasis_offset;
	float kickCooldown = 3.0f;
private:
	float kickTimer = 0.0f;
public:

	float connection_height = 0.1f;
	float wheel_radius = 0.3f;
	float wheel_width = 0.2f;
	float suspensionRestLength = 0.6f;

	float turn_max = 0.7f;
	float turn_speed = 0.1f;


	float force = 10000.0f;
	float brakeForce = 20.0f;

	//update variables
	float turn_current = 0.0f;

	VehicleInfo* car = nullptr;
	PhysVehicle3D* vehicle = nullptr;
};


#endif // !_COMPONENT_CAR_H_

