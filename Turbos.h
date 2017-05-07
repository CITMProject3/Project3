#ifndef  __TURBOS__
#define __TURBOS__

#include "Globals.h"
#include "Time.h"

#include "MathGeoLib\include\MathGeoLib.h"

struct turboOutput
{
	bool alive = true;

	float accelerationBonus = 0.0f;
	float accelerationMin = 0.0f;
	float maxSpeedBonus = 0.0f;
};

class Turbo
{
public:
	//Acceleration time; Turbo time; Decceleration time; Acceleration bonus; Acceleration minimum; Max Speed Bonus
	Turbo(float accel_phase, float turbo_phase, float deccel_phase, float accel_bonus, float accel_min, float speed_bonus) :
		accelerationBonus(accel_bonus), accelerationMinimum(accel_min), maxSpeedBonus(speed_bonus), currentPhase(turbo_done)
	{
		times[turbo_accelerating] = accel_phase;
		times[turbo_running] = turbo_phase;
		times[turbo_deaccelerating] = deccel_phase;
	}

	Turbo()
	{
		times[turbo_accelerating] = 0.0f;
		times[turbo_running] = 0.0f;
		times[turbo_deaccelerating] = 0.0f;
	}

	~Turbo()
	{
	}

private:
	enum turbo_phases {
		turbo_accelerating = 0,
		turbo_running = 1,
		turbo_deaccelerating = 2,
		turbo_done = 3
	};
	turbo_phases currentPhase = turbo_done;

	float times[turbo_done + 1];
	//First phase
	//The kart gains a huge acceleration bonus for a short duration
	//Amount the acceleration of the kart increases
	float accelerationBonus = 0.2f;
	//If the player isn't accelerating, this minimum acceleration will still be applied
	float accelerationMinimum = 0.2f;


	//Second phase
	//The kart acceleration goes back to normal, but the maximum speed limit is still abnormaly big
	//Amount the maxSpeed of the kart increases
	float maxSpeedBonus = 1.0f;


	//Third phase
	//Time it takes for the max speed of the kart to go back to normal


	float timer = 0.0;

public:
	void TurnOn()
	{
		currentPhase = (turbo_phases)0;
	}

	turbo_phases GetCurrentPhase() { return currentPhase; }

	bool IsActive() { return currentPhase != turbo_done; }

	void SetTurbo(float accel_phase, float turbo_phase, float deccel_phase, float accel_bonus, float accel_min, float speed_bonus)
	{
		times[turbo_accelerating] = accel_phase;
		times[turbo_running] = turbo_phase;
		times[turbo_deaccelerating] = deccel_phase;

		accelerationBonus = accel_bonus;
		accelerationMinimum = accel_min;

		maxSpeedBonus = speed_bonus;
	}

	void SetTurbo(const Turbo& copy)
	{
		times[turbo_accelerating] = copy.times[turbo_accelerating];
		times[turbo_running] = copy.times[turbo_running];
		times[turbo_deaccelerating] = copy.times[turbo_deaccelerating];

		accelerationBonus = copy.accelerationBonus;
		accelerationMinimum = copy.accelerationMinimum;

		maxSpeedBonus = copy.maxSpeedBonus;

		currentPhase = turbo_done;
	}

	turboOutput UpdateTurbo(float dt)
	{
		turboOutput ret;

		switch (currentPhase)
		{
		case Turbo::turbo_accelerating:
		{
			ret.accelerationMin = accelerationMinimum;
			ret.accelerationBonus = accelerationBonus;
		}
		case Turbo::turbo_running:
		{
			ret.maxSpeedBonus = maxSpeedBonus;
			break;
		}
		case Turbo::turbo_deaccelerating:
		{
			//Making the speed bonus go to 0 while the time goes on
			ret.maxSpeedBonus = math::Lerp(0.0f, maxSpeedBonus, timer / times[turbo_deaccelerating]);
			break;
		}
		case Turbo::turbo_done:
		{
			ret.alive = false;
			return ret;
		}
		}

		timer += dt;
		while (currentPhase < turbo_done && timer >= times[currentPhase])
		{
			timer = 0.0f;
			currentPhase = (turbo_phases)(currentPhase + 1);
		}		

		return ret;
	}
};



//To add turbos, create a new one here

class turboPicker_class {

public:
	Turbo turboPad = Turbo(0.1f, 0.2f, 0.5f, 0.8f, 0.4f, 0.6f);
	Turbo acrobatic = Turbo(0.5f, 1.2f, 1.4f, 0.5f, 0.6f, 0.8f);

	//We have two turbos for the rocket: One that will activate when using the item. If this turbo goes into (turbo_done), the rocket will explode.
	//If the player releases the rocket, we'll apply rocket_deacceleration to slow the player down
	Turbo rocket = Turbo(0.5f, 0.5f, 0.0f, 0.3f, 0.2f, 0.4f);
	Turbo rocket_deacceleration = Turbo(0.0f, 0.0f, 0.5f, 0.3f, 0.2f, 0.4f);
};

/*
For reference

//Turbo
mini_turbo.SetTurbo("Mini turbo", 300.0f, 25.0f, 1.0f);
turbos.push_back(mini_turbo);

drift_turbo_2.SetTurbo("Drift turbo 2", 300.0f, 35.0f, 1.0f);
turbos.push_back(drift_turbo_2);

drift_turbo_3.SetTurbo("Drift turbo 3", 300.0f, 45.0f, 2.0f);
turbos.push_back(drift_turbo_3);

turbo_pad.SetTurbo("Turbo Pad", 300.0f, 200.0f, 1.5f);
turbos.push_back(turbo_pad);

//Item
rocket_turbo.SetTurbo("Rocket turbo", 0.0f, 50.0f, 5.0f);
*/


#endif // ! __TURBOS__
