#ifndef _COMPONENT_CAR_H_
#define _COMPONENT_CAR_H_

#include "Component.h"
#include "Globals.h"
#include <vector>
#include <string>
#include "MathGeoLib\include\MathGeoLib.h"
#include "Bullet\include\btBulletDynamicsCommon.h"
#include "Bullet\include\btBulletCollisionCommon.h"

using namespace std;

struct VehicleInfo;
struct PhysVehicle3D;
class ComponentAnimation;
class ComponentCollider;

enum PLAYER
{
	PLAYER_1,
	PLAYER_2,
};

enum Player2_State
{
	P2IDLE,
	P2DRIFT_LEFT,
	P2DRIFT_RIGHT,
	P2PUSH_START,
	P2PUSH_LOOP,
	P2PUSH_END,
	P2LEANING,
};

enum TURBO
{
	T_IDLE,
	T_MINI,
	T_DRIFT_MACH_2,
	T_DRIFT_MACH_3,
	T_ROCKET,
};

struct Turbo
{
	string name;

	float accel_boost;
	float speed_boost;
	float turbo_speed;
	float deceleration = 1.0f;
	float fake_accel = 10.0f;

	bool per_ac = false;
	bool per_sp = false;
	bool speed_direct = false;
	bool speed_decrease = false;
	bool speed_increase = false;


	float time;
	float timer = 0.0;

	void SetTurbo(string n, float a, float v, float t)
	{
		name = n;
		accel_boost = a;
		speed_boost = v;
		time = t;
	}
};

class ComponentCar : public Component
{
	//
	//METHODS---------------------------------------------------------------------------------------------------------------------------
	//
public:

	ComponentCar(GameObject* GO);
	~ComponentCar();

	//Inherited Component Methods
	void Update();

	void OnTransformModified();
	void Save(Data& file) const;
	void Load(Data& config);
	void OnInspector(bool debug);

	void OnPlay();

	//Getters
	float GetVelocity()const;

	//Input handler during Game (import in the future to scripting)
	void HandlePlayerInput();
	void GameLoopCheck();
	void TurnOver();
	void Reset();
	void TrueReset();
	void LimitSpeed();

	float GetVelocity();


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
	void StartPush();
	bool Push(float* accel);
	void Leaning(float accel);
	void Acrobatics(PLAYER p);
	void PickItem();
	void UseItem(); //provisional
	void ReleaseItem();
	void IdleTurn();
	void ApplyTurbo();

	void StartDrift();
	void CalcDriftForces();
	void EndDrift();

	void UpdateTurnOver();

	void SetP2AnimationState(Player2_State state, float blend_ratio = 0.0f);
	void UpdateP2Animation();
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

	bool drift_dir_left = false;
	Player2_State p2_state = P2IDLE;

	//TODO: provisional
	GameObject* item = nullptr;
private:
	float kickTimer = 0.0f;
public:

	//Drifting control variables
	float drift_ratio = 0.5f;
	float drift_mult = 1.8f;
	float drift_boost = 1.0f;

	float connection_height = 0.1f;
	float wheel_radius = 0.3f;
	float wheel_width = 0.2f;
	float suspensionRestLength = 0.3f;

	//Game Setting (Previous configuration) ----------------------------------------------------------------

	//Game Car settings ---------
	

	//Car mechanics settings --------
private:
	ComponentAnimation* p1_animation = nullptr;
	ComponentAnimation* p2_animation = nullptr;

	//Turn direction
	float turn_max = 0.7f;
	float turn_speed = 0.1f;

	//Acceleration
	float accel_force = 1000.0f;
	float decel_brake = 100.0f;
	float max_velocity = 80.0f;
	float min_velocity = -20.0f;


	//Drifting
	float drift_turn_boost = 0.15f;
	float drift_min_speed = 20.0f;

	//Push
	float push_force = 10000.0f;
	float push_speed_per = 60.0f;

	//Leaning
	float lean_top_sp = 25.0f;
	float lean_top_acc = 25.0f;
	float lean_red_turn = 25.0f;

	//Acrobatics
	float acro_time = 0.5f;

	//Brake
	float brake_force = 20.0f;
	float back_force = 500.0f;

	//Reset
	float lose_height = 0.0f;
	float3 reset_pos;
	float3 reset_rot;

	//Turbos
	Turbo mini_turbo;
	Turbo drift_turbo_2;
	Turbo drift_turbo_3;

	//Rocket item
	//WARNING: THIS WILL HAVE TO be in a better structure, provisional for vertical slice
	Turbo rocket_turbo;
	
	//Update variables (change during game)----------------------------------------------------------------

	//Car  general variables------
	float velocity_current = 0.0f;

	//Car mechanics variables --------
	float top_velocity = 0.0f;

	//Turn over
	bool turned = false;
	float timer_start_turned = 0.0f;

	//Boosts
	float accel_boost = 0.0f;
	float speed_boost = 0.0f;
	float turn_boost = 0.0f;

	//Acceleration
	float accel = 0.0f;

	//Turn
	float turn_current = 0.0f;
	bool turning_left = false;

	//Drift
	bool drifting = false;
	btVector3 startDriftSpeed;
	bool to_drift_turbo = false;
	int turbo_drift_lvl = 0;

	//Leaning
	bool leaning = false;

	//Pushing
	double pushStartTime = 0.0f;
	bool pushing = false;

	//Acrobatics
	bool acro_front = false;
	bool acro_back = false;
	bool acro_on = false;
	float acro_timer = 0.0f;
	
	//Turbo
	TURBO current_turbo = T_IDLE;
	TURBO last_turbo = T_IDLE;
	Turbo* applied_turbo = nullptr;
	float turbo_accel_boost = 0.0f;
	float turbo_speed_boost = 0.0f;
	float turbo_deceleration = 0.0f;
	float turbo_acceleration = 0.0f;
	float current_speed_boost = 0.0f;
	bool speed_boost_reached = false;
	bool to_turbo_speed = false;
	bool to_turbo_decelerate = false;

	VehicleInfo* car = nullptr;
	PhysVehicle3D* vehicle = nullptr;

	std::vector<GameObject*> wheels_go;

	//2 Player configuration
	PLAYER front_player;
	PLAYER back_player;

	//Items! - provisional
	bool has_item = false;
	//Turbos vector
	//NOTE: this exist because i'm to lazy to write all the stats of the turbos on the inspector, save and load
	vector<Turbo> turbos;

	//  TMP variables----------------------------------------------------------------------------------------------------------------------------------------
	public:
	void WentThroughCheckpoint(ComponentCollider* checkpoint);
	void WentThroughEnd(ComponentCollider* end);
	unsigned char checkpoints = 255;
	GameObject* lastCheckpoint = nullptr;
	unsigned int lap = 0;

};


#endif // !_COMPONENT_CAR_H_

