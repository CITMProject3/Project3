#ifndef _COMPONENT_CAR_H_
#define _COMPONENT_CAR_H_

#include "Component.h"


struct VehicleInfo;
class PhysVehicle3D;
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
	bool CleanUp();

	void UpdateGO();

public:

	VehicleInfo* car;
	PhysVehicle3D* vehicle = nullptr;

	float kickCooldown = 3.0f;
	float kickTimer = 0.0f;

};


#endif // !_COMPONENT_CAR_H_

