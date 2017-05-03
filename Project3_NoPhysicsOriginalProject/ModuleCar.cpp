#include "Globals.h"
#include "Application.h"
#include "ModuleCar.h"

#include "ModuleResourceManager.h"
#include "ModuleGOManager.h"
#include "ComponentTransform.h"
#include "Editor.h"
#include "Assets.h"
#include "GameObject.h"
#include "RaycastHit.h"

#include "ModuleInput.h"

#include "glmath.h"

#include "imgui\imgui.h"

#define DISTANCE_FROM_GROUND 1.0

void ModuleCar::KartLogic()
{
	float3 pos = kart_trs->GetPosition();
	float3 newPos = pos;
	float3 kartY = kart_trs->GetGlobalMatrix().WorldY();
	kartY.Normalize();

	//Setting the two rays, Front and Back
	math::Ray rayF, rayB;
	rayB.dir = rayF.dir = -kartY;
	rayB.pos = rayF.pos = kart_trs->GetPosition() + kartY;
	rayF.pos += kart_trs->GetGlobalMatrix().WorldZ().Normalized();
	rayB.pos -= kart_trs->GetGlobalMatrix().WorldZ().Normalized();

	//Raycasting, checking only for the NavMesh layer
	RaycastHit hitF = App->go_manager->Raycast(rayF, std::vector<int>(1, NAVMESH_LAYER));
	RaycastHit hitB = App->go_manager->Raycast(rayB, std::vector<int>(1, NAVMESH_LAYER));

	bool checkOffTrack = false;

	//Setting the "desired up" value, taking in account if both rays are close enough to the ground or none of them
	float3 desiredUp = float3(0, 1, 0);
	onTheGround = false;
	if ((hitF.object != nullptr && hitF.distance < DISTANCE_FROM_GROUND + 1) && (hitB.object != nullptr && hitB.distance < DISTANCE_FROM_GROUND + 1))
	{
		//In this case, both rays hit an object and are close enough to it to be considered "On The ground". Desired up is an interpolation of the normal output of both raycasts
		onTheGround = true;
		desiredUp = hitF.normal.Lerp(hitB.normal, 0.5f);
		newPos = hitB.point + (hitF.point - hitB.point) / 2;
	}
	else if ((hitF.object != nullptr && hitF.distance < DISTANCE_FROM_GROUND/2.0 + 1) && !(hitB.object != nullptr && hitB.distance < DISTANCE_FROM_GROUND + 0.8))
	{
		//Only the front ray collided. We'll need more comprovations to make sure the kart is not going off track
		onTheGround = true;
		desiredUp = hitF.normal;
		checkOffTrack = true;
	}
	else if (!(hitF.object != nullptr && hitF.distance < DISTANCE_FROM_GROUND/2.0 + 1) && (hitB.object != nullptr && hitB.distance < DISTANCE_FROM_GROUND + 0.8))
	{
		//Only the back ray collided. We'll need more comprovations to make sure the kart is not going off track
		onTheGround = true;
		desiredUp = hitB.normal;
		checkOffTrack = true;
	}

	if (checkOffTrack && onTheGround)
	{
		//Checking if the kart is still on the track
		//This ray shoots from the center of the kart, vertically down. It checks if the kart is still on the track or is about to fall off and should simulate a collision
		math::Ray rayN;
		rayN.dir = float3(0, -1, 0);
		rayN.pos = kart_trs->GetPosition() + float3(0, 1, 0);
		//Raycasting, checking only for the NavMesh layer
		RaycastHit hitN = App->go_manager->Raycast(rayN, std::vector<int>(1, NAVMESH_LAYER));

		if (hitN.object == nullptr)
		{
			//It didn't hit! Simulate a collision!
			rayN.pos += kart_trs->GetGlobalMatrix().WorldX();
			RaycastHit hitR = App->go_manager->Raycast(rayN, std::vector<int>(1, NAVMESH_LAYER));
			rayN.pos -= kart_trs->GetGlobalMatrix().WorldX() * 2;
			RaycastHit hitL = App->go_manager->Raycast(rayN, std::vector<int>(1, NAVMESH_LAYER));

			newPos += kart_trs->GetGlobalMatrix().WorldZ() * speed;

			speed -= speed / 3.0f;

			if (hitL.object)
			{
				horizontalSpeed = speed * -1.5f;
			}
			else if(hitR.object)
			{
				horizontalSpeed = speed * 1.5f;
			}
			else
			{
				speed = -speed;
				newPos += kart_trs->GetGlobalMatrix().WorldZ() * speed;
			}
		}
	}

	desiredUp.Normalize();

	if (desiredUp.AngleBetweenNorm(kartY) > DEGTORAD * 3.0f)
	{
		//Interpolating to obtain the desired rotation
		float3 nextStep;
		if (onTheGround)
		{
			nextStep = kartY.Lerp(desiredUp, 2.0f * time->DeltaTime());
		}
		else
		{
			nextStep = kartY.Lerp(desiredUp, (1.0f/recoveryTime) * time->DeltaTime());
		}
		Quat normal_rot = Quat::RotateFromTo(kartY, nextStep);
		kart_trs->Rotate(normal_rot);
	}


#pragma region drag_for_horizontalSpeed
	if (math::Abs(horizontalSpeed) > drag * 4.0f * time->DeltaTime())
	{
		if (horizontalSpeed > 0)
		{
			horizontalSpeed -= drag * 4.0f * time->DeltaTime();
		}
		else
		{
			horizontalSpeed += drag * 4.0f * time->DeltaTime();
		}
	}
	else
	{
		horizontalSpeed = 0.0f;
	}
#pragma endregion

	//Manage Input to accelerate/brake. Returns acceleration
	if (onTheGround)
	{
		speed += AccelerationInput();
		speed = math::Clamp(speed, -maxSpeed, maxSpeed);		
	}
	horizontalSpeed = math::Clamp(horizontalSpeed, -maxSpeed, maxSpeed);

	//Steering
	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) { Steer(1); }
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) { Steer(-1); }
	Steer(App->input->GetJoystickAxis(0, JOY_AXIS::LEFT_STICK_X));

	//Returning steer to 0 gradually if the player isn't inputting anything
	if (steering == false)
	{
		AutoSteer();
	}
	steering = false;

	if (onTheGround)
	{
		//This simply translataes current steer into a Quaterion we can rotate the kart with
		//currentSteer goes from -1 to 1, and we multiply it by the "Max Steer" value.
		//The "Clamp" thing is to allow less rotation the less the kart is moving
		//The kart can rotate the maximum amount when it goes faster than maxSpeed / 3
		float rotateAngle = maxSteer * math::Clamp(speed / (maxSpeed / 3), -1.0f, 1.0f) * currentSteer * time->DeltaTime();
		Quat tmp = kart_trs->GetRotation().RotateAxisAngle(kartY, -rotateAngle * DEGTORAD);
		kart_trs->Rotate(tmp);

		fallSpeed = 0.0f;
	}
	else
	{
		//Falling. Magic. Gravity. So much wow.
		fallSpeed += CAR_GRAVITY * time->DeltaTime();
		newPos.y -= fallSpeed;
	}

#pragma region Esthetic_rotation_chasis_wheels
	if (chasis)
	{
		ComponentTransform* chasis_trs = (ComponentTransform*)chasis->GetComponent(C_TRANSFORM);
		chasis_trs->SetRotation(float3(0, -currentSteer * 15 * math::Clamp(speed / (maxSpeed / 3), -1.0f, 1.0f), 0));
	}
	if (frontWheel)
	{
		ComponentTransform* wheel_trs = (ComponentTransform*)frontWheel->GetComponent(C_TRANSFORM);
		wheel_trs->SetRotation(float3(0, -currentSteer * 15, 0));
	}
#pragma endregion

	//And finally, we move the kart!
	newPos += kart_trs->GetGlobalMatrix().WorldZ() * speed;
	newPos += kart_trs->GetGlobalMatrix().WorldX() * horizontalSpeed;
	kart_trs->SetPosition(newPos);


	//Safety mesure. Resetting the kart if it falls off the world. jej
	if (kart_trs->GetPosition().y < -200)
	{
		kart_trs->SetPosition(float3(0, 1, 0));
		speed = 0.0f;
		fallSpeed = 0.0f;
	}
}

float ModuleCar::AccelerationInput()
{
	float acceleration = 0.0f;
	//Accelerating
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT || App->input->GetJoystickButton(0, JOY_BUTTON::A))
	{
		if (speed < -0.01f)
		{
			if (speed < -brakePower * time->DeltaTime())
			{
				acceleration += brakePower * time->DeltaTime();
			}
			else
			{
				speed = 0;
			}
		}
		else
		{
			acceleration += maxAcceleration * time->DeltaTime();
		}
	}
	//Braking
	else if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT || App->input->GetJoystickButton(0, JOY_BUTTON::B))
	{
		if (speed > 0.01f)
		{
			if (speed > brakePower * time->DeltaTime())
			{
				acceleration -= brakePower * time->DeltaTime();
			}
			else
			{
				speed = 0;
			}
		}
		else
		{
			acceleration -= (maxAcceleration / 2.0f) * time->DeltaTime();
		}
	}
	//If there's no input, drag force slows car down
	else
	{
		if (speed > drag * time->DeltaTime())
		{
			acceleration = -drag * time->DeltaTime();
		}
		else if (speed < -drag * time->DeltaTime())
		{
			acceleration += drag * time->DeltaTime();
		}
		else
		{
			speed = 0;
		}
	}
	return acceleration;
}

void ModuleCar::Steer(float amount)
{
	amount = math::Clamp(amount, -1.0f, 1.0f);
	if (amount < -0.1 || amount > 0.1)
	{
		currentSteer += maneuverability * time->DeltaTime() * amount;
		amount = math::Abs(amount);
		currentSteer = math::Clamp(currentSteer, -amount, amount);
		steering = true;
	}
}

void ModuleCar::AutoSteer()
{
	//Whenever the player isn't steering, bring slowly the wheels back to the neutral position
	if (currentSteer > maneuverability * time->DeltaTime())
	{
		currentSteer -= maneuverability * time->DeltaTime();
	}
	else if (currentSteer < -maneuverability * time->DeltaTime())
	{
		currentSteer += maneuverability * time->DeltaTime();
	}
	else { currentSteer = 0; }
}

