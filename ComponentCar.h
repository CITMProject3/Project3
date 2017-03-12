#ifndef _COMPONENT_CAR_H_
#define _COMPONENT_CAR_H_

#include "Component.h"
#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include "Bullet\include\btBulletDynamicsCommon.h"
#include "Bullet\include\btBulletCollisionCommon.h"

struct VehicleInfo;
struct PhysVehicle3D;

enum PLAYER
{
	PLAYER_1,
	PLAYER_2,
};
class ComponentCar : public Component
{
	//
	//METHODS---------------------------------------------------------------------------------------------------------------------------
	//
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
	void GameLoopCheck();
	void Reset();
	void LimitSpeed();


private:
	void CreateCar();
	void UpdateGO();
	void JoystickControls(float* accel, float* brake, bool* turning);
	void KeyboardControls(float* accel, float* brake, bool* turning);

	//Render how the car will be. No need for the bullet car to be created, it's just a simulation
	void RenderWithoutCar();

	//Controls methods (to use in different parts)
	void Brake(float* accel, float* brake);
	bool Turn(bool* left_turn, bool left);
	bool JoystickTurn(bool* left_turn, float x_joy_input);
	void Accelerate(float* accel);
	bool Push(float* accel);
	void IdleTurn();

	//----------------------------------------------------------------------------------------------------------------------------------------
	//
	//ATTRIBUTES----------------------------------------------------------------------------------------------------------------------------
	//
public:
	float3 chasis_size;
	float3 chasis_offset;
	float kickCooldown = 3.0f;
	float kick_force_time = 2.0f;
	bool  on_kick = false;
	bool turning_left = false;

private:
	float kickTimer = 0.0f;
public:

	float connection_height = 0.1f;
	float wheel_radius = 0.3f;
	float wheel_width = 0.2f;
	float suspensionRestLength = 0.3f;

	//Game Setting (Previous configuration) ----------------------------------------------------------------

	//Game Car settings ---------
	float max_velocity = 80.0f;

	//Car mechanics settings --------

	//Drifting
	float drift_ratio = 0.5f;
	float drift_mult = 1.0f;
	float drift_boost = 1.0f;

	//Turn direction
	float turn_max = 0.7f;
	float turn_speed = 0.1f;

	//Acceleration
	float accel_force = 1000.0f;

	//Push
	float push_force = 10000.0f;
	float push_speed_per = 60.0f;


	//Brake
	float brake_force = 20.0f;
	float back_force = 500.0f;

	//Kick turbo
	float force = 1000.0f;

	//Reset
	float lose_height = 0.0f;
	float3 reset_pos;
	float3 reset_rot;

	
	//Update variables (change during game)----------------------------------------------------------------

	//Car  general variables------

	float velocity_current = 0.0f;

	//Car mechanics variables --------

	//Turn
	float turn_current = 0.0f;

	//Acrobatics
	bool acrobatics = false;

	

	btVector3 startDriftSpeed;

	VehicleInfo* car = nullptr;
	PhysVehicle3D* vehicle = nullptr;

	bool drift_no_phys = true;
	bool drift_phys = false;

	//2 Player configuration
	PLAYER front_player;
	PLAYER back_player;

	//----------------------------------------------------------------------------------------------------------------------------------------
};


#endif // !_COMPONENT_CAR_H_

