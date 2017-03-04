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

public:
	float3 chasis_size;
	float3 chasis_offset;
	float kickCooldown = 3.0f;
	float kickTimer = 0.0f;

	float connection_height = 0.2f;
	float wheel_radius = 0.3f;
	float wheel_width = 0.2f;
	float suspensionRestLength = 0.6f;

	float dturn = 0.7f;
	float force = 100000.0f;
	float brakeForce = 20.0f;

	VehicleInfo* car = nullptr;
	PhysVehicle3D* vehicle = nullptr;
};


#endif // !_COMPONENT_CAR_H_

