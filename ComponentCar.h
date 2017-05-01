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
class PhysVehicle3D;
class ComponentAnimation;
class ComponentCollider;

enum CAR_TYPE
{
	T_WOOD,
	T_KOJI,

};
enum PLAYER
{
	PLAYER_1,
	PLAYER_2,
	PLAYER_3,
	PLAYER_4,
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

enum MAX_TURN_CHANGE_MODE
{
	M_SPEED,
	M_INTERPOLATION,
};

enum GROUND_CONTACT
{
	G_NONE,
	G_BEGIN,
	G_REPEAT,
	G_EXIT,
};

struct Car
{
	CAR_TYPE type = T_KOJI;
	//Turn direction
	float base_turn_max = 0.7f;
	float turn_speed = 1.5f;
	float turn_speed_joystick = 1.5f;
	float time_to_idle = 0.2f;
	bool  idle_turn_by_interpolation = true;

	//----Max turn change 
	float velocity_to_begin_change = 10.0f;
	float turn_max_limit = 0.01f;

	//By speed
	float base_max_turn_change_speed = -0.01f;
	float base_max_turn_change_accel = -0.1f;

	//----


	//Acceleration
	float accel_force = 1000.0f;
	float decel_brake = 100.0f;
	float max_velocity = 80.0f;
	float min_velocity = -20.0f;

	//Drifting
	float drift_turn_boost = 0.15f;
	float drift_turn_max = 0.7;
	float drift_min_speed = 20.0f;
	float drift_ratio = 0.5f;
	float drift_mult = 1.8f;
	float drift_boost = 1.0f;

	//Push
	float push_force = 10000.0f;
	float push_speed_per = 60.0f;

	//Acrobatics
	float acro_time = 0.5f;

	//Brake
	float brake_force = 20.0f;
	float back_force = 500.0f;

	//Full brake
	float full_brake_force = 300.0f;
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

	void SetFrontPlayer(PLAYER player);
	void SetBackPlayer(PLAYER player);

	void BlockInput(bool block);
	void TestFunction();

	//Getters
	float GetVelocity()const;

	
private:
	void CheckGroundCollision();
	void OnGroundCollision(GROUND_CONTACT state);
public:
	void HandlePlayerInput();
	void GameLoopCheck();
	void TurnOver();
	void Reset();
	void LimitSpeed();

	float GetVelocity();
	float GetMaxVelocity()const;
	float GetMinVelocity()const;
	float GetMaxTurnByCurrentVelocity(float sp);
	unsigned int GetFrontPlayer();
	unsigned int GetBackPlayer();
	PhysVehicle3D* GetVehicle();
	bool GetGroundState() const;
	float GetAngularVelocity() const;
	TURBO GetCurrentTurbo()const;
	Turbo* GetAppliedTurbo()const;

	void SetCarType(CAR_TYPE type);

private:
	void CreateCar();
	void UpdateGO();
	void JoystickControls(float* accel, float* brake, bool* turning);
	void KeyboardControls(float* accel, float* brake, bool* turning);

	//Render how the car will be. No need for the bullet car to be created, it's just a simulation
	void RenderWithoutCar();

	//Controls methods (to use in different parts)
	void Brake(float* accel, float* brake, bool with_trigger = false, float lt_joy_axis = 0);
	void FullBrake(float* brake);
	bool Turn(bool* left_turn, bool left);
	bool JoystickTurn(bool* left_turn, float x_joy_input);
	void LimitTurn();
	void Accelerate(float* accel, bool with_trigger = false, float rt_joy_axis = 0);
	void StartPush();
	bool Push(float* accel);
	void PushUpdate(float* accel);
	void Leaning(float accel);
	void Acrobatics(PLAYER p);

public:
	void PickItem();
	void UseItem(); //provisional
	void ReleaseItem();

	void AddHitodama();
	void RemoveHitodama();

private:
	void IdleTurn();
	void ApplyTurbo();

	void StartDrift();
	void CalcDriftForces();
	void EndDrift();
	void DriftTurbo();

	void UpdateTurnOver();

	void SetP2AnimationState(Player2_State state, float blend_ratio = 0.0f);
	void UpdateP2Animation();
	//----------------------------------------------------------------------------------------------------------------------------------------
	//
	//ATTRIBUTES----------------------------------------------------------------------------------------------------------------------------
	//
public:
	Car* kart = nullptr;
	Car wood;
	Car koji;

	Player2_State p2_state = P2IDLE;
	ComponentAnimation* p1_animation = nullptr;
	ComponentAnimation* p2_animation = nullptr;

	bool lock_input = false;
	uint team = 0;
	uint place = 1;
	bool finished = false;

private:
	bool raceStarted = false;

public:
	//Game Setting (Previous configuration) ----------------------------------------------------------------

	//Game Car settings ---------
	float3 chasis_size;
	float3 chasis_offset;

	float connection_height = 0.1f;
	float wheel_radius = 0.3f;
	float wheel_width = 0.2f;
	float suspensionRestLength = 0.3f;



	//Car mechanics settings --------
private:

	//Common in both cars----
	//Turn over
	float turn_over_reset_time = 5.0f;

	//Turn max change
	bool limit_to_a_turn_max = false;
	bool accelerated_change = false;

	MAX_TURN_CHANGE_MODE current_max_turn_change_mode = M_INTERPOLATION;
	bool show_graph = false;

	//Drift Turbo
	int clicks_to_drift_turbo = 3;

	//Reset
	float lose_height = 0.0f;

	//Turbos
	Turbo mini_turbo;
	Turbo drift_turbo_2;
	Turbo drift_turbo_3;

	//Unique for car type----

	//Turn direction
	/*float base_turn_max = 0.7f;
	float turn_speed = 1.5f;
	float turn_speed_joystick = 1.5f;
	float time_to_idle = 0.2f;
	bool  idle_turn_by_interpolation =	true;
	
	//----Max turn change 
	float velocity_to_begin_change = 10.0f;
	float turn_max_limit = 0.01f;

	//By speed
	float base_max_turn_change_speed = -0.01f;
	float base_max_turn_change_accel = -0.1f;

	//----


	//Acceleration
	float accel_force = 1000.0f;
	float decel_brake = 100.0f;
	float max_velocity = 80.0f;
	float min_velocity = -20.0f;

	//Drifting
	float drift_turn_boost = 0.15f;
	float drift_turn_max = 0.7;
	float drift_min_speed = 20.0f;
	float drift_ratio = 0.5f;
	float drift_mult = 1.8f;
	float drift_boost = 1.0f;

	//Push
	float push_force = 10000.0f;
	float push_speed_per = 60.0f;

	//Acrobatics
	float acro_time = 0.5f;

	//Brake
	float brake_force = 20.0f;
	float back_force = 500.0f;

	//Full brake
	float full_brake_force = 300.0f;*/





	//Rocket item
public:
	Turbo rocket_turbo;
private:
	
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
	float turn_max;
	float turn_current = 0.0f;
	bool turning_left = false;

	//Drift
	bool drifting = false;
	btVector3 startDriftSpeed;
	bool to_drift_turbo = false;
	bool drift_dir_left = false;
	int turbo_drift_lvl = 0;
	int drift_turbo_clicks = 0;

	//Leaning
	bool leaning = false;

	//Pushing
	double pushStartTime = 0.0f;
	bool pushing = false;

	//Acrobatics
	bool acro_front = false;
	bool acro_back = false;
	bool acro_on = false;
	bool acro_done = false;
	float acro_timer = 0.0f;
	
	//Turbo
	public:
	TURBO current_turbo = T_IDLE;
	private:
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

	//Ground contact
	bool on_ground = false;

	//Reset
	float3 reset_pos;
	Quat reset_rot;

	//Turbos vector
	//NOTE: this exist because i'm to lazy to write all the stats of the turbos on the inspector, save and load
	vector<Turbo> turbos;

	//Hitodamas
	int num_hitodamas = 0;

	//  Checkpoint variables----------------------------------------------------------------------------------------------------------------------------------------
	public:
		void WentThroughCheckpoint(int checkpoint, float3 resetPos, Quat resetRot);
		void WentThroughEnd(int checkpoint, float3 resetPos, Quat resetRot);
		uint checkpoints = MAXUINT - 10;
		float3 last_check_pos = float3::zero;
		Quat last_check_rot = Quat::identity;
		unsigned int lap = 0;

		unsigned int n_checkpoints = 0;

};


#endif // !_COMPONENT_CAR_H_

