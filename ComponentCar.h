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
#include "Wheel.h"
#include "RaycastHit.h"

#include "Timer.h"

#define CAR_GRAVITY 18.0f

//TODO fix conversion Units to KM/H
#define UNITS_TO_KMH 133.0f
#define KMH_TO_UNITS 0.0075f

using namespace std;

class ComponentAnimation;
class ComponentCollider;
class ComponentTransform;
class ComponentAudioSource;

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

enum DRIFT_STATE
{
	drift_none = 0,
	drift_failed,
	drift_right_0,
	drift_right_1,
	drift_right_2,
	drift_left_0,
	drift_left_1,
	drift_left_2
};

class MashButtonCounter
{
public:
	MashButtonCounter() {}
private:
	uint nTaps = 0;
public:
	void Update(uint button, uint player);
	void Reset() { nTaps = 0; }
	uint GetNTaps() { return nTaps; }
};

struct CarAttributeModifier
{
	float bonusMaxSpeed = 0.0f;
	float bonusBrakePower = 0.0f;
	float bonusMaxAcceleration = 0.0f;
	float bonusManeuverability = 0.0f;
	float bonusMaxSteering = 0.0f;
	float bonusDrag = 0.0f;
	float bonusWallBounciness = 0.0f;

	void Reset() { *this = CarAttributeModifier(); }
};

class ComponentCar : public Component
{
private:
	//Car status info
	bool onTheGround = true;
	bool lastFrameOnGround = true;

	bool steering = false;
	bool pushing = false;
	Timer push_timer;
	float time_between_pushes = 0.250f;

	//Car functionality values
	float maxSpeed = 1.2f;
	float maxAcceleration = 0.3f;
	float brakePower = 0.8f;
	float maneuverability = 5.0f;
	float maxSteer = 120.0f;
	float drag = 0.3f;
	float push_incr = 1.1f;
	float push_force = 0.001f;
	float max_push_force = 1.0f;
	float push_threshold = 0.70f; // Ratio below which push is applied
	
	float WallsBounciness = 0.5f;

	//Variable values that directly affect the car frame by frame
	float speed = 0.0f;
	float currentSteer = 0.0f;
	float horizontalSpeed = 0.0f;
	float fallSpeed = 0.0f;

	float maxSteerReduction = 0.5f;

	float testVar = 0.0f;

	ComponentTransform* kart_trs = nullptr;

	ComponentAudioSource *audio_source = nullptr;

	std::vector<Wheel> wheels;

public:
	CarAttributeModifier mods;

	float3 kartX, kartY, kartZ;	

	//Shadow Ray results
	bool shadowRayHit;
	RaycastHit shadowRayResult;

	//Collider
	Cube_P collShape;

	ComponentTransform* kart_front_wheel_l = nullptr;
	ComponentTransform* parent_front_l = nullptr;
	ComponentTransform* kart_front_wheel_r = nullptr;
	ComponentTransform* parent_front_r = nullptr;
	ComponentTransform* kart_back_wheel_l = nullptr;
	ComponentTransform* kart_back_wheel_r = nullptr;
private:
	float3 collOffset = float3(0, 0.8, 0);
	PhysBody3D* collider = nullptr;

	//Drifting
	DRIFT_STATE drifting;
	DRIFT_STATE lastFrame_drifting;

	int collisionwWhileDrifting = 0;
	int collisionsUntilStopDrifting = 5;
	float driftCollisionTimer = 0.0f;
	float driftCollisionRecovery = 3.0f;

	float driftAirTimer = 0.0f;

	MashButtonCounter driftButtonMasher;

	float acrobaticsDelay = 0.5f;
	Timer p1AcrobaticsPress;
	Timer p2AcrobaticsPress;
public:
	bool acrobaticsDone = false;
private:

	//Feedback stuff
	float4x4 kartTransform = float4x4::identity;

	float fb_jumpSpeed = 0.0f;
	float fb_verticalCorrection = 0.0f;
	float fb_vertical = 0.0f;
	float fb_rotation_Y = 0.0f;
	float fb_rotation_FW = 0.0f;

	//Inspector variables for drifting
	int driftPhaseChange = 3;

	float driftingFixedTurn = 0.85f;
	float current_driftingFixedTurn = 0.0f;

	float driftingAdjustableTurn = 0.65f;
	float steerChangeSpeed = 1.5f;
	float horizontalSpeedDrifting = 0.8f;

	float minimumSteerToStartDrifting = 0.6f;
	float minimumSpeedRatioToStartDrifting = 0.4f;

	float driftJumpSpeed = 10.0f;
	float driftJumpGravityMultiplier = 3.0f;
	float driftVisualTurn = 25.0f;

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
	ComponentTransform* GetKartTRS() { return kart_trs; }

private:
	void DebugInput();

	void CheckGroundCollision();
	void OnGroundCollision(GROUND_CONTACT state);

	void KartLogic();
	float AccelerationInput();
	void Steer(float amount);
	void AutoSteer();
	void CheckOnTheGround();
	void Drift(float dir);
	void DriftManagement();
	void PlayersInput();

	void SteerKart();
	void RotateKart(float3 desiredUp);
	void HorizontalDrag();

	void WallHit(const float3 &normal, const float3 &kartZ, const float3 &kartX);
public:
	void WallHit(const float3 &normal);

	void SetPos(float3 pos);
	bool wantToSetPos = false;
	float3 setpos;
	void SetRot(Quat rot);
	bool wantToSetRot = false;
	Quat setrot;
	void SetScale(float3 scale);
	bool wantToSetScale = false;
	float3 setscale;

	void Reset();
private:
	void TrueReset();
public:

	float hitWallTimer = 0.0f;
	float resetTimer = 0.0f;
	bool wantToReset = false;

	float GetVelocity();
	float GetMaxVelocity()const;
	const float *GetSpeed() const; // For RTPC Wwise used
	unsigned int GetFrontPlayer();
	unsigned int GetBackPlayer();
	bool GetGroundState() const;
	float GetAngularVelocity() const;
	Turbo GetAppliedTurbo()const;

	void SetCarType(CAR_TYPE type);

private:
	void CastShadowRay();
	void UpdateAnims();
	void UpdateRenderTransform();

public:
	void PickItem();
	void UseItem();

	bool GetInvertStatus() const;
	void SetInvertStatus(bool status);

	bool AddHitodama();
	bool RemoveHitodama();
	int GetNumHitodamas() const;

	void NewTurbo(Turbo turboToApply);
	void TurboPad();

	DRIFT_STATE GetDriftState()const;
private:

	void UpdateP1Animation();
	void SetP2AnimationState(Player2_State state, float blend_ratio = 0.0f);
	void UpdateP2Animation();
public:
	void OnGetHit(float velocity_reduction);
	//----------------------------------------------------------------------------------------------------------------------------------------
	//
	//ATTRIBUTES----------------------------------------------------------------------------------------------------------------------------
	//
public:

	Player1_State p1_state = P1IDLE;
	Player2_State p2_state = P2DRIFT_RIGHT;

	ComponentAnimation* p1_animation = nullptr;
	ComponentAnimation* p2_animation = nullptr;

	bool lock_input = false;
	uint team = 0;
	uint place = 1;
	bool finished = false;

private:
	bool raceStarted = false;

private:

	//Common in both cars----
	//Car mechanics settings --------
	bool inverted_controls = false;
	int invert_value = 1;
	//Reset
	float loose_height = 1.0f;


	//Update variables (change during game)----------------------------------------------------------------

	//Turbo
private:
	Turbo turbo;
	turboOutput turbo_mods;

	turboPicker_class turboPicker;

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

	int lastCheckpoint = 0;
	int last2Checkpoint = 0;
	bool wentThroughEnd = false;
	unsigned int n_checkpoints = 0;
	bool wrongDirection = false;
};


#endif // !_COMPONENT_CAR_H_

