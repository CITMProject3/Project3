#ifndef _COMPONENT_CAR_H_
#define _COMPONENT_CAR_H_

#include "Component.h"
#include "Globals.h"
#include <vector>
#include <string>
#include "MathGeoLib\include\MathGeoLib.h"
#include "Bullet\include\btBulletDynamicsCommon.h"
#include "Bullet\include\btBulletCollisionCommon.h"

#include "Turbos.h"

#include "Primitive.h"

#define CAR_GRAVITY 0.8f

//TODO fix conversion Units to KM/H
#define UNITS_TO_KMH 133.0f
#define KMH_TO_UNITS 0.0075f

using namespace std;

class ComponentAnimation;
class ComponentCollider;
class ComponentTransform;

class PhysBody3D;

enum CAR_TYPE
{
	T_KOJI,
	T_WOOD
};

enum PLAYER
{
	PLAYER_1,
	PLAYER_2,
	PLAYER_3,
	PLAYER_4
};

enum Player1_State
{
	P1IDLE,
	P1TURN,
	P1MAXTURN_L,
	P1MAXTURN_R,
	P1ACROBATICS,
	P1GET_HIT
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
	P2GET_HIT,
	P2USE_ITEM,
	P2ACROBATICS
};

enum MAX_TURN_CHANGE_MODE
{
	M_SPEED,
	M_INTERPOLATION
};

enum GROUND_CONTACT
{
	G_NONE,
	G_BEGIN,
	G_REPEAT,
	G_EXIT
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


class ComponentCar : public Component
{
private:
	//Car status info
	bool onTheGround = true;
	bool lastFrameOnGround = true;

	bool steering = false;

	//Car functionality values
	float maxSpeed = 1.2f;
	float maxAcceleration = 0.3f;
	float brakePower = 0.8f;
	float maneuverability = 5.0f;
	float maxSteer = 120.0f;
	float drag = 0.3f;
	//Time that takes a car on the air to put itself straight again
	float recoveryTime = 2.0f;

	float WallsBounciness = 0.75f;

	//Variable values that directly affect the car frame by frame
	float speed = 0.0f;
	float currentSteer = 0.0f;
	float horizontalSpeed = 0.0f;
	float fallSpeed = 0.0f;

	float3 kartX, kartY, kartZ;

	ComponentTransform* kart_trs = nullptr;

	//Collider
	Cube_P collShape;
	float3 collOffset = float3(0, 0.8, 0);
	PhysBody3D* collider = nullptr;


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

	//Getters
	float GetVelocity()const;


private:
	void DebugInput();

	void CheckGroundCollision();
	void OnGroundCollision(GROUND_CONTACT state);

	void KartLogic();
	float AccelerationInput();
	void Steer(float amount);
	void AutoSteer();
	void CheckOnTheGround();
	void Drift();
	void PlayersInput();

	void SteerKart();
	void RotateKart(float3 desiredUp);
	void HorizontalDrag();

	void WallHit(const float3 &normal, const float3 &kartZ, const float3 &kartX);
public:
	void WallHit(const float3 &normal);

	void HandlePlayerInput();
	void GameLoopCheck();
	void TurnOver();
	void Reset();

	float GetVelocity();
	float GetMaxVelocity()const;
	unsigned int GetFrontPlayer();
	unsigned int GetBackPlayer();
	bool GetGroundState() const;
	float GetAngularVelocity() const;
	Turbo GetAppliedTurbo()const;

	void SetCarType(CAR_TYPE type);

private:
	void UpdateGO();
	void JoystickControls(float* accel, float* brake, bool* turning, bool inverse = false);

	//Controls methods (to use in different parts)
	void StartPush();
	bool Push(float* accel);
	void PushUpdate(float* accel);
	void Leaning(float accel);
	void Acrobatics(PLAYER p);

public:
	void PickItem();
	void UseItem();

	bool AddHitodama();
	bool RemoveHitodama();
	int GetNumHitodamas() const;

	void NewTurbo(Turbo turboToApply);
	void TurboPad();
private:
	void DriftTurbo();

	void UpdateTurnOver();

	void UpdateP1Animation();
	void SetP2AnimationState(Player2_State state, float blend_ratio = 0.0f);
	void UpdateP2Animation();
	void OnGetHit();
	//----------------------------------------------------------------------------------------------------------------------------------------
	//
	//ATTRIBUTES----------------------------------------------------------------------------------------------------------------------------
	//
public:
	Car* kart = nullptr;
	Car wood;
	Car koji;

	bool drift_dir_left = false;

	Player1_State p1_state = P1IDLE;
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
	//Car mechanics settings --------
	bool inverted_controls;
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
	float loose_height = -100.0f;


	//Update variables (change during game)----------------------------------------------------------------

	//Turn over
	bool turned = false;
	float timer_start_turned = 0.0f;

	float turn_boost = 0.0f;

	//Drift
	bool drifting = false;
	btVector3 startDriftSpeed;
	bool to_drift_turbo = false;
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
private:
	Turbo turbo;
	turboOutput turbo_mods;

	turboPicker_class turboPicker;

	std::vector<GameObject*> wheels_go;

	//2 Player configuration
	PLAYER front_player;
	PLAYER back_player;

	//Items! - provisional
	bool has_item = false;

	//Reset
	float3 reset_pos;
	Quat reset_rot;

	//Hitodamas
	int num_hitodamas = 0;
	int max_hitodamas = 5;
	int bonus_hitodamas = 2;

	//  Checkpoint variables----------------------------------------------------------------------------------------------------------------------------------------
public:
	void WentThroughCheckpoint(int checkpoint, float3 resetPos, Quat resetRot);
	void WentThroughEnd(int checkpoint, float3 resetPos, Quat resetRot);
	uint checkpoints = MAXUINT - 10;
	float3 last_check_pos = float3::zero;
	Quat last_check_rot = Quat::identity;
	unsigned int lap = 1;

	unsigned int n_checkpoints = 0;
};


#endif // !_COMPONENT_CAR_H_

