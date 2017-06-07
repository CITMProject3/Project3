#include "ComponentCar.h"

#include "Application.h"
#include "ModulePhysics3D.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentAnimation.h"
#include "ComponentAudioSource.h"

#include "imgui/imgui.h"

#include "Primitive.h"
#include "EventQueue.h"
#include "EventLinkGos.h"

#include "ModuleGOManager.h"
#include "ComponentCanvas.h"

#include "ComponentCollider.h"

#include "Time.h"

#include <string>

#include "SDL\include\SDL_scancode.h"
#include "Brofiler\include\Brofiler.h"


#define DISTANCE_FROM_GROUND 1.3f

#define ITERATE_WHEELS for (std::vector<Wheel>::iterator it = wheels.begin(); wheels.empty() == false && it != wheels.end(); it++)

void ComponentCar::WallHit(const float3 &normal, const float3 &kartZ, const float3 &kartX)
{
	if (hitWallTimer > 2.0f)
	{
		float3 fw, side;

		normal.Decompose(kartZ, fw, side);

		Plane p;
		p.Set(kart_trs->GetPosition(), kartX);

		if (p.IsInPositiveDirection(normal))
		{
			horizontalSpeed += side.Length() * Clamp((math::Abs(speed) / maxSpeed), 0.2f, 9999.0f) * (WallsBounciness + mods.bonusWallBounciness);
			if (horizontalSpeed < 0) { horizontalSpeed = 0; }
		}
		else
		{
			horizontalSpeed -= side.Length() * Clamp((math::Abs(speed) / maxSpeed), 0.2f, 9999.0f) * (WallsBounciness + mods.bonusWallBounciness);
			if (horizontalSpeed > 0) { horizontalSpeed = 0; }
		}

		p.Set(kart_trs->GetPosition(), kartZ);
		if (p.IsInPositiveDirection(normal))
		{
			speed += fw.Length() * Clamp((math::Abs(speed) / maxSpeed), 0.2f, 9999.0f) * (WallsBounciness + mods.bonusWallBounciness);
			if (speed < 0 && fw.LengthSq() > side.LengthSq()) { speed = 0; }
		}
		else
		{
			speed -= fw.Length() * Clamp((math::Abs(speed) / maxSpeed), 0.2f, 9999.0f) * (WallsBounciness + mods.bonusWallBounciness);
			if (speed > 0 && fw.LengthSq() > side.LengthSq()) { speed = 0; }
		}

		if (drifting != drift_none)
		{
			collisionwWhileDrifting++;
			driftCollisionTimer = 0.0f;
		}
	}
}

void ComponentCar::WallHit(const float3 & normal)
{
	WallHit(normal, kartZ, kartX);
}

void ComponentCar::SetPos(float3 pos)
{
	setpos = pos;
	wantToSetPos = true;
}

void ComponentCar::SetRot(Quat rot)
{
	setrot = rot;
	wantToSetRot = true;
}

void ComponentCar::SetScale(float3 scale)
{
	setscale = scale;
	wantToSetScale = true;
}

ComponentCar::ComponentCar(GameObject* GO) : Component(C_CAR, GO)
{
	SetCarType(T_KOJI);

	collShape.size = float3(1.8f, 1.6f, 3.1f);
	reset_pos = { 0.0f, 0.0f, 0.0f };
	reset_rot = { 1.0f, 1.0f, 1.0f, 1.0f };

	//Player config
	front_player = PLAYER_1;
	back_player = PLAYER_2;

	inverted_controls = false;
	invert_value = 1;

	kartTransform = ((ComponentTransform*)(game_object->GetComponent(ComponentType::C_TRANSFORM)))->GetGlobalMatrix();
}

ComponentCar::~ComponentCar()
{

}

void ComponentCar::Update()
{
	BROFILER_CATEGORY("ComponentCar::Update", Profiler::Color::GhostWhite)

		if (kart_trs)
		{
			kart_trs->Set(kartTransform);
			kart_trs->UpdateMatrix();

			float4x4 tmp = kart_trs->GetGlobalMatrix();
			tmp = tmp * float4x4::Translate(collOffset);
			tmp.Transpose();
			if (collider != nullptr)
			{
				collider->SetTransform(tmp.ptr());
			}
			collShape.transform = tmp;
			if (App->StartInGame() == false && App->physics->renderColliders)
			{
				collShape.Render();
			}
		}
		else
		{
			kart_trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
		}

	if (App->IsGameRunning())
	{
		DebugInput();

		if (wantToReset == true)
		{
			TrueReset();
			wantToReset = false;
		}

		if (wantToSetPos)
		{
			wantToSetPos = false;
			kart_trs->SetPosition(setpos);
		}
		if (wantToSetRot)
		{
			wantToSetRot = false;
			kart_trs->SetRotation(setrot);
		}
		if (wantToSetScale)
		{
			wantToSetScale = false;
			kart_trs->SetScale(setscale);
		}
		float3 scale = kart_trs->GetScale();
		if (scale.x > 1.2f || scale.y > 1.2f || scale.z > 1.2f)
		{
			SetScale(float3(1, 1, 1));
		}
		if (hitWallTimer < 10.0f)
		{
			hitWallTimer += time->DeltaTime();
		}

		turbo_mods = turbo.UpdateTurbo(time->DeltaTime());
		KartLogic();
		CastShadowRay();
		CheckGroundCollision();
		UpdateAnims();
		UpdateRenderTransform();
	}
}

void ComponentCar::KartLogic()
{
	float3 pos = kart_trs->GetPosition();
	float3 newPos = pos;
	kartX = kart_trs->GetGlobalMatrix().WorldX().Normalized();
	kartY = kart_trs->GetGlobalMatrix().WorldY().Normalized();
	kartZ = kart_trs->GetGlobalMatrix().WorldZ().Normalized();

	bool checkOffTrack = false;
	onTheGround = false;
	//Setting the "desired up" value, taking in account if both rays are close enough to the ground or none of them
	float3 desiredUp = float3(0, 0, 0);
	float offset = 0.0f;
	ITERATE_WHEELS
	{
		//Casting the weel ray and updating it's values
		it->Cast();

		if (it->hit && it->distance < DISTANCE_FROM_GROUND + 1.5f + offset && it->angleFromY < 45 * DEGTORAD)
		{
			offset = 0.5f;
			onTheGround = true;
			desiredUp += it->hitNormal;
		}
		else
		{
			checkOffTrack = true;
		}
	}

		//WE use CheckOffTrack 'cause if it's false it means that all wheels hit
		if (checkOffTrack == false && onTheGround && fallSpeed < 1.5f)
		{
			newPos.y = shadowRayResult.point.y;
			/*newPos.y = 0.0f;
			ITERATE_WHEELS
			{
				newPos.y += it->hitPoint.y;
			}
			newPos.y /= wheels.size();*/
		}

	//We use on the ground, 'cause it's true if at least one of the
	if (onTheGround == false)
	{
		desiredUp = float3(0, 1, 0) + kartZ * 0.4f;
	}
	desiredUp.Normalize();

	if (checkOffTrack && onTheGround)
	{
		CheckOnTheGround();
	}

	RotateKart(desiredUp);

	PlayersInput();


	if (steering == false)
	{
		//Returning steer to 0 gradually if the player isn't inputting anything
		AutoSteer();
	}
	steering = false;

	if (onTheGround && fallSpeed < 1.5f)
	{
		SteerKart();
		fallSpeed = 0.0f;
	}
	else
	{
		//Falling. Magic. Gravity. So much wow.
		fallSpeed -= CAR_GRAVITY * time->DeltaTime();
		newPos.y += fallSpeed * time->DeltaTime();
	}


	//Checking if one of the rays was cast onto a wall and, hence, needs to collide
	if (onTheGround)
	{
		ITERATE_WHEELS
		{
			if ((it->angleFromY > 45.0f * DEGTORAD && (it->hitNormal.y < 0.65f && it->hitNormal.y > -0.65f) && it->distance < DISTANCE_FROM_GROUND + 0.25f))
			{
				WallHit(it->hitNormal, kartZ, kartX);
			}
		}
	}
	else
	{
		math::Ray frontRay;
		frontRay.dir = kartZ;

		frontRay.pos = kart_trs->GetPosition();

		RaycastHit frontHit;
		bool hit = App->physics->RayCast(frontRay, frontHit);
		if (hit && frontHit.distance < 1.5f)
		{
			WallHit(frontHit.normal, kartZ, kartX);
		}
	}

	float3 speedDir = kartZ;
	//And finally, we move the kart!
	if (onTheGround == false)
	{
		speedDir.y = max(speedDir.y, 0.0f);
	}
	newPos += speedDir * speed;

	newPos += kartX * horizontalSpeed;
	kart_trs->SetPosition(newPos);


	//Safety mesure. Resetting the kart if it falls off the world. jej
	if (kart_trs->GetPosition().y < loose_height)
	{
		Reset();
	}
}

float ComponentCar::AccelerationInput()
{
	float acceleration = 0.0f;
	float rTrigger = (App->input->GetJoystickAxis(front_player, JOY_AXIS::RIGHT_TRIGGER) + 1.0f) / 2.0f;
	if (rTrigger == 0.5f) { rTrigger = 0.0f; }
	float lTrigger = (App->input->GetJoystickAxis(front_player, JOY_AXIS::LEFT_TRIGGER) + 1.0f) / 2.0f;
	if (lTrigger == 0.5f) { lTrigger = 0.0f; }

	//Accelerating
	if (!lock_input && ((App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT && front_player == PLAYER_1) || rTrigger > 0.2f))
	{
		if (rTrigger < 0.1f) { rTrigger = 1.0f; }
		//We recieved the order to move forward

		// Pushing (always, when going forward and backwards)
		if (!lock_input && ((App->input->GetJoystickButton(back_player, JOY_BUTTON::A) == KEY_DOWN && drifting == drift_none) || (App->input->GetKey(SDL_SCANCODE_G) == KEY_DOWN && front_player == PLAYER::PLAYER_1)))
		{
			// Pushing is applied when speed / maxSpeed is below push_threshold (60%-70%)
			if (speed / maxSpeed < push_threshold)
			{
				//LOG("Max speed %f", maxSpeed);
				push_timer.Start();
				pushing = true;
			}
		}

		if (pushing)
		{
			if (push_timer.ReadSec() < time_between_pushes)
			{
				if (push_force < max_push_force)
					push_force *= push_incr;

				acceleration += push_force;
				//LOG("Applying Force! %f", acceleration);
			}
			else
			{
				pushing = false;
				push_force = 0.001f;
				//LOG("Stopping Force!");
			}
		}

		if (speed < -0.01f)
		{
			//If we're going backwards, we apply the brakePower instead of the acceleration
			if (speed < -(brakePower + mods.bonusBrakePower) * time->DeltaTime())
			{
				acceleration += ((brakePower + mods.bonusBrakePower) + turbo_mods.accelerationBonus) * time->DeltaTime() * rTrigger;
			}
			else
			{
				speed = 0.0f;
			}
		}
		else
		{
			acceleration += (maxAcceleration + turbo_mods.accelerationBonus + mods.bonusMaxAcceleration) * time->DeltaTime() * rTrigger;
		}
	}
	//Braking
	else if (!lock_input && ((App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT && front_player == PLAYER_1) || lTrigger > 0.2f))
	{
		if (lTrigger < 0.1f) { lTrigger = 1.0f; }
		if (speed > 0.01f)
		{
			if (speed > (brakePower + mods.bonusBrakePower) * time->DeltaTime())
			{
				acceleration -= ((brakePower + mods.bonusBrakePower) + turbo_mods.accelerationBonus) * time->DeltaTime() * lTrigger;
			}
			else
			{
				speed = 0.0f;
			}
		}
		else
		{
			//We divide the result by 4, this way the kart accelerates way slower when going backwards
			acceleration -= ((maxAcceleration + turbo_mods.accelerationBonus + mods.bonusMaxAcceleration) / 2.0f) * time->DeltaTime() * lTrigger;
		}
	}
	//If there's no input, drag force slows car down
	else
	{
		if (speed > (drag + mods.bonusDrag) * time->DeltaTime())
		{
			acceleration = -(drag + mods.bonusDrag) * time->DeltaTime();
		}
		else if (speed < -(drag + mods.bonusDrag) * time->DeltaTime())
		{
			acceleration += (drag + mods.bonusDrag) * time->DeltaTime();
		}
		else
		{
			speed = 0.0f;
		}
	}	

	if (turbo_mods.alive == true && acceleration < turbo_mods.accelerationMin)
	{
		acceleration = turbo_mods.accelerationMin;
	}

	return acceleration;
}

void ComponentCar::Steer(float amount)
{
	amount = math::Clamp(amount, -1.0f, 1.0f);
	if (drifting == drift_none || drifting == drift_failed)
	{
		current_driftingFixedTurn = 0.0f;

		if (amount < -0.1 || amount > 0.1)
		{
			currentSteer += (maneuverability + mods.bonusManeuverability) * time->DeltaTime() * amount;
			amount = math::Abs(amount);
			currentSteer = math::Clamp(currentSteer, -amount, amount);
			steering = true;
		}
	}
	else
	{
		steering = true;
		Drift(amount);
	}
}

void ComponentCar::AutoSteer()
{
	//Whenever the player isn't steering, bring slowly the wheels back to the neutral position
	if (currentSteer > (maneuverability + mods.bonusManeuverability) * time->DeltaTime())
	{
		currentSteer -= (maneuverability + mods.bonusManeuverability) * time->DeltaTime();
	}
	else if (currentSteer < -(maneuverability + mods.bonusManeuverability) * time->DeltaTime())
	{
		currentSteer += (maneuverability + mods.bonusManeuverability) * time->DeltaTime();
	}
	else { currentSteer = 0; }
}

void ComponentCar::CheckOnTheGround()
{
	//Checking if the kart is still on the track
	//This ray shoots from the center of the kart, vertically down. It checks if the kart is still on the track or is about to fall off and should simulate a collision
	if (shadowRayHit && shadowRayResult.distance > DISTANCE_FROM_GROUND + 0.2f)
	{
		onTheGround = false;
	}
}

void ComponentCar::Drift(float dir)
{
	if (drifting == drift_none || drifting == drift_failed)
	{
		return;
	}

	if (current_driftingFixedTurn <= driftingFixedTurn)
	{
		current_driftingFixedTurn += 1.0f * time->DeltaTime();
	}
	else
	{
		current_driftingFixedTurn = driftingFixedTurn;
	}

	driftButtonMasher.Update(JOY_BUTTON::A, back_player);

	if (driftButtonMasher.GetNTaps() > driftPhaseChange || App->input->GetKey(SDL_SCANCODE_KP_2) == KEY_DOWN)
	{
		driftButtonMasher.Reset();

		switch (drifting)
		{
		case drift_right_0:
			drifting = drift_right_1; break;
		case drift_right_1:
			drifting = drift_right_2; break;
		case drift_left_0:
			drifting = drift_left_1; break;
		case drift_left_1:
			drifting = drift_left_2; break;
		}
	}

	float desiredCurrentSteer = 0.0f;

	if (drifting == drift_left_0 || drifting == drift_left_1 || drifting == drift_left_2)
	{
		desiredCurrentSteer = -current_driftingFixedTurn + (dir * driftingAdjustableTurn);
		dir += 0.6f;
	}
	else
	{
		desiredCurrentSteer = current_driftingFixedTurn + (dir * driftingAdjustableTurn);
		dir -= 0.6f;
	}
	dir *= 2.0f;

	float steerChange = steerChangeSpeed * time->DeltaTime();

	if (currentSteer < desiredCurrentSteer)
	{
		currentSteer += steerChange;
	}
	else
	{
		currentSteer -= steerChange;
	}

	horizontalSpeed += horizontalSpeedDrifting * time->DeltaTime() * -dir;
}

void ComponentCar::DriftManagement()
{
	if (drifting == drift_failed && (App->input->GetKey(SDL_SCANCODE_SPACE) != KEY_REPEAT && App->input->GetJoystickButton(front_player, JOY_BUTTON::X) != KEY_REPEAT))
	{
		drifting = drift_none;
		if (audio_source) audio_source->PlayAudio(7);	// Reduce Drifting
	}

	if (drifting != drift_failed && (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT || App->input->GetJoystickButton(front_player, JOY_BUTTON::X) == KEY_REPEAT))
	{
		//Checking we have enough speed to drift
		if (speed > maxSpeed * minimumSpeedRatioToStartDrifting && driftAirTimer < 0.65f)
		{
			if (onTheGround == false)
			{
				driftAirTimer += time->DeltaTime();
			}

			//If we weren't drifting, we enter the state of drift
			if (drifting == drift_none && onTheGround)
			{
				if (currentSteer > minimumSteerToStartDrifting)
				{
					fb_jumpSpeed = driftJumpSpeed;
					drifting = drift_right_0;
					driftButtonMasher.Reset();
					if (audio_source) audio_source->PlayAudio(6);	// Init Drifting
				}
				else if (currentSteer < -minimumSteerToStartDrifting)
				{
					fb_jumpSpeed = driftJumpSpeed;
					drifting = drift_left_0;
					driftButtonMasher.Reset();
					if (audio_source) audio_source->PlayAudio(6);	// Init Drifting
				}
			}
		}
		else if (lastFrame_drifting != drift_none && drifting != drift_none)
		{			
			//If we don't have enough speed, the drift is considered a failure
			drifting = drift_failed;
			if (audio_source) audio_source->PlayAudio(8); // Stopping Drifting
		}
		else
		{
			//if (audio_source) audio_source->PlayAudio(7);	// Reduce Drifting
			drifting = drift_none;
		}
	}
	else if(drifting != drift_failed)
	{
		//if (audio_source) audio_source->PlayAudio(7);	// Reduce Drifting
		drifting = drift_none;
	}

	if (onTheGround == true)
	{
		driftAirTimer = 0.0f;
	}

	if (collisionwWhileDrifting >= collisionsUntilStopDrifting)
	{
		drifting = drift_failed;
		collisionwWhileDrifting = 0;
		if (audio_source) audio_source->PlayAudio(8); // Stopping Drifting
	}

	if (collisionwWhileDrifting > 0)
	{
		driftCollisionTimer += time->DeltaTime();
		if (driftCollisionTimer > driftCollisionRecovery)
		{
			collisionwWhileDrifting = 0;
		}
	}
	else
	{
		driftCollisionTimer = 0.0f;
	}

	//Avoid drifting from stopping when the kart is jumping in the air
	if (fb_vertical > 0.01f && (drifting == drift_none || drifting == drift_failed))
	{
		drifting = lastFrame_drifting;
	}

	//When we exit a drift, apply the correspondant turbo
	if (drifting == drift_none && lastFrame_drifting != drift_none && lastFrame_drifting != drift_failed)
	{
		switch (lastFrame_drifting)
		{
		case drift_right_1:
			NewTurbo(turboPicker.drift1);
			break;
		case drift_left_1:
			NewTurbo(turboPicker.drift1);
			break;
		case drift_right_2:
			NewTurbo(turboPicker.drift2);
			break;
		case drift_left_2:
			NewTurbo(turboPicker.drift2);
			break;
		}
		if (audio_source) audio_source->PlayAudio(7);	// Reduce Drifting
		drifting = drift_none;
	}
	lastFrame_drifting = drifting;
}

void ComponentCar::PlayersInput()
{
	//Horizontally, the kart will always try to slow down, no matter the inputs
	HorizontalDrag();

	//Manage Input to accelerate/brake. Returns acceleration
	if (onTheGround)
	{
		speed += AccelerationInput();
		speed = math::Clamp(speed, (-maxSpeed - turbo_mods.maxSpeedBonus - mods.bonusMaxSpeed) / 3.0f, maxSpeed + turbo_mods.maxSpeedBonus + mods.bonusMaxSpeed);
	}
	horizontalSpeed = math::Clamp(horizontalSpeed, -maxSpeed, maxSpeed);

	//Steering
	if (lock_input == false)
	{
		float steerAmount = App->input->GetJoystickAxis(front_player, JOY_AXIS::LEFT_STICK_X);
		if (front_player == PLAYER_1)
		{
			if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) { steerAmount += 1; }
			if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) { steerAmount -= 1; }
		}
		Steer(steerAmount);
	}

	DriftManagement();

	//Acrobatics
	if (onTheGround == false && acrobaticsDone == false)
	{
		KEY_STATE state = App->input->GetJoystickButton(front_player, JOY_BUTTON::X);
		if (state != KEY_DOWN || state != KEY_REPEAT) { state = App->input->GetKey(SDL_SCANCODE_KP_1); }
		if (state == KEY_DOWN || state == KEY_REPEAT)
		{
			if (p2AcrobaticsPress.ReadSec() < acrobaticsDelay)
			{
				acrobaticsDone = true;
			}
			else
			{
				p1AcrobaticsPress.Start();
			}
		}
		state = App->input->GetJoystickButton(back_player, JOY_BUTTON::X);
		if (state != KEY_DOWN || state != KEY_REPEAT) { state = App->input->GetKey(SDL_SCANCODE_KP_2); }
		if (state == KEY_DOWN || state == KEY_REPEAT)
		{
			if (p1AcrobaticsPress.ReadSec() < acrobaticsDelay)
			{
				acrobaticsDone = true;
			}
			else
			{
				p2AcrobaticsPress.Start();
			}
		}
	}
}

void ComponentCar::SteerKart()
{
	//This simply translataes current steer into a Quaterion we can rotate the kart with
	//currentSteer goes from -1 to 1, and we multiply it by the "Max Steer" value.

	float steerReduction = 1.0f;
	if (speed < maxSpeed * 0.75f)
	{
		//The "Clamp" thing is to allow less rotation the less the kart is moving
		//The kart can rotate the maximum amount when it goes faster than maxSpeed / 2
		steerReduction = math::Clamp(math::Abs(speed) / (maxSpeed / 2), 0.0f, 1.0f);
	}
	else
	{
		float diference = speed - maxSpeed * 0.75f;
		steerReduction = 1 - (diference / (maxSpeed*0.75f));
		steerReduction = Clamp(steerReduction, maxSteerReduction, 1.0f);
	}

	if (speed < 0.0f)
	{
		steerReduction *= -1;
	}
	float rotateAngle = (maxSteer + mods.bonusMaxSteering) * steerReduction * currentSteer * time->DeltaTime();
	Quat tmp = kart_trs->GetRotation().RotateAxisAngle(kartY, -(rotateAngle* invert_value) * DEGTORAD);
	kart_trs->Rotate(tmp);

	if (kart_front_wheel_l != nullptr)
	{
		float3 rotation = kart_front_wheel_l->GetRotationEuler();
		//God damn it somebody kill me for this please
		if ((*kart_front_wheel_l->GetGameObject()->name.begin()) == 'I')
			rotation.x -= (3000 * speed * time->DeltaTime());
		else
			rotation.x += (3000 * speed * time->DeltaTime());
		kart_front_wheel_l->SetRotation(rotation);

		if (parent_front_l == nullptr)
		{
			parent_front_l = (ComponentTransform*)(kart_front_wheel_l->GetGameObject()->GetParent()->GetComponent(C_TRANSFORM));
		}

		float3 parent_rotation = parent_front_l->GetRotationEuler();
		parent_rotation.y = -40 * currentSteer;
		parent_front_l->SetRotation(parent_rotation);
	}
	if (kart_front_wheel_r != nullptr)
	{
		float3 rotation = kart_front_wheel_r->GetRotationEuler();
		//God damn it somebody kill me for this please
		if ((*kart_front_wheel_r->GetGameObject()->name.begin()) == 'I')
			rotation.x -= (3000 * speed * time->DeltaTime());
		else
			rotation.x += (3000 * speed * time->DeltaTime());
		kart_front_wheel_r->SetRotation(rotation);

		if (parent_front_r == nullptr)
		{
			parent_front_r = (ComponentTransform*)kart_front_wheel_r->GetGameObject()->GetParent()->GetComponent(C_TRANSFORM);
		}

		float3 parent_rotation = parent_front_r->GetRotationEuler();
		parent_rotation.y = -40 * currentSteer;
		parent_front_r->SetRotation(parent_rotation);
	}
	if (kart_back_wheel_l != nullptr)
	{
		float3 rotation = kart_back_wheel_l->GetRotationEuler();
		rotation.x += (3000 * speed * time->DeltaTime());
		kart_back_wheel_l->SetRotation(rotation);
	}
	if (kart_back_wheel_r != nullptr)
	{
		float3 rotation = kart_back_wheel_r->GetRotationEuler();
		rotation.x += (3000 * speed * time->DeltaTime());
		kart_back_wheel_r->SetRotation(rotation);
	}
}

void ComponentCar::RotateKart(float3 desiredUp)
{
	desiredUp.Normalize();

	//If the kart's Y is different from the "desired up", we slowly rotate the kart
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
			nextStep = kartY.Lerp(desiredUp, 0.01f * time->DeltaTime());
		}
		Quat normal_rot = Quat::RotateFromTo(kartY, nextStep);
		kart_trs->Rotate(normal_rot);
	}
}

void ComponentCar::HorizontalDrag()
{
	if (math::Abs(horizontalSpeed) > (drag + mods.bonusDrag) * 4.0f * time->DeltaTime())
	{
		if (horizontalSpeed > 0)
		{
			horizontalSpeed -= (drag + mods.bonusDrag) * 4.0f * time->DeltaTime();
		}
		else
		{
			horizontalSpeed += (drag + mods.bonusDrag) * 4.0f * time->DeltaTime();
		}
	}
	else
	{
		horizontalSpeed = 0.0f;
	}
}


void ComponentCar::OnPlay()
{
	float x = 0.5f;
	float z = 0.8f;
	float3 dir = float3(0.8, 1, 0.9);

	//Back left
	wheels.push_back(Wheel(this, float2(-x, -z), float3(-dir.x, -dir.y, -dir.z)));
	//Back Right
	wheels.push_back(Wheel(this, float2(x, -z), float3(dir.x, -dir.y, -dir.z)));
	//Front left
	wheels.push_back(Wheel(this, float2(-x, z), float3(-dir.x, -dir.y, dir.z)));
	//Front Right
	wheels.push_back(Wheel(this, float2(x, z), float3(dir.x, -dir.y, dir.z)));

	if (kart_trs)
	{
		reset_pos = kart_trs->GetPosition();
		reset_rot = kart_trs->GetRotation();
	}
	checkpoints = MAXUINT - 10;
	lap = 1;
	raceStarted = false;
	finished = false;
	n_checkpoints = 0;
	speed = 0.0f;
	fallSpeed = 0.0f;

	collider = App->physics->AddVehicle(collShape, this);

	lastFrame_drifting = drifting = drift_none;

	audio_source = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
}

void ComponentCar::SetFrontPlayer(PLAYER player)
{
	if (player >= 0 && player < 4)
	{
		front_player = player;
	}
}

void ComponentCar::SetBackPlayer(PLAYER player)
{
	if (player >= 0 && player < 4)
	{
		back_player = player;
	}
}

void ComponentCar::BlockInput(bool block)
{
	lock_input = block;
}

float ComponentCar::GetVelocity() const
{
	return speed * UNITS_TO_KMH;
}

// CONTROLS-----------------------------

void ComponentCar::PickItem()
{
	has_item = true;
}

void ComponentCar::UseItem()
{
	SetP2AnimationState(P2USE_ITEM, 0.5f);

	if (has_item)
	{
		turbo.SetTurbo(turboPicker.rocket);
		turbo.TurnOn();
		has_item = false;
	}
}

bool ComponentCar::GetInvertStatus() const
{
	return inverted_controls;
}

void ComponentCar::SetInvertStatus(bool status)
{
	if (status != inverted_controls)
	{
		inverted_controls = status;
		if (status)
		{
			invert_value = -1;
		}
		else
		{
			invert_value = 1;
		}
	}
}

bool ComponentCar::AddHitodama()
{
	if (num_hitodamas < max_hitodamas)
	{
		num_hitodamas++;
		maxSpeed += 0.01f;
		return true;
	}
	return false;
}

bool ComponentCar::RemoveHitodama()
{
	if (num_hitodamas > 0)
	{
		num_hitodamas--;
		maxSpeed -= 0.01f;
		return true;
	}
	return false;
}

int ComponentCar::GetNumHitodamas() const
{
	return num_hitodamas;
}

void ComponentCar::NewTurbo(Turbo turboToApply)
{
	turbo.SetTurbo(turboToApply);
	turbo.TurnOn();
}

void ComponentCar::TurboPad()
{
	if(inverted_controls == false)
		NewTurbo(turboPicker.turboPad);
	else
		NewTurbo(turboPicker.turboPadOnEvil);	
}

DRIFT_STATE ComponentCar::GetDriftState() const
{
	return drifting;
}

void ComponentCar::SetP2AnimationState(Player2_State state, float blend_ratio)
{
	if (p2_animation != nullptr)
	{
		switch (state)
		{
		case (P2IDLE):
		{
			p2_state = state;
			p2_animation->PlayAnimation(3, blend_ratio);
			break;
		}
		case(P2DRIFT_LEFT):
		{
			if (p2_state != P2DRIFT_LEFT)
			{
				p2_state = state;
				p2_animation->PlayAnimation(2, blend_ratio);
			}
			break;
		}
		case(P2DRIFT_RIGHT):
		{
			if (p2_state != P2DRIFT_RIGHT)
			{
				p2_state = state;
				p2_animation->PlayAnimation(1, blend_ratio);
			}
			break;
		}
		case(P2PUSH_START):
		{
			p2_state = state;
			p2_animation->PlayAnimation(4, blend_ratio);
			break;
		}
		case(P2PUSH_LOOP):
		{
			p2_state = state;
			p2_animation->PlayAnimation(5, blend_ratio);
			break;
		}
		case(P2PUSH_END):
		{
			p2_state = state;
			p2_animation->PlayAnimation(6, blend_ratio);
			break;
		}
		case (P2GET_HIT):
		{
			p2_state = state;
			p2_animation->PlayAnimation(8, blend_ratio);
			break;
		}
		case(P2USE_ITEM):
		{
			p2_state = state;
			p2_animation->PlayAnimation(9, blend_ratio);
			break;
		}
		case(P2ACROBATICS):
		{
			p2_state = state;
			p2_animation->PlayAnimation(10, blend_ratio);
			break;
		}
		}
	}
}

void ComponentCar::UpdateP1Animation()
{
	if (p1_animation != nullptr)
	{
		switch (p1_state)
		{
		case(P1ACROBATICS):
		{
			if (p1_animation->playing == false)
			{
				p1_state = P1IDLE;
			}
			break;
		}
		case(P1GET_HIT):
		{
			if (p1_animation->playing == false)
			{
				p1_state = P1IDLE;
			}
			break;
		}
		case(P1MAXTURN_L):
		{
			if (steering > -0.85f)
				p1_state = P1IDLE;
			break;
		}
		case (P1MAXTURN_R):
		{
			if (currentSteer < 0.85f)
				p1_state = P1IDLE;
			break;
		}
		case(P1TURN):
		{
			float ratio = (currentSteer + 1.0f) / 2.0f;
			p1_animation->LockAnimationRatio(ratio);
			if (currentSteer >= 9.0f || currentSteer <= -0.9f) p1_state = P1IDLE;
			break;
		}
		case(P1IDLE):
		{
			if (currentSteer <= -0.9f)
			{
				p1_state = P1MAXTURN_L;
				p1_animation->PlayAnimation(1, 0.5f);
			}
			else if (currentSteer >= 0.9f)
			{
				p1_state = P1MAXTURN_R;
				p1_animation->PlayAnimation(2, 0.5f);
			}
			else
			{
				p1_state = P1TURN;
				p1_animation->PlayAnimation((uint)0, 0.5f);
				float ratio = (currentSteer + 1.0f) / 2.0f;
				p1_animation->LockAnimationRatio(ratio);
			}
			break;
		}
		}
	}
}

void ComponentCar::UpdateP2Animation()
{
	if (p2_animation != nullptr)
	{
		switch (p2_state)
		{
		case(P2IDLE):
		{
			if (drifting != drift_none && drifting != drift_failed)
			{
				switch (drifting)
				{
				case drift_right_0:
				case drift_right_1:
				case drift_right_2:
					SetP2AnimationState(P2DRIFT_RIGHT, 0.5f);
					break;
				case drift_left_0:
				case drift_left_1:
				case drift_left_2:
					SetP2AnimationState(P2DRIFT_LEFT, 0.5f);
					break;
				}
			}
			else if (pushing == true)
			{
				SetP2AnimationState(P2PUSH_START, 0.5f);
			}
			else
			{
				if (p2_animation->current_animation->index != 3) SetP2AnimationState(P2IDLE, 0.5f);
				p2_animation->current_animation->ticks_per_second = 8.0f + 24.0f * Clamp((speed / maxSpeed), 0.3f, 1.5f);
			}
			break;
		}
		case(P2PUSH_START):
		{
			if (p2_animation->playing == false && pushing == true)
			{
				SetP2AnimationState(P2PUSH_LOOP, 0.5f);
			}
			else if (p2_animation->playing == false)
			{
				SetP2AnimationState(P2PUSH_END, 0.5f);
			}
			break;
		}
		case(P2PUSH_LOOP):
		{
			if (pushing == false)
			{
				SetP2AnimationState(P2PUSH_END, 0.5f);
			}
			break;
		}
		case (P2PUSH_END):
		{
			if (p2_animation->playing == false)
			{
				SetP2AnimationState(P2IDLE, 0.5f);
			}
			break;
		}
		case(P2DRIFT_LEFT):
		case(P2DRIFT_RIGHT):
		{
			if (drifting == drift_none)
			{
				SetP2AnimationState(P2IDLE, 0.5f);
			}
			else if (drifting == drift_failed)
			{
				SetP2AnimationState(P2GET_HIT, 0.5f);
			}
			break;
		}
		case(P2GET_HIT):
		{
			if (p2_animation->playing == false)
			{
				SetP2AnimationState(P2IDLE, 0.5f);
			}
			break;
		}
		case(P2ACROBATICS):
		{
			if (p2_animation->playing == false)
			{
				SetP2AnimationState(P2IDLE, 0.5f);
			}
			break;
		}
		case(P2USE_ITEM):
		{
			if (p2_animation->playing == false)
			{
				SetP2AnimationState(P2IDLE, 0.5f);
			}
			break;
		}
		}
	}
}

void ComponentCar::OnGetHit(float velocity_reduction)
{
	speed *= (1.0f - velocity_reduction);
	horizontalSpeed *= (1.0f - velocity_reduction);
	fallSpeed = 7.0f;

	if (p2_animation != nullptr)
	{
		SetP2AnimationState(P2GET_HIT, 0.5f);
	}
	if (p1_animation != nullptr)
	{
		p1_state = P1GET_HIT;
		p1_animation->PlayAnimation(3, 0.5f);
	}
}

void ComponentCar::WentThroughCheckpoint(int checkpoint, float3 resetPos, Quat resetRot)
{
	bool lastWrongDirection = wrongDirection;
	wrongDirection = false;
	if (checkpoint == checkpoints + 1)
	{
		n_checkpoints++;
		last_check_pos = resetPos;
		last_check_rot = resetRot;
		checkpoints = checkpoint;
	}
	else
	{
		if (checkpoint != lastCheckpoint)
		{
			last2Checkpoint = lastCheckpoint;
			lastCheckpoint = checkpoint;
		}
		if (wentThroughEnd)
		{
			if (lastWrongDirection)
			{
				lastCheckpoint = checkpoint;
				last2Checkpoint = lastCheckpoint + 1;
			}
			else
			{
				last2Checkpoint = lastCheckpoint = checkpoint;
			}
			wentThroughEnd = false;
		}

		if(lastCheckpoint < last2Checkpoint)
		{
			wrongDirection = true;
		}
	}
	
}

void ComponentCar::WentThroughEnd(int checkpoint, float3 resetPos, Quat resetRot)
{
	if (checkpoints + 1 >= checkpoint)
	{
		if (raceStarted == false)
		{
			raceStarted = true;
		}
		else
		{
			lap++;
		}
		n_checkpoints++;
		checkpoints = 0;
		last_check_pos = resetPos;
		last_check_rot = resetRot;

	}
	wentThroughEnd = true;

	if (lap >= 4)
	{
		finished = true;
		BlockInput(true);
		lap = 4;
	}
}
//--------------------------------------

void ComponentCar::Reset()
{
	wantToReset = true;
}

void ComponentCar::TrueReset()
{
	if (checkpoints >= MAXUINT - 20)
	{
		kart_trs->SetRotation(reset_rot);
		kart_trs->SetPosition(reset_pos + float3(kartX * front_player * 0.5f));
	}
	else
	{
		kart_trs->SetRotation(last_check_rot);
		kart_trs->SetPosition(last_check_pos + float3(kartX * front_player * 0.5f));
	}
	wrongDirection = false;
	acrobaticsDone = false;
	currentSteer = 0;
	OnGetHit(1.0f);
	lastFrame_drifting = drifting = drift_none;
}

float ComponentCar::GetVelocity()
{
	return speed * UNITS_TO_KMH;
}

float ComponentCar::GetMaxVelocity() const
{
	return (maxSpeed + turbo_mods.maxSpeedBonus + mods.bonusMaxSpeed) * UNITS_TO_KMH;
}

const float *ComponentCar::GetSpeed() const
{
	return &speed;
}

unsigned int ComponentCar::GetFrontPlayer()
{
	return front_player;
}

unsigned int ComponentCar::GetBackPlayer()
{
	return back_player;
}

bool ComponentCar::GetGroundState() const
{
	return onTheGround;
}

float ComponentCar::GetAngularVelocity() const
{
	float tmp = currentSteer;
	if (tmp > 1.0f) { tmp = 1.0f; }
	else if (tmp < -1.0f) { tmp = -1.0f; }

	return tmp * (maxSteer + mods.bonusMaxSteering) * DEGTORAD;
}

Turbo ComponentCar::GetAppliedTurbo() const
{
	return turbo;
}

void ComponentCar::SetCarType(CAR_TYPE type)
{
	//TODO: change values depending on the kart type
	/*switch (type)
	{
	case T_KOJI:
	kart = &koji;
	kart->type = T_KOJI;
	break;
	case T_WOOD:
	kart = &wood;
	kart->type = T_WOOD;
	break;
	}*/
}

void ComponentCar::DebugInput()
{
	if (App->input->GetKey(SDL_SCANCODE_X) == KEY_DOWN)
	{
		NewTurbo(turboPicker.acrobatic);
	}
	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
	{
		Reset();
	}
	//Reset button
	if (resetTimer > 2.0f)
	{
		if (App->input->GetJoystickButton(front_player, JOY_BUTTON::SELECT) == KEY_REPEAT && App->input->GetJoystickButton(front_player, JOY_BUTTON::START) == KEY_REPEAT)
		{
			Reset();
			resetTimer = 0.0f;
		}
	}
	else
	{
		resetTimer += time->DeltaTime();
	}
	if (App->input->GetKey(SDL_SCANCODE_C) == KEY_DOWN)
	{
		OnGetHit(1.0f);
	}
}

void ComponentCar::CheckGroundCollision()
{
	BROFILER_CATEGORY("ComponentCar::CheckGroundCollision", Profiler::Color::HoneyDew)
		if (lastFrameOnGround != onTheGround)
		{
			if (onTheGround)
			{
				OnGroundCollision(G_BEGIN);
			}
			else
			{
				OnGroundCollision(G_EXIT);
			}
			lastFrameOnGround = onTheGround;
		}
	//We don't need repeat nor none for now
}

void ComponentCar::OnGroundCollision(GROUND_CONTACT state)
{
	if (state == G_EXIT)
	{
		//Changes when exits ground contact
		if (fallSpeed < 1.5f)
		{
			float3 realSpeed = kart_trs->GetGlobalMatrix().WorldZ().Normalized() * speed;
			fallSpeed = realSpeed.y;
		}
	}
	else if (state == G_BEGIN)
	{
		if (acrobaticsDone)
		{
			acrobaticsDone = false;
			turbo.SetTurbo(turboPicker.acrobatic);
		}
	}
}

void ComponentCar::OnTransformModified()
{}

void ComponentCar::CastShadowRay()
{
	math::Ray rayN;
	rayN.dir = float3(0, -1, 0);
	rayN.pos = kart_trs->GetPosition() + float3(0, 1, 0);
	shadowRayHit = App->physics->RayCast(rayN, shadowRayResult);

}

void ComponentCar::UpdateAnims()
{
	BROFILER_CATEGORY("ComponentCar::UpdateGO", Profiler::Color::HoneyDew)

		//Updating turn animation

		//Player 1 animation
		if (p1_animation != nullptr)
		{
			UpdateP1Animation();
		}

	//Player 2 animation
	if (p2_animation != nullptr)
	{
		UpdateP2Animation();
	}

}

void ComponentCar::UpdateRenderTransform()
{
	kart_trs->UpdateMatrix();
	kartTransform = kart_trs->GetGlobalMatrix();

	float desired_Y, desired_FW, up_correction;

	//Setting "ideal" desired values for each rotation and translation

	//Vertical displacement
	if (fb_vertical > 0.01f || fb_jumpSpeed > 0.01f)
	{
		fb_jumpSpeed -= CAR_GRAVITY * driftJumpGravityMultiplier * time->DeltaTime();
		fb_vertical += fb_jumpSpeed * time->DeltaTime();
	}
	else
	{
		fb_vertical = 0.0f;
	}
	
	switch (drifting)
	{
	case drift_right_0:
		desired_Y = -driftVisualTurn;
		desired_FW = -7.0f;
		up_correction = 0.25f;
		break;
	case drift_right_1:
		desired_Y = -driftVisualTurn;
		desired_FW = -10.0f;
		up_correction = 0.5f;
		break;
	case drift_right_2:
		desired_Y = -driftVisualTurn;
		desired_FW = -15.0f;
		up_correction = 0.7f;
		break;
	case drift_left_0:
		desired_Y = driftVisualTurn;
		desired_FW = 7.0f;
		up_correction = 0.25f;
		break;
	case drift_left_1:
		desired_Y = driftVisualTurn;
		desired_FW = 10.0f;
		up_correction += 0.5f;
		break;
	case drift_left_2:
		desired_Y = driftVisualTurn;
		desired_FW = 15.0f;
		up_correction = 0.7f;
		break;
	default:
		desired_Y = 0.0f;
		desired_FW = 0.0f;
		up_correction = 0.0f;
		break;
	}

	//Making the kart "Wobble" when it's not flat on the ground
	if (desired_FW != 0)
	{
		desired_FW += math::Cos(time->RealTimeSinceStartup() * 720.0f * DEGTORAD) * 2.0f;
	}

	//Speed at which the kart goes to the desired values
	float rotSpeed = 60.0f * time->DeltaTime();
	float moveSpeed = 10.0f * time->DeltaTime();

	testVar = fb_vertical;
	//Moving current vertical position to the desired one
	if (math::Abs(fb_verticalCorrection - up_correction) < moveSpeed)
		fb_verticalCorrection = up_correction;
	else
	{
		if (fb_verticalCorrection > up_correction) { fb_verticalCorrection -= moveSpeed; }
		else { fb_verticalCorrection += moveSpeed; }
	}
	//Actually moving the kart to the position
	if (fb_vertical != 0.0f || fb_verticalCorrection != 0.0f)
	{
		kart_trs->SetPosition(kart_trs->GetPosition() + float3(0, fb_vertical + fb_verticalCorrection, 0));
	}
	else
	{
		fb_verticalCorrection = 0.0f;
		fb_jumpSpeed = 0.0f;
	}

	//Moving the rotations to their "desired" values
	if (math::Abs(fb_rotation_Y - desired_Y) < rotSpeed)
		fb_rotation_Y = desired_Y;
	else
	{
		if (fb_rotation_Y > desired_Y) { fb_rotation_Y -= rotSpeed; }
		else { fb_rotation_Y += rotSpeed; }
	}

	if (math::Abs(fb_rotation_FW - desired_FW) < rotSpeed)
		fb_rotation_FW = desired_FW;
	else
	{
		if (fb_rotation_FW > desired_FW) { fb_rotation_FW -= rotSpeed; }
		else { fb_rotation_FW += rotSpeed; }
	}

	//Actually rotating the kart
	if (math::Abs(fb_rotation_Y) > 1.0f)
		kart_trs->Rotate(Quat::RotateAxisAngle(float3(0, 1, 0), fb_rotation_Y * DEGTORAD));
	else
		fb_rotation_Y = 0.0f;

	kart_trs->UpdateMatrix();
	if (math::Abs(fb_rotation_FW) > 1.0f)
		kart_trs->Rotate(Quat::RotateAxisAngle(kart_trs->GetForward(), fb_rotation_FW * DEGTORAD));
	else
		fb_rotation_FW = 0.0f;
		
	kart_trs->UpdateMatrix();
}

void ComponentCar::Save(Data& file) const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);

	//Common on both cars

	//Physics 2.0 data

	data.AppendFloat3("collider_offset", collOffset.ptr());
	data.AppendFloat3("collider_size", collShape.size.ptr());

	data.AppendFloat("maxSpeed", maxSpeed);
	data.AppendFloat("maxAcceleration", maxAcceleration);
	data.AppendFloat("brakePower", brakePower);
	data.AppendFloat("maneuverability", maneuverability);
	data.AppendFloat("maxSteer", maxSteer);
	data.AppendFloat("drag", drag);
	data.AppendFloat("push_incr", push_incr);
	data.AppendFloat("max_push_force", max_push_force);
	data.AppendFloat("push_threshold", push_threshold);

	data.AppendFloat("WallsBounciness", WallsBounciness);

	data.AppendFloat("maxSteerReduction", maxSteerReduction);

	data.AppendInt("driftPhaseChange", driftPhaseChange);

	//Game loop settings
	//data.AppendFloat("lose_height", loose_height);

	//Drifting Values
	data.AppendFloat("driftingFixedTurn", driftingFixedTurn);
	data.AppendFloat("driftingAdjustableTurn", driftingAdjustableTurn);
	data.AppendFloat("steerChangeSpeed", steerChangeSpeed);
	data.AppendFloat("horizontalSpeedDrifting", horizontalSpeedDrifting);
	data.AppendFloat("minimumSteerToStartDrifting", minimumSteerToStartDrifting);
	data.AppendFloat("minimumSpeedRatioToStartDrifting", minimumSpeedRatioToStartDrifting);
	data.AppendFloat("driftJumpSpeed", driftJumpSpeed);
	data.AppendFloat("driftJumpGravityMultiplier", driftJumpGravityMultiplier);
	data.AppendFloat("driftVisualTurn", driftVisualTurn);

	data.AppendInt("collisionsUntilStopDrifting", collisionsUntilStopDrifting);
	data.AppendFloat("driftCollisionRecovery", driftCollisionRecovery);

	//Controls settings , Unique for each--------------

	//Hitodamas
	data.AppendInt("max_hitodamas", max_hitodamas);
	data.AppendInt("bonus_hitodamas", bonus_hitodamas);

	file.AppendArrayValue(data);
}

void ComponentCar::Load(Data& conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	//Physics 2.0 values

	collOffset = conf.GetFloat3("collider_offset");
	collShape.size = conf.GetFloat3("collider_size");

	//I know this is weird and ugly... but if we don't check the 0.0 value, the car will always be with all stats on 0 on older scenes, making them unlpayable
	float tmp;
	tmp = conf.GetFloat("maxSpeed");
	if (tmp != 0.0f) { maxSpeed = tmp; }
	tmp = conf.GetFloat("maxAcceleration");
	if (tmp != 0.0f) { maxAcceleration = tmp; }
	tmp = conf.GetFloat("brakePower");
	if (tmp != 0.0f) { brakePower = tmp; }
	tmp = conf.GetFloat("maneuverability");
	if (tmp != 0.0f) { maneuverability = tmp; }
	tmp = conf.GetFloat("maxSteer");
	if (tmp != 0.0f) { maxSteer = tmp; }
	tmp = conf.GetFloat("drag");
	if (tmp != 0.0f) { drag = tmp; }
	tmp = conf.GetFloat("push_incr");
	if (tmp != 0.0f) { push_incr = tmp; }
	tmp = conf.GetFloat("max_push_force");
	if (tmp != 0.0f) { max_push_force = tmp; }
	tmp = conf.GetFloat("push_threshold");
	if (tmp != 0.0f) { push_threshold = tmp; }
	tmp = conf.GetFloat("WallsBounciness");
	if (tmp != 0.0f) { WallsBounciness = tmp; }

	tmp = conf.GetFloat("maxSteerReduction");
	if (tmp != 0.0f) { maxSteerReduction = tmp; }

	tmp = conf.GetInt("driftPhaseChange");
	if (tmp != 0.0) { driftPhaseChange = tmp; }

	tmp = conf.GetFloat("driftingFixedTurn");
	if (tmp != 0.0f) { driftingFixedTurn = tmp; }
	tmp = conf.GetFloat("driftingAdjustableTurn");
	if (tmp != 0.0f) { driftingAdjustableTurn = tmp; }
	tmp = conf.GetFloat("steerChangeSpeed");
	if (tmp != 0.0f) { steerChangeSpeed = tmp; }
	tmp = conf.GetFloat("horizontalSpeedDrifting");
	if (tmp != 0.0f) { horizontalSpeedDrifting = tmp; }
	tmp = conf.GetFloat("minimumSteerToStartDrifting");
	if (tmp != 0.0f) { minimumSteerToStartDrifting = tmp; }
	tmp = conf.GetFloat("minimumSpeedRatioToStartDrifting");
	if (tmp != 0.0f) { minimumSpeedRatioToStartDrifting = tmp; }
	tmp = conf.GetFloat("driftJumpSpeed");
	if (tmp != 0.0f) { driftJumpSpeed = tmp; }
	tmp = conf.GetFloat("driftJumpGravityMultiplier");
	if (tmp != 0.0f) { driftJumpGravityMultiplier = tmp; }
	tmp = conf.GetFloat("driftVisualTurn");
	if (tmp != 0.0f) { driftVisualTurn = tmp; }

	tmp = conf.GetFloat("collisionsUntilStopDrifting");
	if (tmp != 0.0f) { collisionsUntilStopDrifting = tmp; }
	tmp = conf.GetFloat("driftCollisionRecovery");
	if (tmp != 0.0f) { driftCollisionRecovery = tmp; }


	//Game loop settings
	//loose_height = conf.GetFloat("lose_height");

	//Gameplay settings-----------------

	//Hitodamas
	max_hitodamas = conf.GetInt("max_hitodamas");
	bonus_hitodamas = conf.GetInt("bonus_hitodamas");

}

void ComponentCar::OnInspector(bool debug)
{
	string str = (string("Car") + string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##car");
		}

		if (ImGui::BeginPopup("delete##car"))
		{
			if (ImGui::MenuItem("Delete##car"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}
		ImGui::DragFloat("Max Speed", &maxSpeed, 0.1f, 0.1f, 20.0f);
		ImGui::DragFloat("Max Acceleration", &maxAcceleration, 0.01f, 0.01f, 20.0f);
		ImGui::DragFloat("Brake Power", &brakePower, 0.1f, 0.1f, 20.0f);
		ImGui::DragFloat("Maneuverability", &maneuverability, 0.1f, 0.1f, 20.0f);
		ImGui::DragFloat("Max Steer", &maxSteer, 1.0f, 0.0f, 300.0f);
		ImGui::DragFloat("Drag", &drag, 0.01f, 0.01f, 20.0f);
		ImGui::DragFloat("Bounciness", &WallsBounciness, 0.1f, 0.1f, 4.0f);

		ImGui::DragFloat("Push Incr", &push_incr, 1.05f, 1.0f, 2.0f);
		ImGui::DragFloat("Max Push force", &max_push_force, 0.1f, 0.1f, 4.0f);
		ImGui::DragFloat("Push threshold", &push_threshold, 0.70f, 0.1f, 1.0f);
		ImGui::Text("Maximum maneuverability reduction (percentual):");
		ImGui::InputInt("Clicks to improve turbo", &driftPhaseChange);
		ImGui::DragFloat("##maxmanReduction", &maxSteerReduction, 0.05f, 0.0f, 0.95f);
		ImGui::Separator();
		ImGui::Text("Collider:");
		if (App->IsGameRunning() == false)
		{
			ImGui::DragFloat3("Collider Size", collShape.size.ptr(), 0.1f, 0.2f, 100.0f);
		}
		ImGui::DragFloat3("Collider Offset", collOffset.ptr(), 0.1f, -50.0f, 50.0f);
		if (ImGui::TreeNode("Debug output"))
		{
			ImGui::Text(
				"Place: %i\n"
				"Speed: %f\n"
				"Horizontal speed: %f\n"
				"Vertical speed: %f\n"
				"Current Steer: %f\n"
				"Pushing: %s\n"
				"Steering: %s\n"
				"On The Ground: %s\n"
				"DriftButtonMashing: %u\n"
				"Wrong direction: %s\n"
				"Last checkpoint: %i\n"
				"Last 2checkpoint: %i\n"
				"TestVar: %f"
				,place, speed, horizontalSpeed, fallSpeed, currentSteer, pushing ? "true" : "false", steering ? "true" : "false", onTheGround ? "true" : "false", driftButtonMasher.GetNTaps(), wrongDirection ? "true" : "false", lastCheckpoint, last2Checkpoint, testVar);


			string currentDriftPhase;
			switch (drifting)
			{
			case drift_none: currentDriftPhase = "None"; break;
			case drift_failed: currentDriftPhase = "Failed"; break;
			case drift_right_0: currentDriftPhase = "Drift Right"; break;
			case drift_right_1: currentDriftPhase = "Drift Right T1"; break;
			case drift_right_2: currentDriftPhase = "Drift Right T2"; break;
			case drift_left_0: currentDriftPhase = "Drift Left"; break;
			case drift_left_1: currentDriftPhase = "Drift Left T1"; break;
			case drift_left_2: currentDriftPhase = "Drift Left T2"; break;
			}
			ImGui::NewLine();
			ImGui::Text("Current drift phase:\n%s", currentDriftPhase.data());
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Turbo display"))
		{
			ImGui::Text("Phase: %u\nAccel bonus: %f\nAccel min: %f\nMax Speed Bonus: %f", turbo.GetCurrentPhase(), turbo_mods.accelerationBonus, turbo_mods.accelerationMin, turbo_mods.maxSpeedBonus);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Drifting"))
		{
			ImGui::Text("Clicks for phase change");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(	"Number of times the second player has to\n"
									"press \"X\"in order to give the first player\n"
									"a 1st or 2nd Tier turbo");
			ImGui::InputInt("##ClicksForPhaseChange", &driftPhaseChange);

			ImGui::Text("Drifting fixed turn");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(	"When drifting, if no directions are inputted,\n"
									"the amount of steering the kart will have\n"
									"(from 0 to 1)");
			ImGui::InputFloat("##DriftingFixedturn", &driftingFixedTurn);

			ImGui::Text("Drifting adjustable turn");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(	"When drifting, the amount that will be added\n"
									"or substracted from the \"Fixed Turn\" value\n"
									"currentSteer = FixedTurn + AdjustableTurn * JoystickInput[+-1]");
			ImGui::InputFloat("##DriftingAdjustableTurn", &driftingAdjustableTurn);

			ImGui::Text("Steer Change Speed");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(	"Speed at which the \"Current Steer\" value\n"
									"changes. High values make the drift more\n"
									"responsive, low ones make the turn smoother");
			ImGui::InputFloat("##SteerChangeSpeed", &steerChangeSpeed);

			ImGui::Text("Horizontal speed");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(	"Horizontal speed when drifting, perpendicular\n"
									"to the kart. This is what makes it slide.\n"
									"The value is multipliet by the direction inputted\n"
									"by the joystick, slightly modified to go from\n"
									"0 to 1");
			ImGui::InputFloat("##HorizontalSpeed", &horizontalSpeedDrifting);

			ImGui::Text("Minimum steer to start");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(	"Minimum steering the player must be inputting in\n"
									"order for the kart to start drifting.");
			ImGui::InputFloat("##MinimumDriftSteer", &minimumSteerToStartDrifting);

			ImGui::Text("Minimum speed to start");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(	"Minimum ratio (speed/maxSpeed) required for the kart\n"
									"to start drifting");
			ImGui::InputFloat("##MinimumDriftSpeed", &minimumSpeedRatioToStartDrifting);

			ImGui::Text("Drift Jump Speed");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(	"Vertical speed the kart gets when starting a drift.\n");
			ImGui::InputFloat("##DriftJumpSpeed", &driftJumpSpeed);

			ImGui::Text("Drift Jump Gravity Multiplier");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(	"Since the jump must be quick, we multiply the\n"
									"effect of gravity on the kart when jumping by this");
			ImGui::InputFloat("##DriftGravityMultiplier", &driftJumpGravityMultiplier);

			ImGui::Text("Visual Turn");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(	"Degrees the kart turns when drifting. Take in\n"
									"account that this is only visual and won't affect\n"
									"the drifting itself. Values Too high may result in\n"
									"an \"unresponsive\" drift ending");
			ImGui::InputFloat("##DriftVisualTurn", &driftVisualTurn);

			ImGui::Text("Amount of collisions until drift failure");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(	"Amount of times the function \"OnWallHit\" must be\n"
									"called in order to consider a drifting failed.\n"
									"Take in account that the most direct colisions will call\n"
									"it more than once during a collision.");
			ImGui::InputInt("##collisionsUntilStopDrifting", &collisionsUntilStopDrifting);

			ImGui::Text("Timer until collision recovery");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(	"Time until the kart \"forgets\" that it hit a wall\n"
									"and the collision counter resets back to 0.");
			ImGui::InputFloat("##driftCollisionRecovery", &driftCollisionRecovery);

			ImGui::TreePop();
		}

		ImGui::Separator();
		ImGui::NewLine();
		ImGui::Text("Old configuration");
		ImGui::Separator();

		//Choose car type popup
		if (ImGui::Button("Kart Type :"))
			ImGui::OpenPopup("Kart Type");

		if (ImGui::BeginPopup("Kart Type"))
		{
			if (ImGui::Selectable("Wood"))
			{
				SetCarType(T_WOOD);
			}
			if (ImGui::Selectable("Koji Lion"))
			{
				SetCarType(T_KOJI);
			}

			ImGui::EndPopup();
		}


		int player_f = (int)front_player;
		if (ImGui::InputInt("Front player joystick", &player_f, 1))
		{
			math::Clamp(player_f, 0, 3);
			front_player = (PLAYER)player_f;
		}
		int player_b = (int)back_player;
		if (ImGui::InputInt("Back player joystick", &player_b, 1))
		{
			math::Clamp(player_b, 0, 3);
			back_player = (PLAYER)player_b;
		}

		int lap = this->lap;
		if (ImGui::InputInt("Current lap", &lap))
		{
			if (lap < 0) lap = 0;
			this->lap = lap;
		}

		ImGui::Text("Last checkpoint: %u", checkpoints);
		/*
		if (vehicle)
		{
		if (ImGui::TreeNode("Read Stats"))
		{
		ImGui::Text("");

		ImGui::Text("Top velocity (Km/h) : %f", top_velocity);
		ImGui::Text("Current velocity (Km/h): %f", vehicle->GetKmh());
		ImGui::Text("Velocity boost (%): %f", speed_boost);
		ImGui::Text("");

		ImGui::Text("Current engine force : %f", accel);
		ImGui::Text("Engine force boost (%): %f", accel_boost);
		ImGui::Text("");

		ImGui::Text("Current turn: %f", turn_current);
		ImGui::Text("Current turn max: %f", turn_max);
		ImGui::Text("Turn boost (%): %f", turn_boost);
		ImGui::Text("");


		ImGui::Checkbox("On ground", &on_ground);

		bool on_t = current_turbo != T_IDLE;
		ImGui::Checkbox("On turbo", &on_t);

		if (on_t)
		{
		ImGui::Text("Time left: %f", (applied_turbo->time - applied_turbo->timer));
		}
		bool hasItem = has_item;
		if (ImGui::Checkbox("Has item", &hasItem))
		{
		if (hasItem == true)
		{
		PickItem();
		}
		}
		if (turned)
		{
		ImGui::Text("Time to reset: %f", (turn_over_reset_time - timer_start_turned));
		}

		ImGui::Text("");
		ImGui::TreePop();
		}
		}
		if (ImGui::TreeNode("Car settings"))
		{
		if (ImGui::TreeNode("Game loop settings"))
		{
		ImGui::Text("");

		ImGui::Text("Lose height");
		ImGui::SameLine();
		ImGui::DragFloat("##Lheight", &lose_height, 0.1f, 0.0f, 2.0f);

		ImGui::Text("");
		ImGui::TreePop();
		}
		*/
		if (ImGui::TreeNode("Control settings"))
		{
			/*
			if (ImGui::TreeNode("Acceleration settings"))
			{
			ImGui::Text("");
			ImGui::Text("Max speed");
			ImGui::SameLine();
			if (ImGui::DragFloat("##MxSpeed", &kart->max_velocity, 1.0f, 0.0f, 1000.0f)) {}

			ImGui::Text("Min speed");
			ImGui::SameLine();
			if (ImGui::DragFloat("##MnSpeed", &kart->min_velocity, 1.0f, -100.0f, 0.0f)) {}

			ImGui::Text("Accel");
			ImGui::SameLine();
			if (ImGui::DragFloat("##AccForce", &kart->accel_force, 1.0f, 0.0f)) {}

			ImGui::Text("Deceleration");
			ImGui::SameLine();
			if (ImGui::DragFloat("##DecelForce", &kart->decel_brake, 1.0f, 0.0f)) {}

			ImGui::Text("");
			ImGui::TreePop();
			}

			if (ImGui::TreeNode("Handling settings"))
			{
			ImGui::Text("");

			ImGui::Text("Base turn max");
			ImGui::SameLine();
			if (ImGui::DragFloat("##Turnmax", &kart->base_turn_max, 0.1f, 0.0f, 2.0f)) {}


			ImGui::Text("Turn speed");
			ImGui::SameLine();
			if (ImGui::DragFloat("##Wheel_turn_speed", &kart->turn_speed, 0.01f, 0.0f, 2.0f)) {}

			ImGui::Text("Joystick turn speed");
			if (ImGui::DragFloat("##joystick_turn_speed", &kart->turn_speed_joystick, 0.01f, 0.0f, 2.0f)) {}

			ImGui::Checkbox("Idle turn by interpolation", &kart->idle_turn_by_interpolation);
			if (kart->idle_turn_by_interpolation)
			{
			ImGui::Text("Time to idle turn");
			ImGui::DragFloat("##id_turn_time", &kart->time_to_idle, 0.01f, 0.0f);
			}

			ImGui::Text("");
			ImGui::TreePop();
			}

			if (ImGui::TreeNode("Max turn change settings"))
			{
			ImGui::Text("Velocity to begin change");
			ImGui::DragFloat("##v_to_change", &kart->velocity_to_begin_change, 0.1f, 0.0f);

			ImGui::Text("Limit max turn");
			ImGui::DragFloat("##l_max_turn", &kart->turn_max_limit, 1.0f, 0.0f);

			bool by_interpolation = (current_max_turn_change_mode == M_INTERPOLATION);
			bool by_speed = (current_max_turn_change_mode == M_SPEED);
			if (ImGui::Checkbox("By interpolation", &by_interpolation))
			current_max_turn_change_mode = M_INTERPOLATION;
			ImGui::SameLine();
			if (ImGui::Checkbox("By speed", &by_speed))
			current_max_turn_change_mode = M_SPEED;

			if (by_speed)
			{
			ImGui::Text("Base speed of max turn change");
			ImGui::DragFloat("##s_mx_tn_change", &kart->base_max_turn_change_speed, 0.1f);

			ImGui::Checkbox("Limit to a certain turn max", &limit_to_a_turn_max);

			ImGui::Checkbox("Accelerate the change", &accelerated_change);
			if (accelerated_change)
			{
			ImGui::Text("Base accel of max turn change speed");
			ImGui::DragFloat("##a_mx_tn_change", &kart->base_max_turn_change_accel, 0.01f);
			}
			}


			ImGui::Checkbox("Show max turn/ velocity graph", &show_graph);

			if (show_graph)
			{
			float values[14];

			for (int i = 0; i < 14; i++)
			{
			values[i] = GetMaxTurnByCurrentVelocity(float(i)* 10.0f);
			}

			ImGui::PlotLines("Max turn / Velocity", values, 14);
			}

			//NOTE: put a graph so the designers know how it  will affect turn max change over time
			ImGui::TreePop();


			}

			if (ImGui::TreeNode("Brake settings"))
			{
			ImGui::Text("");

			ImGui::Text("Brake force");
			ImGui::SameLine();
			if (ImGui::DragFloat("##Brake_force", &kart->brake_force, 1.0f, 0.0f, 1000.0f)) {}

			ImGui::Text("Back force");
			ImGui::SameLine();
			if (ImGui::DragFloat("##Back_force", &kart->back_force, 1.0f, 0.0f)) {}

			ImGui::Text("Full brake force");
			ImGui::SameLine();
			if (ImGui::DragFloat("##full_br_force", &kart->full_brake_force, 1.0f, 0.0f)) {}

			ImGui::Text("");
			ImGui::TreePop();
			}

			if (ImGui::TreeNode("Push settings"))
			{
			ImGui::Text("");

			ImGui::Text("Push force");
			ImGui::SameLine();
			if (ImGui::DragFloat("##push_force", &kart->push_force, 10.0f, 0.0f)) {}

			ImGui::Text("Push speed limit");
			ImGui::SameLine();
			if (ImGui::DragFloat("##push_sp", &kart->push_speed_per, 1.0f, 0.0f, 100.0f)) {}

			ImGui::Text("");
			ImGui::TreePop();
			}
			if (ImGui::TreeNode("Turbos"))
			{
			if (ImGui::TreeNode("mini turbo"))
			{
			ImGui::Checkbox("Accel %", &mini_turbo.per_ac);
			ImGui::SameLine();
			ImGui::Checkbox("Speed %", &mini_turbo.per_sp);

			ImGui::DragFloat("Accel boost", &mini_turbo.accel_boost, 1.0f, 0.0f);
			ImGui::DragFloat("Speed boost", &mini_turbo.speed_boost, 1.0f, 0.0f);
			ImGui::DragFloat("Duration", &mini_turbo.time);

			ImGui::Checkbox("Speed decrease", &mini_turbo.speed_decrease);
			if (mini_turbo.speed_decrease == true)
			{
			ImGui::DragFloat("Deceleration", &mini_turbo.deceleration, 1.0f, 0.0f);
			}

			ImGui::Checkbox("Direct speed", &mini_turbo.speed_direct);

			ImGui::TreePop();
			}

			if (ImGui::TreeNode("Drift turbo 2"))
			{
			ImGui::Checkbox("Accel %", &drift_turbo_2.per_ac);
			ImGui::SameLine();
			ImGui::Checkbox("Speed %", &drift_turbo_2.per_sp);

			ImGui::DragFloat("Accel boost", &drift_turbo_2.accel_boost, 1.0f, 0.0f);
			ImGui::DragFloat("Speed boost", &drift_turbo_2.speed_boost, 1.0f, 0.0f);
			ImGui::DragFloat("Duration", &drift_turbo_2.time);

			ImGui::Checkbox("Speed decrease", &drift_turbo_2.speed_decrease);
			if (drift_turbo_2.speed_decrease == true)
			{
			ImGui::DragFloat("Deceleration", &drift_turbo_2.deceleration, 1.0f, 0.0f);
			}

			ImGui::Checkbox("Direct speed", &drift_turbo_2.speed_direct);

			ImGui::TreePop();
			}


			if (ImGui::TreeNode("Drift turbo 3"))
			{
			ImGui::Checkbox("Accel %", &drift_turbo_3.per_ac);
			ImGui::SameLine();
			ImGui::Checkbox("Speed %", &drift_turbo_3.per_sp);

			ImGui::DragFloat("Accel boost", &drift_turbo_3.accel_boost, 1.0f, 0.0f);
			ImGui::DragFloat("Speed boost", &drift_turbo_3.speed_boost, 1.0f, 0.0f);
			ImGui::DragFloat("Duration", &drift_turbo_3.time);

			ImGui::Checkbox("Speed decrease", &drift_turbo_3.speed_decrease);
			if (drift_turbo_3.speed_decrease == true)
			{
			ImGui::DragFloat("Deceleration", &drift_turbo_3.deceleration, 1.0f, 0.0f);
			}
			ImGui::Checkbox("Direct speed", &drift_turbo_3.speed_direct);

			ImGui::TreePop();
			}

			ImGui::TreePop();
			}

			if (ImGui::TreeNode("Hitodamas"))
			{
			ImGui::DragInt("Max hitodamas", &max_hitodamas, 1.0f, 0, 100);
			ImGui::DragInt("Bonus hitodamas", &bonus_hitodamas, 1.0f, 0, 100);
			ImGui::TreePop();
			}
			if (ImGui::TreeNode("Items"))
			{
			ImGui::DragInt("Max hitodamas", &max_hitodamas, 1.0f, 0, 100);
			ImGui::DragInt("Bonus hitodamas", &bonus_hitodamas, 1.0f, 0, 100);
			if (ImGui::TreeNode("Rocket"))
			{

			if (ImGui::TreeNode("Turbo config"))
			{
			ImGui::Checkbox("Accel %", &rocket_turbo.per_ac);
			ImGui::SameLine();
			ImGui::Checkbox("Speed %", &rocket_turbo.per_sp);

			ImGui::DragFloat("Accel boost", &rocket_turbo.accel_boost, 1.0f, 0.0f);
			ImGui::DragFloat("Speed boost", &rocket_turbo.speed_boost, 1.0f, 0.0f);
			ImGui::DragFloat("Duration", &rocket_turbo.time);

			ImGui::Checkbox("Speed decrease", &rocket_turbo.speed_decrease);
			if (mini_turbo.speed_decrease == true)
			{
			ImGui::DragFloat("Deceleration", &rocket_turbo.deceleration, 1.0f, 0.0f);
			}
			ImGui::Checkbox("Direct speed", &rocket_turbo.speed_direct);

			ImGui::TreePop();
			}
			ImGui::TreePop();
			}
			ImGui::TreePop();
			}
			*/
			ImGui::TreePop();
		}
	} //Endof Car settings
}//Endof Collapsing header

void MashButtonCounter::Update(uint button, uint player)
{
	if (App->IsGameRunning() && App->IsGamePaused() == false)
	{
		if (App->input->GetJoystickButton(player, (JOY_BUTTON)button) == KEY_DOWN)
		{
			nTaps++;
		}
	}
}
