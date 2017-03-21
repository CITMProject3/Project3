#include "ComponentCar.h"

#include "Application.h"
#include "ModulePhysics3D.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui/imgui.h"

#include "Primitive.h"
#include "PhysVehicle3D.h"
#include "EventQueue.h"
#include "EventLinkGos.h"

#include "Time.h"

#include <string>

#include "SDL\include\SDL_scancode.h"

ComponentCar::ComponentCar(GameObject* GO) : Component(C_CAR, GO), chasis_size(1.0f, 0.2f, 2.0f), chasis_offset(0.0f, 0.0f, 0.0f)
{
	car = new VehicleInfo();

	car->mass = 400.0f;
	car->suspensionStiffness = 100.0f;
	car->suspensionCompression = 0.83f;
	car->suspensionDamping = 20.0f;
	car->maxSuspensionTravelCm = 1000.0f;
	car->frictionSlip = 50.5;
	car->maxSuspensionForce = 6000.0f;

	car->num_wheels = 4;
	car->wheels = new Wheel[4];

	//
	reset_pos = { 0.0f, 0.0f, 0.0f };
	reset_rot = { 0.0f, 0.0f, 0.0f };

	for (uint i = 0; i < 4; i++)
		wheels_go.push_back(nullptr);

	//Player config
	front_player = PLAYER_1;
	back_player = PLAYER_2;

	//Turbo
	mini_turbo.SetTurbo(500.0f, 50.0f, 3.0f);
}

ComponentCar::~ComponentCar()
{
	delete car;
}

void ComponentCar::Update()
{
	if (App->IsGameRunning())
	{
		if (vehicle)
		{		
			HandlePlayerInput();
			vehicle->Render();
			UpdateGO();
			GameLoopCheck();

		}
		else
			CreateCar();
	}
	else
	{
		vehicle = nullptr;
		RenderWithoutCar();
	}

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
				ImGui::Text("Turn boost (%): %f", turn_boost);
				ImGui::Text("");

				bool on_t = current_turbo != T_IDLE;
				ImGui::Checkbox("On turbo", &on_t);

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

				ImGui::Text("Pos");
				ImGui::SameLine();
				ImGui::DragFloat3("##Rpoint", reset_pos.ptr());

				ImGui::Text("Rot");
				ImGui::SameLine();
				ImGui::DragFloat3("##Rrot", reset_rot.ptr());

				ImGui::Text("");
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Control settings"))
			{
				if (ImGui::TreeNode("Acceleration settings"))
				{
					ImGui::Text("");
					ImGui::Text("Max speed");
					ImGui::SameLine();
					if (ImGui::DragFloat("##MxSpeed", &max_velocity, 1.0f, 0.0f, 1000.0f)) {}

					ImGui::Text("Min speed");
					ImGui::SameLine();
					if(ImGui::DragFloat("##MnSpeed", &min_velocity, 1.0f, -100.0f, 0.0f)){}

					ImGui::Text("Accel");
					ImGui::SameLine();
					if(ImGui::DragFloat("##AccForce", &accel_force, 1.0f, 0.0f)){}

					ImGui::Text("");
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Handling settings"))
				{
					ImGui::Text("");

					ImGui::Text("Turn max");
					ImGui::SameLine();
					if (ImGui::DragFloat("##Turnmax", &turn_max, 0.1f, 0.0f, 2.0f)) {}


					ImGui::Text("Turn speed");
					ImGui::SameLine();
					if (ImGui::DragFloat("##Wheel_turn_speed", &turn_speed, 0.01f, 0.0f, 2.0f)) {}

					ImGui::Text("");
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Brake settings"))
				{
					ImGui::Text("");

					ImGui::Text("Brake force");
					ImGui::SameLine();
					if (ImGui::DragFloat("##Brake_force", &brake_force, 1.0f, 0.0f, 1000.0f)) {}

					ImGui::Text("Back force");
					ImGui::SameLine();
					if (ImGui::DragFloat("##Back_force", &back_force, 1.0f, 0.0f)) {}

					ImGui::Text("");
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Push settings"))
				{
					ImGui::Text("");

					ImGui::Text("Push force");
					ImGui::SameLine();
					if (ImGui::DragFloat("##push_force", &push_force, 10.0f, 0.0f)) {}

					ImGui::Text("Push speed limit");
					ImGui::SameLine();
					if (ImGui::DragFloat("##push_sp", &push_speed_per, 1.0f, 0.0f, 100.0f)) {}

					ImGui::Text("");
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Leaning settings"))
				{
					ImGui::Text("");

					ImGui::Text("Speed boost");
					ImGui::SameLine();
					if(ImGui::DragFloat("##lean_sp_boost", &lean_top_sp, 0.5f, 0.0f, 200.0f)){}

					ImGui::Text("Accel boost");
					ImGui::SameLine();
					if (ImGui::DragFloat("##lean_accel_boost", &lean_top_acc, 0.5f, 0.0f, 200.0f)) {}

					ImGui::Text("Turn decrease");
					ImGui::SameLine();
					if (ImGui::DragFloat("##lean_res_turn", &lean_red_turn, 0.5f, 0.0f, 100.0f)) {}

					ImGui::Text("");
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Turbos"))
				{
					if (ImGui::TreeNode("Mini Turbo"))
					{
						ImGui::Checkbox("Accel %", &mini_turbo.per_ac);
						ImGui::SameLine();
						ImGui::Checkbox("Speed %", &mini_turbo.per_sp);

						ImGui::DragFloat("Accel boost", &mini_turbo.accel_boost, 1.0f, 0.0f);
						ImGui::DragFloat("Speed boost", &mini_turbo.speed_boost, 1.0f, 0.0f);
						ImGui::DragFloat("Duration", &mini_turbo.time);

						ImGui::Checkbox("Speed decrease", &mini_turbo.speed_decrease);
						if (mini_turbo.speed_decrease)
						{
							ImGui::DragFloat("Deceleration", &mini_turbo.deceleration, 1.0f, 0.0f);
						}
						ImGui::Checkbox("Direct speed", &mini_turbo.speed_direct);
						

						ImGui::TreePop();
					}

					ImGui::TreePop();
				}

				ImGui::TreePop();
			}

			if (App->IsGameRunning() == false)
			{
				if (ImGui::TreeNode("Chasis settings"))
				{
					ImGui::Text("Size");
					ImGui::SameLine();
					ImGui::DragFloat3("##Chasis size", chasis_size.ptr(), 0.1f, 0.1f, 5.0f);

					ImGui::Text("Offset");
					ImGui::SameLine();
					ImGui::DragFloat3("##Chasis offset", chasis_offset.ptr(), 0.1f, 0.1f, 5.0f);

					ImGui::Text("Mass");
					ImGui::SameLine();
					ImGui::DragFloat("##Mass", &car->mass, 1.0f, 0.1f, floatMax);

					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Suspension"))
				{
					ImGui::Text("Rest length");
					ImGui::SameLine();
					ImGui::DragFloat("##Suspension rest length", &suspensionRestLength, 0.1f, 0.1f, floatMax);

					ImGui::Text("Max travel (Cm)");
					ImGui::SameLine();
					ImGui::DragFloat("##Max suspension travel Cm", &car->maxSuspensionTravelCm, 1.0f, 0.1f, floatMax);

					ImGui::Text("Stiffness");
					ImGui::SameLine();
					ImGui::DragFloat("##Suspension stiffness", &car->suspensionStiffness, 0.1f, 0.1f, floatMax);

					ImGui::Text("Damping");
					ImGui::SameLine();
					ImGui::DragFloat("##Suspension Damping", &car->suspensionDamping, 1.0f, 0.1f, floatMax);

					ImGui::Text("Max force");
					ImGui::SameLine();
					ImGui::DragFloat("##Max suspension force", &car->maxSuspensionForce, 1.0f, 0.1f, floatMax);

					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Wheel settings"))
				{
					ImGui::Text("Connection height");
					ImGui::SameLine();
					ImGui::DragFloat("##Connection height", &connection_height, 0.1f, floatMin, floatMax);

					ImGui::Text("Radius");
					ImGui::SameLine();
					ImGui::DragFloat("##Wheel radius", &wheel_radius, 0.1f, 0.1f, floatMax);

					ImGui::Text("Width");
					ImGui::SameLine();
					ImGui::DragFloat("##Wheel width", &wheel_width, 0.1f, 0.1f, floatMax);
					ImGui::TreePop();
				}

				ImGui::Text("Friction Slip");
				ImGui::SameLine();
				ImGui::DragFloat("##Friction Slip", &car->frictionSlip, 1.0f, 0.1f, floatMax);
			}//Endof IsGameRunning() == false

			ImGui::Separator();
			ImGui::Text("Drifting settings");
			ImGui::NewLine();
			ImGui::InputFloat("Drift exit boost", &drift_boost);
			ImGui::InputFloat("Drift turn boost", &drift_turn_boost);
			ImGui::TreePop();
		} //Endof Car settings
		
		if (ImGui::TreeNode("Wheels"))
		{
			if (App->editor->assign_wheel != -1 && App->editor->wheel_assign != nullptr)
			{
				wheels_go[App->editor->assign_wheel] = App->editor->wheel_assign;
				App->editor->assign_wheel = -1;
				App->editor->wheel_assign = nullptr;
			}

			ImGui::Text("Front Left");
			if (wheels_go[0] != nullptr)
			{
				ImGui::Text(wheels_go[0]->name.c_str());
				ImGui::SameLine();
			}
			if (ImGui::Button("Assign Wheel##1"))
			{
				App->editor->assign_wheel = 0;
				App->editor->wheel_assign = nullptr;
			}
			ImGui::Text("Front Right");
			if (wheels_go[1] != nullptr)
			{
				ImGui::Text(wheels_go[1]->name.c_str());
				ImGui::SameLine();
			}
			if (ImGui::Button("Assign Wheel##2"))
			{
				App->editor->assign_wheel = 1;
				App->editor->wheel_assign = nullptr;

			}
			ImGui::Text("Back Left");
			if (wheels_go[2] != nullptr)
			{
				ImGui::Text(wheels_go[2]->name.c_str());
				ImGui::SameLine();
			}
			if (ImGui::Button("Assign Wheel##3"))
			{
				App->editor->assign_wheel = 2;
				App->editor->wheel_assign = nullptr;
			}
			ImGui::Text("Back Right");
			if (wheels_go[3] != nullptr)
			{
				ImGui::Text(wheels_go[3]->name.c_str());
				ImGui::SameLine();
			}
			if (ImGui::Button("Assign Wheel##4"))
			{
				App->editor->assign_wheel = 3;
				App->editor->wheel_assign = nullptr;
			}
			ImGui::TreePop();
		}
	}//Endof Collapsing header
}

void ComponentCar::HandlePlayerInput()
{
	float brake;
	bool turning = false;

	accel_boost = speed_boost = turn_boost = 0.0f;
	
	accel = brake = 0.0f;

	//  KEYBOARD CONTROLS__P1  ///////////////////////////////////////////////////////////////////////////////
	
	//Previous kick turbo (now usedd to test how tiles would work)
	/*if (kickTimer < kickCooldown) { kickTimer += time->DeltaTime(); }
	if (kickTimer >= kickCooldown)
	{
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN)
	{
	//accel = force;
	on_kick = true;
	kickTimer = 0.0f;
	}
	}

	if (on_kick && kickTimer < kick_force_time)
	{
	accel = force;
	}*/
	if (drifting == true)
	{
		turn_boost += drift_turn_boost;
	}
	
	KeyboardControls(&accel, &brake, &turning);

	//  JOYSTICK CONTROLS__P1  //////////////////////////////////////////////////////////////////////////////////
	/*if (App->input->GetNumberJoysticks() > 0)
	{
		//Kick to accelerate
		if (kickTimer >= kickCooldown)
		{
			if (App->input->GetJoystickButton(0, JOY_BUTTON::A) == KEY_DOWN)
			{
				accel = force;
				kickTimer = 0.0f;
			}
		}
		//Brake
		if (App->input->GetJoystickButton(0, JOY_BUTTON::B))
		{
			brake = brake_force;
		}
		//Turn
		float X_joy_input = App->input->GetJoystickAxis(0, JOY_AXIS::LEFT_STICK_X);
		if (math::Abs(X_joy_input) > 0.1f)
		{
			turn_current = turn_max * -X_joy_input;
		}
		if (App->input->GetJoystickButton(0, JOY_BUTTON::DPAD_RIGHT))
		{
			turning = true;

			turn_current -= turn_speed;
			if (turn_current < -turn_max)
				turn_current = -turn_max;
		}
		if (App->input->GetJoystickButton(0, JOY_BUTTON::DPAD_LEFT))
		{
			turning = true;

			turn_current += turn_speed;
			if (turn_current > turn_max)
				turn_current = turn_max;
		}


		if (App->input->GetJoystickButton(0, JOY_BUTTON::SELECT))
		{
			Reset();
		}
	}*/
	JoystickControls(&accel, &brake, &turning);
	ApplyTurbo();

	//Acrobactics control
	if (acro_on)
	{
		acro_timer += time->DeltaTime();

		if (acro_timer >= acro_time)
		{
			acro_on = false;
			acro_timer = 0.0f;
			acro_back = false;
			acro_front = false;
		}
	}
	
	//---------------------
	if (!turning)
		IdleTurn();

	if (drifting)
		CalcDriftForces();

	if (vehicle)
	{
		accel += accel_boost;
		//Doing this so it doesn't stop from braking
		vehicle->Turn(turn_current);
		vehicle->ApplyEngineForce(accel);
		vehicle->Brake(brake);

		if (accel != 0)
			LimitSpeed();
	}
}

void ComponentCar::JoystickControls(float* accel, float* brake, bool* turning)
{
	bool acro_front, acro_back;
	acro_front = acro_back = false;

	if (App->input->GetNumberJoysticks() > 0)
	{
		//Insert here all the new mechanicsç

		//Back player-------------------

		//Leaning
		if (App->input->GetJoystickButton(back_player, JOY_BUTTON::Y) == KEY_REPEAT)
		{
			Leaning(*accel);
		}

		//Acrobatics
		if (App->input->GetJoystickButton(back_player, JOY_BUTTON::X) == KEY_DOWN)
		{
			Acrobatics(back_player);
		}
		//Power Up

		//Push
		if (App->input->GetJoystickButton(back_player, JOY_BUTTON::A) == KEY_DOWN)
		{
			Push(accel);
		}

		//Slide attack


		//Front player------------------
		//Acceleration
		if (App->input->GetJoystickButton(front_player, JOY_BUTTON::A) == KEY_REPEAT)
		{
			Accelerate(accel);
		}

		//Brake/Backwards
		if (App->input->GetJoystickButton(front_player, JOY_BUTTON::B) == KEY_REPEAT)
		{
			Brake(accel, brake);
		}
		
		//Direction
		float x_joy_input = App->input->GetJoystickAxis(front_player, JOY_AXIS::LEFT_STICK_X);
		*turning = JoystickTurn(&turning_left, x_joy_input);

		if (App->input->GetJoystickButton(front_player, JOY_BUTTON::DPAD_RIGHT) == KEY_REPEAT)
		{
			*turning = Turn(&turning_left, false);
		}
		if (App->input->GetJoystickButton(front_player, JOY_BUTTON::DPAD_LEFT) == KEY_REPEAT)
		{
			*turning = Turn(&turning_left, true);
		}

		//Drifting
		
	
		//Acrobatics
		if (App->input->GetJoystickButton(front_player, JOY_BUTTON::X) == KEY_DOWN)
		{
			Acrobatics(front_player);
		}

		

	}
}

void ComponentCar::KeyboardControls(float* accel, float* brake, bool* turning)
{
	//Back player
	if (App->input->GetKey(SDL_SCANCODE_K) == KEY_DOWN)
	{
		Push(accel);
	}
	if (App->input->GetKey(SDL_SCANCODE_J) == KEY_REPEAT)
	{
		Leaning(*accel);
	}
	if (App->input->GetKey(SDL_SCANCODE_L) == KEY_DOWN)
	{
		Acrobatics(back_player);
	}
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN)
	{
		current_turbo = T_MINI;
	}

	//Front player
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
	{
		Accelerate(accel);
	}
	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		*turning = Turn(&turning_left, false);
	}
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		*turning = Turn(&turning_left, true);
	}
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		Brake(accel, brake);
	}
	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
	{
		Reset();
	}
	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN)
	{
		Acrobatics(front_player);
	}
	
	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && *turning == true)
	{
		StartDrift();
	}
	else if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_UP)
	{ 
		EndDrift();
	}	
}

// CONTROLS-----------------------------
bool ComponentCar::Turn(bool* left_turn, bool left)
{
	bool ret = true;
	float t_speed = turn_speed;

	float top_turn = turn_max + turn_boost;

	if (left)
	{
		*left_turn = true;
	}
	else
	{
		*left_turn = false;
		t_speed = -t_speed;
	}

	turn_current += t_speed;

	if (turn_current > top_turn)
		turn_current = top_turn;

	else if(turn_current < -top_turn)
		turn_current = -top_turn;

	return true;
}

bool ComponentCar::JoystickTurn(bool* left_turn, float x_joy_input)
{
	if (math::Abs(x_joy_input) > 0.1f)
	{
		turn_current = turn_speed * -x_joy_input;

		//TODO: adjust this with drifting
		return true;
	}
	return false;
}

void ComponentCar::Brake(float* accel, float* brake)
{
	if (vehicle->GetKmh() <= 0)
		*accel = -back_force;

	else
		*brake = brake_force;
}

void ComponentCar::Accelerate(float* accel)
{
	*accel += accel_force;
}

bool ComponentCar::Push(float* accel)
{
	bool ret = false;
	if (vehicle->GetKmh() < (max_velocity / 100)* push_speed_per)
	{
		*accel += push_force;
	}

	return ret;
}

void ComponentCar::Leaning(float accel)
{
	if (vehicle->GetKmh() > 0.0f)
	{
		accel_boost += ((accel / 100)*lean_top_acc);
		speed_boost += ((max_velocity / 100)*lean_top_sp);
		turn_boost -= ((turn_max / 100)*lean_red_turn);
	}
}

void ComponentCar::Acrobatics(PLAYER p)
{
	bool tmp_front = acro_front;
	bool tmp_back = acro_back;

	if(p == front_player)
	{ 
		acro_front = true;
	}
	else if (p == back_player)
	{
		acro_back = true;
	}

	if (acro_back && acro_front)
	{
		//Apply turbo
		current_turbo = T_MINI;

		acro_front = false;
		acro_back = false;
	}
	else if(tmp_back != acro_back || tmp_front != acro_front)
	{
		//Start timer
		acro_timer = 0.0f;

		acro_on = true;
	}
}


void ComponentCar::IdleTurn()
{
	if (turn_current > 0)
	{
		turn_current -= turn_speed;
		if (turn_current < 0)
			turn_current = 0;
	}
	else if (turn_current < 0)
	{
		turn_current += turn_speed;
		if (turn_current > 0)
			turn_current = 0;
	}
}

void ComponentCar::ApplyTurbo()
{
	bool start = false;

	if (start = (last_turbo != current_turbo))
	{
		switch (current_turbo)
		{
		case T_IDLE:
			applied_turbo = nullptr;
			break;
		case T_MINI:
			applied_turbo = &mini_turbo;
			break;

		}
	}

	last_turbo = current_turbo;

	//If there's a turbo on, apply it
	if (applied_turbo)
	{
		//Speed boost after first frame
		if (to_turbo_speed)
		{
			vehicle->SetModularVelocity(max_velocity + turbo_speed_boost);
			to_turbo_speed = false;
		}

		//Changes applied when turbo started
		if (start)
		{
			applied_turbo->timer = 0.0f;

			if (applied_turbo->per_ac)
				turbo_accel_boost = ((accel_force / 100) * applied_turbo->accel_boost);
			else
				turbo_accel_boost = applied_turbo->accel_boost;

			if (applied_turbo->per_sp)
				turbo_speed_boost = ((max_velocity / 100) * applied_turbo->speed_boost);
			else
				turbo_speed_boost = applied_turbo->speed_boost;


			if (applied_turbo->speed_direct)
			{
				to_turbo_speed = true;
			}

			turbo_deceleration = applied_turbo->deceleration;
			to_turbo_decelerate = applied_turbo->speed_decrease;
		}

		//Turbo applied every frame till it's time finish and then go to idle turbo
		if (applied_turbo->timer < applied_turbo->time)
		{
			accel_boost += turbo_accel_boost;
			speed_boost += turbo_speed_boost;

			applied_turbo->timer += time->DeltaTime();
		}
		else
		{
			current_turbo = T_IDLE;
		}
	}

	//Deceleration (without brake)
	if (current_turbo == T_IDLE && to_turbo_decelerate)
	{
		if (turbo_speed_boost > 0.0f)
		{
			turbo_speed_boost -= turbo_deceleration * time->DeltaTime();
			speed_boost += turbo_speed_boost;
		}
	}
}

void ComponentCar::StartDrift()
{
	drifting = true;
	drift_dir_left = turning_left;
	startDriftSpeed = vehicle->vehicle->getRigidBody()->getLinearVelocity();
	vehicle->SetFriction(0);
}

void ComponentCar::CalcDriftForces()
{
	vehicle->vehicle->getRigidBody()->clearForces();

	float4x4 matrix;
	vehicle->GetRealTransform().getOpenGLMatrix(matrix.ptr());
	matrix.Transpose();
	
	float3 front = matrix.WorldZ();
	float3 left = matrix.WorldX();
	float3 final_dir;
	if (drift_dir_left == true)
		left = -left;
	final_dir = left.Lerp(front, drift_ratio);

	btVector3 vector(final_dir.x, final_dir.y, final_dir.z);
	float l = startDriftSpeed.length();
	vehicle->vehicle->getRigidBody()->setLinearVelocity(vector * l * drift_mult);

	//Debugging lines
	//Front vector
	float3 start_line = matrix.TranslatePart();
	float3 end_line = start_line + front;
	App->renderer3D->DrawLine(start_line, end_line, float4(1, 0, 0, 1));
	//Left vector
	end_line = start_line + left;
	App->renderer3D->DrawLine(start_line, end_line, float4(0, 1, 0, 1));
	//Force vector
	end_line = start_line + final_dir;
	App->renderer3D->DrawLine(start_line, end_line, float4(1, 1, 1, 1));
}

void ComponentCar::EndDrift()
{
	vehicle->Turn(0);
	turn_current = 0;
	vehicle->SetFriction(car->frictionSlip);
	drifting = false;
	//Old turbo
	/*
	float data[16];
	vehicle->GetRealTransform().getOpenGLMatrix(data);
	float4x4 matrix = float4x4(data[0], data[1], data[2], data[3],
	data[4], data[5], data[6], data[7],
	data[8], data[9], data[10], data[11],
	data[12], data[13], data[14], data[15]);
	matrix.Transpose();

	float3 speed(matrix.WorldZ());
	speed *= startDriftSpeed.length();
	speed *= drift_boost;
	vehicle->SetLinearSpeed(speed.x, speed.y, speed.z);
	vehicle->vehicle->getRigidBody()->clearForces();
	vehicle->Turn(0);
	turn_current = 0;
	vehicle->SetFriction(car->frictionSlip);
	*/
}
//--------------------------------------

void ComponentCar::GameLoopCheck()
{
	if (game_object->transform->GetPosition().y <= lose_height)
		Reset();
}

void ComponentCar::Reset()
{
	vehicle->SetPos(reset_pos.x, reset_pos.y, reset_pos.z);
	vehicle->SetRotation(reset_rot.x, reset_rot.y, reset_rot.z);
	vehicle->SetLinearSpeed(0.0f, 0.0f, 0.0f);
}

void ComponentCar::LimitSpeed()
{
	//Tmp convertor
	float KmhToMs = 0.2777777;

	if (vehicle)
	{
		top_velocity = (max_velocity + speed_boost);
		if (vehicle->GetKmh() > top_velocity)
		{
			vehicle->SetModularSpeed(top_velocity * KmhToMs);
		}
		else if (vehicle->GetKmh() < min_velocity)
		{
			vehicle->SetModularSpeed(-(min_velocity * KmhToMs));
		}
	}
}

void ComponentCar::CreateCar()
{
	car->transform.Set(game_object->transform->GetGlobalMatrix());

	// Car properties ----------------------------------------
	car->chassis_size.Set(chasis_size.x, chasis_size.y, chasis_size.z);
	car->chassis_offset.Set(chasis_offset.x, chasis_offset.y, chasis_offset.z);


	float half_width = car->chassis_size.x*0.5f;
	float half_length = car->chassis_size.z*0.5f;

	float3 direction(0, -1, 0);
	float3 axis(-1, 0, 0);

	// FRONT-LEFT ------------------------
	car->wheels[0].connection.Set(half_width - 0.1f * wheel_width + chasis_offset.x, connection_height + chasis_offset.y, half_length - wheel_radius + chasis_offset.z);
	car->wheels[0].direction = direction;
	car->wheels[0].axis = axis;
	car->wheels[0].suspensionRestLength = suspensionRestLength;
	car->wheels[0].radius = wheel_radius;
	car->wheels[0].width = wheel_width;
	car->wheels[0].front = true;
	car->wheels[0].drive = false;
	car->wheels[0].brake = false;
	car->wheels[0].steering = true;

	// FRONT-RIGHT ------------------------
	car->wheels[1].connection.Set(-half_width + 0.1 * wheel_width + chasis_offset.x, connection_height + chasis_offset.y, half_length - wheel_radius + chasis_offset.z);
	car->wheels[1].direction = direction;
	car->wheels[1].axis = axis;
	car->wheels[1].suspensionRestLength = suspensionRestLength;
	car->wheels[1].radius = wheel_radius;
	car->wheels[1].width = wheel_width;
	car->wheels[1].front = true;
	car->wheels[1].drive = false;
	car->wheels[1].brake = false;
	car->wheels[1].steering = true;

	// REAR-LEFT ------------------------
	car->wheels[2].connection.Set(half_width - 0.1f * wheel_width + chasis_offset.x, connection_height + chasis_offset.y, -half_length + wheel_radius + chasis_offset.z);
	car->wheels[2].direction = direction;
	car->wheels[2].axis = axis;
	car->wheels[2].suspensionRestLength = suspensionRestLength;
	car->wheels[2].radius = wheel_radius;
	car->wheels[2].width = wheel_width;
	car->wheels[2].front = false;
	car->wheels[2].drive = true;
	car->wheels[2].brake = true;
	car->wheels[2].steering = false;

	// REAR-RIGHT ------------------------
	car->wheels[3].connection.Set(-half_width + 0.1f * wheel_width + chasis_offset.x, connection_height + chasis_offset.y, -half_length + wheel_radius + chasis_offset.z);
	car->wheels[3].direction = direction;
	car->wheels[3].axis = axis;
	car->wheels[3].suspensionRestLength = suspensionRestLength;
	car->wheels[3].radius = wheel_radius;
	car->wheels[3].width = wheel_width;
	car->wheels[3].front = false;
	car->wheels[3].drive = true;
	car->wheels[3].brake = true;
	car->wheels[3].steering = false;

	vehicle = App->physics->AddVehicle(*car);
}

void ComponentCar::OnTransformModified()
{}

void ComponentCar::UpdateGO()
{
	game_object->transform->Set(vehicle->GetTransform().Transposed());

	for (uint i = 0; i < wheels_go.size(); i++)
	{
		if (wheels_go[i] != nullptr)
		{
			ComponentTransform* w_trs = (ComponentTransform*)wheels_go[i]->GetComponent(C_TRANSFORM);
			float4x4 trans;
			vehicle->vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix(*trans.v);
			trans.Transpose();

			float3 scale = trans.GetScale();
			w_trs->SetGlobal(trans);
			w_trs->SetScale(scale);
		}
	}
}

void ComponentCar::RenderWithoutCar()
{
	//RENDERING CHASIS

	Cube_P chasis;
	chasis.size = chasis_size;
	chasis.transform = game_object->transform->GetGlobalMatrix().Transposed();
	float3 pos, scal;
	float3x3 rot;
	chasis.transform.Decompose(pos, rot, scal);
	float3 realOffset = rot * chasis_offset;
	chasis.transform = chasis.transform.Transposed() * chasis.transform.Translate(chasis_offset);
	chasis.transform.Transpose();
	chasis.Render();

	//RENDERING WHEELS

	Cylinder_P wheel;
	float3 wheelOffset;
	int _x, _z;
	for (int i = 0; i < 4; i++)
	{
		wheel.radius = wheel_radius;
		wheel.height = wheel_width;

		wheel.transform = game_object->transform->GetGlobalMatrix().Transposed();
		if (i == 0) { _x = 1; _z = 1; }
		else if (i == 1) { _x = -1; _z = -1; }
		else if (i == 2) { _x = -1; _z = 1; }
		else { _x = 1; _z = -1; }

		wheelOffset = chasis_offset;
		wheelOffset += float3((-chasis_size.x / 2.0f + 0.1f * wheel_width) * _x, connection_height - chasis_size.y / 2.0f, (-chasis_size.z / 2.0f + wheel_radius) * _z);

		realOffset = rot * wheelOffset;
		wheel.transform = wheel.transform.Transposed() * wheel.transform.Translate(wheelOffset);
		wheel.transform.Transpose();

		wheel.transform.Translate(realOffset);

		wheel.Render();
	}
}

void ComponentCar::Save(Data& file) const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);

	//Game loop settings
	data.AppendFloat("lose_height", lose_height);
	data.AppendFloat3("reset_pos", reset_pos.ptr());
	data.AppendFloat3("reset_rot", reset_rot.ptr());

	//Chassis settings
	data.AppendFloat3("chasis_size", chasis_size.ptr());
	data.AppendFloat3("chasis_offset", chasis_offset.ptr());

	//Controls settings --------------
	//Acceleration
	data.AppendFloat("acceleration", accel_force);
	data.AppendFloat("max_speed", max_velocity);
	data.AppendFloat("min_speed", min_velocity);

	//Turn 
	data.AppendFloat("turn_max", turn_max);
	data.AppendFloat("turn_speed", turn_speed);

	//Push
	data.AppendFloat("push_force", push_force);
	data.AppendFloat("push_speed_per", push_speed_per);

	//Brake
	data.AppendFloat("brakeForce", brake_force);
	data.AppendFloat("backForce", back_force);

	//Leaning
	data.AppendFloat("lean_accel_boost", lean_top_acc);
	data.AppendFloat("lean_speed_boost", lean_top_sp);
	data.AppendFloat("lean_turn_strict", lean_red_turn);

	//Drift 
	data.AppendFloat("driftRatio", drift_ratio);
	data.AppendFloat("driftMult", drift_mult);
	data.AppendFloat("driftBoost", drift_boost);

	//Turbos-------
	//Mini turbo
	data.AppendFloat("miniturbo_accel_boost", mini_turbo.accel_boost);
	data.AppendFloat("miniturbo_speed_boost", mini_turbo.speed_boost);
	data.AppendFloat("miniturbo_turbo_speed", mini_turbo.turbo_speed);
	data.AppendFloat("miniturbo_deceleration", mini_turbo.deceleration);
	data.AppendFloat("miniturbo_time", mini_turbo.time);

	data.AppendBool("miniturbo_accel_per", mini_turbo.per_ac);
	data.AppendBool("miniturbo_speed_per", mini_turbo.per_sp);
	data.AppendBool("miniturbo_speed_direct", mini_turbo.speed_direct);
	data.AppendBool("miniturbo_speed_decrease", mini_turbo.speed_decrease);


	//data.AppendFloat("kick_cooldown", kickCooldown);
	//--------------------------------------------------
	//Wheel settings
	data.AppendFloat("connection_height", connection_height);
	data.AppendFloat("wheel_radius", wheel_radius);
	data.AppendFloat("wheel_width", wheel_width);

	// Saving UUID's GameObjects linked as wheels on Component Car
	if (wheels_go[0]) data.AppendUInt("Wheel Front Left", wheels_go[0]->GetUUID());
	if (wheels_go[1]) data.AppendUInt("Wheel Front Right", wheels_go[1]->GetUUID());
	if (wheels_go[2]) data.AppendUInt("Wheel Back Left", wheels_go[2]->GetUUID());
	if (wheels_go[3]) data.AppendUInt("Wheel Back Right", wheels_go[3]->GetUUID());	
	
	//Car physics settings
	data.AppendFloat("mass", car->mass);
	data.AppendFloat("suspensionStiffness", car->suspensionStiffness);
	data.AppendFloat("suspensionCompression", car->suspensionCompression);
	data.AppendFloat("suspensionDamping", car->suspensionDamping);
	data.AppendFloat("suspensionRestLength", suspensionRestLength);
	data.AppendFloat("maxSuspensionTravelCm", car->maxSuspensionTravelCm);
	data.AppendFloat("frictionSlip", car->frictionSlip);
	data.AppendFloat("maxSuspensionForce", car->maxSuspensionForce);

	file.AppendArrayValue(data);
}

void ComponentCar::Load(Data& conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	//Game loop settings
	lose_height = conf.GetFloat("lose_height");
	reset_pos = conf.GetFloat3("reset_pos");
	reset_rot = conf.GetFloat3("reset_rot");

	//Chassis settings
	chasis_size = conf.GetFloat3("chasis_size");
	chasis_offset = conf.GetFloat3("chasis_offset");

	//Gameplay settings-----------------
	//Acceleration
	accel_force = conf.GetFloat("acceleration"); 
	max_velocity = conf.GetFloat("max_speed"); 
	min_velocity = conf.GetFloat("min_speed");

	//Turn 
	turn_max = conf.GetFloat("turn_max"); 
	turn_speed = conf.GetFloat("turn_speed");

	//Push
	push_force = conf.GetFloat("push_force"); 
	push_speed_per = conf.GetFloat("push_speed_per");

	//Brake
	brake_force = conf.GetFloat("brakeForce"); 
	back_force = conf.GetFloat("backForce"); 

	//Leaning
	lean_top_acc = conf.GetFloat("lean_accel_boost");  
	lean_top_sp = conf.GetFloat("lean_speed_boost"); 
	lean_red_turn = conf.GetFloat("lean_turn_strict");

	//Turbo
	//Mini turbo
	mini_turbo.accel_boost = conf.GetFloat("miniturbo_accel_boost");
	mini_turbo.speed_boost = conf.GetFloat("miniturbo_speed_boost");
	mini_turbo.turbo_speed = conf.GetFloat("miniturbo_turbo_speed");
	mini_turbo.deceleration = conf.GetFloat("miniturbo_deceleration");
	mini_turbo.time = conf.GetFloat("miniturbo_time");

	mini_turbo.per_ac = conf.GetBool("miniturbo_accel_per");
	mini_turbo.per_sp = conf.GetBool("miniturbo_speed_per");
	mini_turbo.speed_direct = conf.GetBool("miniturbo_speed_direct");
	mini_turbo.speed_decrease = conf.GetBool("miniturbo_speed_decrease");

	//kickCooldown = conf.GetFloat("kick_cooldown");
	//Wheel settings
	connection_height = conf.GetFloat("connection_height");
	wheel_radius = conf.GetFloat("wheel_radius");
	wheel_width = conf.GetFloat("wheel_width");

	// Posting events to further loading of GameObject wheels when all have been loaded)
	if (conf.GetUInt("Wheel Front Left") != 0)
	{
		EventLinkGos *ev = new EventLinkGos((GameObject**)&wheels_go[0], conf.GetUInt("Wheel Front Left"));
		App->event_queue->PostEvent(ev);
	}

	if (conf.GetUInt("Wheel Front Right") != 0)
	{
		EventLinkGos *ev = new EventLinkGos((GameObject**)&wheels_go[1], conf.GetUInt("Wheel Front Right"));
		App->event_queue->PostEvent(ev);
	}

	if (conf.GetUInt("Wheel Back Left") != 0)
	{
		EventLinkGos *ev = new EventLinkGos((GameObject**)&wheels_go[2], conf.GetUInt("Wheel Back Left"));
		App->event_queue->PostEvent(ev);
	}

	if (conf.GetUInt("Wheel Back Right") != 0)
	{
		EventLinkGos *ev = new EventLinkGos((GameObject**)&wheels_go[3], conf.GetUInt("Wheel Back Right"));
		App->event_queue->PostEvent(ev);
	}

	//Car settings
	car->mass = conf.GetFloat("mass");
	car->suspensionStiffness = conf.GetFloat("suspensionStiffness");
	car->suspensionCompression = conf.GetFloat("suspensionCompression");
	car->suspensionDamping = conf.GetFloat("suspensionDamping");
	suspensionRestLength = conf.GetFloat("suspensionRestLength");
	car->maxSuspensionTravelCm = conf.GetFloat("maxSuspensionTravelCm");
	car->frictionSlip = conf.GetFloat("frictionSlip");
	car->maxSuspensionForce = conf.GetFloat("maxSuspensionForce");

	//Drifting settings
	drift_ratio = conf.GetFloat("driftRatio");
	drift_mult = conf.GetFloat("driftMult");
	drift_boost = conf.GetFloat("driftBoost");
}


