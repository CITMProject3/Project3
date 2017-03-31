#include "ComponentCar.h"

#include "Application.h"
#include "ModulePhysics3D.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentAnimation.h"

#include "imgui/imgui.h"

#include "Primitive.h"
#include "PhysVehicle3D.h"
#include "EventQueue.h"
#include "EventLinkGos.h"

#include "ModuleGOManager.h"
#include "ComponentCanvas.h"

#include "ComponentCollider.h"

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

	turn_max = base_turn_max;

	//
	reset_pos = { 0.0f, 0.0f, 0.0f };
	reset_rot = { 1.0f, 1.0f, 1.0f, 1.0f};

	for (uint i = 0; i < 4; i++)
		wheels_go.push_back(nullptr);

	//Player config
	front_player = PLAYER_1;
	back_player = PLAYER_2;

	//Turbo
	mini_turbo.SetTurbo("Mini turbo", 300.0f, 25.0f, 1.0f);
	turbos.push_back(mini_turbo);

	drift_turbo_2.SetTurbo("Drift turbo 2", 300.0f, 35.0f, 1.0f);
	turbos.push_back(drift_turbo_2);

	drift_turbo_3.SetTurbo("Drift turbo 3", 300.0f, 45.0f, 2.0f);
	turbos.push_back(drift_turbo_3);

	//Item
	rocket_turbo.SetTurbo("Rocket turbo", 0.0f, 50.0f, 10.0f);
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
			if (App->StartInGame() == false)
			{
				vehicle->Render();
			}
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

	ImGui::ShowTestWindow();

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
		ImGui::Text("Bool pushing: %i", (int)pushing);
		ImGui::Text("Current lap: %i", lap);
		if (lastCheckpoint != nullptr)
		{
			ImGui::Text("Last checkpoint: %s", lastCheckpoint->name.data());
		}
		else
		{
			ImGui::Text("Last checkpoint: NULL");
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
				ImGui::Text("Current turn max: %f", turn_max);
				ImGui::Text("Turn boost (%): %f", turn_boost);
				ImGui::Text("");

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

			if (ImGui::TreeNode("Control settings"))
			{
				if (ImGui::TreeNode("Turn over settings"))
				{
					ImGui::Text("Time to reset");
					ImGui::SameLine();
					if (ImGui::DragFloat("##rt_time", &turn_over_reset_time, 0.1f, 0.5f, 10.0f)) {}

					ImGui::TreePop();
				}
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

					ImGui::Text("Deceleration");
					ImGui::SameLine();
					if(ImGui::DragFloat("##DecelForce", &decel_brake, 1.0f, 0.0f)){}

					ImGui::Text("");
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Handling settings"))
				{
					ImGui::Text("");

					ImGui::Text("Base turn max");
					ImGui::SameLine();
					if (ImGui::DragFloat("##Turnmax", &base_turn_max, 0.1f, 0.0f, 2.0f)) {}


					ImGui::Text("Turn speed");
					ImGui::SameLine();
					if (ImGui::DragFloat("##Wheel_turn_speed", &turn_speed, 0.01f, 0.0f, 2.0f)) {}

					ImGui::Text("");
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Max turn change settings"))
				{
					ImGui::Text("Velocity to begin change");
					ImGui::DragFloat("##v_to_change", &velocity_to_begin_change, 0.1f, 0.0f);

					ImGui::Text("Limit max turn");
					ImGui::DragFloat("##l_max_turn", &turn_max_limit, 1.0f, 0.0f);

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
						ImGui::DragFloat("##s_mx_tn_change", &base_max_turn_change_speed, 0.1f);
						
						ImGui::Checkbox("Limit to a certain turn max", &limit_to_a_turn_max);

						ImGui::Checkbox("Accelerate the change", &accelerated_change);
						if (accelerated_change)
						{
							ImGui::Text("Base accel of max turn change speed");
							ImGui::DragFloat("##a_mx_tn_change", &base_max_turn_change_accel, 0.01f);
						}
					}

					
					ImGui::Checkbox("Show max turn/ velocity graph", &show_graph);

					if (show_graph)
					{
						float values[14];

						for (int i = 0; i < 14; i ++)
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
					if (ImGui::DragFloat("##Brake_force", &brake_force, 1.0f, 0.0f, 1000.0f)) {}

					ImGui::Text("Back force");
					ImGui::SameLine();
					if (ImGui::DragFloat("##Back_force", &back_force, 1.0f, 0.0f)) {}

					ImGui::Text("Full brake force");
					ImGui::SameLine();
					if(ImGui::DragFloat("##full_br_force", &full_brake_force, 1.0f, 0.0f)){}

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
					/*for (int i = 0; i < turbos.size(); i++)
					{
						Turbo* tmp = &turbos[i];

						if (ImGui::TreeNode(tmp->name.c_str()))
						{
							ImGui::Checkbox("Accel %", &(tmp->per_ac));
							ImGui::SameLine();
							ImGui::Checkbox("Speed %", &(tmp->per_sp));

							ImGui::DragFloat("Accel boost", &(tmp->accel_boost), 1.0f, 0.0f);
							ImGui::DragFloat("Speed boost", &(tmp->speed_boost), 1.0f, 0.0f);
							ImGui::DragFloat("Duration", &(tmp->time));

							ImGui::Checkbox("Speed decrease", &(tmp->speed_decrease));
							if (tmp->speed_decrease == true)
							{
								ImGui::DragFloat("Deceleration", &(tmp->deceleration), 1.0f, 0.0f);
							}
							ImGui::Checkbox("Direct speed", &(tmp->speed_direct));


							ImGui::TreePop();
						}
					}*/

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
						/*if (mini_turbo.speed_direct == true)
						{
							ImGui::Checkbox("Speed increase", &mini_turbo.speed_increase);

							if (mini_turbo.speed_increase)
							{
								ImGui::DragFloat("Fake acceleration", &mini_turbo.fake_accel, 1.0f, 0.0f);
							}
						}*/


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
						/*if (drift_turbo_2.speed_direct == true)
						{
							ImGui::Checkbox("Speed increase", &drift_turbo_2.speed_increase);

							if (drift_turbo_2.speed_increase)
							{
								ImGui::DragFloat("Fake acceleration", &drift_turbo_2.fake_accel, 1.0f, 0.0f);
							}
						}*/

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

						/*if (drift_turbo_3.speed_direct == true)
						{
							ImGui::Checkbox("Speed increase", &drift_turbo_3.speed_increase);

							if (drift_turbo_3.speed_increase)
							{
								ImGui::DragFloat("Fake acceleration", &drift_turbo_3.fake_accel, 1.0f, 0.0f);
							}
						}*/

						ImGui::TreePop();
					}
					
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Items"))
				{

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

							/*if (rocket_turbo.speed_direct == true)
							{
								ImGui::Checkbox("Speed increase", &rocket_turbo.speed_increase);

								if (rocket_turbo.speed_increase)
								{
									ImGui::DragFloat("Fake acceleration", &rocket_turbo.fake_accel, 1.0f, 0.0f);
								}
							}*/

							ImGui::TreePop();
						}
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
			ImGui::InputFloat("Drift min speed", &drift_min_speed);
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
		//TODO: provisional
		ImGui::Separator();
		if (item != nullptr)
		{
			ImGui::Text(item->name.c_str());
			ImGui::SameLine();
		}

		if (ImGui::Button("Assign Item"))
		{
			App->editor->assign_wheel = -1;
			App->editor->wheel_assign = nullptr;
			App->editor->assign_item = true;
			App->editor->to_assign_item = this;
		}

	}//Endof Collapsing header
}

void ComponentCar::OnPlay()
{
	ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
	if (trs)
	{
		reset_pos = trs->GetPosition();
		reset_rot = trs->GetRotation();
	}
	checkpoints = 255;
}

float ComponentCar::GetVelocity() const
{
	return vehicle->GetKmh();
}

void ComponentCar::HandlePlayerInput()
{
	turn_max = GetMaxTurnByCurrentVelocity(GetVelocity());

	float brake;
	bool turning = false;
	leaning = false;
	accel_boost = speed_boost = turn_boost = 0.0f;
	
	accel = brake = 0.0f;

	if (pushing)
	{
		if ( time->TimeSinceGameStartup() - pushStartTime >= 0.5f)
			pushing = false;
	}
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

	if (p2_animation != nullptr && p2_animation->current_animation != nullptr)
	{
		if (p2_animation->current_animation->index == 5)
		{
			Push(&accel);
		}
	}

	if (vehicle)
	{
		accel += accel_boost;
		//Doing this so it doesn't stop from braking
		vehicle->Turn(turn_current);
		vehicle->ApplyEngineForce(accel);
		vehicle->Brake(brake);

		if (!accel && !brake)
		{
			vehicle->Brake(decel_brake);
		}

		LimitSpeed();
	}



	UpdateTurnOver();
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
		if (App->input->GetJoystickButton(back_player, JOY_BUTTON::B) == KEY_REPEAT)
		{
			UseItem();
		}

		if (App->input->GetJoystickButton(back_player, JOY_BUTTON::B) == KEY_UP)
		{
			ReleaseItem();
		}
		//Push
		if (App->input->GetJoystickButton(back_player, JOY_BUTTON::A) == KEY_DOWN)
		{
			StartPush();
			//Push(accel);
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
		if (App->input->GetJoystickButton(front_player, JOY_BUTTON::RB) == KEY_DOWN && *turning == true)
		{
			StartDrift();
		}
		else if ( drifting == true && App->input->GetJoystickButton(front_player, JOY_BUTTON::RB) == KEY_UP)
		{
			EndDrift();
		}
	
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
		StartPush();
		LOG("Key push down");
//		Push(accel);
	}
	if (App->input->GetKey(SDL_SCANCODE_J) == KEY_REPEAT)
	{
		Leaning(*accel);
	}
	if (App->input->GetKey(SDL_SCANCODE_L) == KEY_DOWN)
	{
		Acrobatics(back_player);
	}
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
	{
		//current_turbo = T_MINI;
		UseItem();
	}
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_UP)
	{
		//current_turbo = T_MINI;
		ReleaseItem();
	}

	if (App->input->GetKey(SDL_SCANCODE_X) == KEY_REPEAT)
	{
		FullBrake(brake);
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
	else if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_UP && drifting == true)
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

	if (drifting == false)
	{
		if (left)
		{
			*left_turn = true;
		}
		else
		{
			*left_turn = false;
			t_speed = -t_speed;
		}
	}
	else if (left == false)
		t_speed = -t_speed;

	turn_current += t_speed * time->DeltaTime();

	if (drifting == false)
	{
		if (turn_current > top_turn)
			turn_current = top_turn;

		else if(turn_current < -top_turn)
			turn_current = -top_turn;
	}
	else
	{
		//Drifting wheel limitation 0 -> top_current
		if (drift_dir_left == false)
			top_turn = -top_turn;
		if (drift_dir_left ? turn_current < 0 : turn_current > 0)
			turn_current = 0;
		if (drift_dir_left ? turn_current > top_turn : turn_current < top_turn)
			turn_current = top_turn;
	}


	return true;
}

bool ComponentCar::JoystickTurn(bool* left_turn, float x_joy_input)
{
	if (math::Abs(x_joy_input) > 0.1f)
	{
		if (drifting == false)
			turn_current = turn_max * -x_joy_input;
		else
		{
			//Normalizing x_joy_input to 0-1 vlaue
			x_joy_input += 1;
			x_joy_input /= 2;

			if (drift_dir_left == true)
			{
				
				turn_current = turn_max * x_joy_input;
			}
			else
			{
				turn_current = -turn_max * x_joy_input;
			}
		}
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

void ComponentCar::FullBrake(float* brake)
{
	if (vehicle->GetKmh() > 0)
		*brake = full_brake_force;
}
void ComponentCar::Accelerate(float* accel)
{
	*accel += accel_force;
}

void ComponentCar::StartPush()
{
	pushing = true;
	pushStartTime = time->TimeSinceGameStartup();
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
	if (vehicle->GetKmh() > 0.0f && current_turbo == T_IDLE)
	{
		SetP2AnimationState(P2LEANING, 0.5f);
		leaning = true;
		accel_boost += ((accel / 100)*lean_top_acc);
		speed_boost += ((max_velocity / 100)*lean_top_sp);
		turn_boost -= ((turn_max / 100)*lean_red_turn);
	}
}

void ComponentCar::Acrobatics(PLAYER p)
{
	//This flow will need to be checked, it may cause some minor bugs in acrobatics functionality
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
		//current_turbo = T_MINI;
		if (drifting)
		{
			switch (turbo_drift_lvl)
			{
			case 0:
				turbo_drift_lvl = 1;
				break;
			case 1:
				turbo_drift_lvl = 2;
				break;
			case 2:
				turbo_drift_lvl = 3;
				break;
			}

			to_drift_turbo = true;
		}

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

void ComponentCar::PickItem()
{
	has_item = true;
	if (item != nullptr)
	{
		item->SetActive(true);
		for (std::vector<GameObject*>::const_iterator it = item->GetChilds()->begin(); it != item->GetChilds()->end(); it++)
		{
			GameObject* go = (*it)->GetChilds()->front();
			ComponentTransform* trans = (ComponentTransform*)go->GetComponent(C_TRANSFORM);
			float3 rotation = trans->GetRotationEuler();
			rotation.z = 0;
			trans->SetRotation(rotation);
			go->SetActive(false);
		}
	}
}

void ComponentCar::UseItem()
{
	if (has_item)
	{
		current_turbo = T_ROCKET;
		has_item = false;
	}

	//Rotating yellow cubes
	if (item != nullptr)
	{
		for (std::vector<GameObject*>::const_iterator it = item->GetChilds()->begin(); it != item->GetChilds()->end(); it++)
		{
			GameObject* go = (*it)->GetChilds()->front();
			if (go->IsActive() == false) go->SetActive(true);
			ComponentTransform* trans = (ComponentTransform*)go->GetComponent(C_TRANSFORM);
			float3 rotation = trans->GetRotationEuler();
			rotation.z += 1000 * time->DeltaTime();
			while (rotation.z > 360)
				rotation.z -= 360;

			trans->SetRotation(rotation);
		}
	}

	if (applied_turbo && current_turbo)
	{
		if (applied_turbo->timer >= applied_turbo->time)
		{
			ReleaseItem();
			vehicle->SetLinearSpeed(0.0f, 0.0f, 0.0f);
			current_turbo == T_IDLE;
			if (item != nullptr)
				item->SetActive(false);
		}
	}
}

void ComponentCar::ReleaseItem()
{
	if (current_turbo = T_ROCKET)
	{
		current_turbo = T_IDLE;
		if (item != nullptr)
		{
			item->SetActive(false);
		}
	}
}
void ComponentCar::IdleTurn()
{
	if (turn_current > 0)
	{
		turn_current -= turn_speed * time->DeltaTime();
		if (turn_current < 0)
			turn_current = 0;
	}
	else if (turn_current < 0)
	{
		turn_current += turn_speed * time->DeltaTime();
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
		case T_DRIFT_MACH_2:
			applied_turbo = &drift_turbo_2;
			break;
		case T_DRIFT_MACH_3:
			applied_turbo = &drift_turbo_3;
			break;
		case T_ROCKET:
			applied_turbo = &rocket_turbo;
			break;
		}
	}

	last_turbo = current_turbo;

	//If there's a turbo on, apply it
	if (applied_turbo)
	{
	

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


			if (applied_turbo->speed_direct && !applied_turbo->speed_increase)
			{
				float3 fv = game_object->transform->GetForward();
				float s_offset = 0.5;
				vehicle->SetVelocity(fv.x, fv.y, fv.z, max_velocity + turbo_speed_boost - s_offset);
			}

			turbo_deceleration = applied_turbo->deceleration;
			turbo_acceleration = applied_turbo->fake_accel;
			to_turbo_decelerate = applied_turbo->speed_decrease;
			current_speed_boost = 0.0f;
			speed_boost_reached = false;

		}

		//Turbo applied every frame till it's time finish and then go to idle turbo
		if (applied_turbo->timer < applied_turbo->time)
		{
			if (!speed_boost_reached)
			{
				if (applied_turbo->speed_direct && applied_turbo->speed_increase)
				{
					//Testing inn progress of progressive acceleration
					current_speed_boost += turbo_acceleration * time->DeltaTime();

					if (vehicle->GetKmh() > top_velocity)
					{
						speed_boost_reached = true;
					}

					float3 fv = game_object->transform->GetForward();
					float s_offset = 0.5;
					float current_velocity = GetVelocity();
					float desired_velocity = GetVelocity() + turbo_acceleration; //* time->DeltaTime();
					vehicle->SetVelocity(fv.x, fv.y, fv.z, desired_velocity);
				}

			}
			

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
	/*
	if (GetVelocity() >= drift_min_speed)
	{
		drifting = true;
		drift_dir_left = turning_left;
		startDriftSpeed = vehicle->vehicle->getRigidBody()->getLinearVelocity();
		vehicle->SetFriction(0);
	}
	*/
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
	float4x4 matrix;
	vehicle->GetRealTransform().getOpenGLMatrix(matrix.ptr());
	matrix.Transpose();

	float3 out_vector = matrix.WorldZ() * (float)startDriftSpeed.length();
	vehicle->vehicle->getRigidBody()->setLinearVelocity(btVector3(out_vector.x, out_vector.y, out_vector.z));
	//vehicle->SetLinearSpeed(startDriftSpeed);
	drifting = false;

	//New turbo
	if (to_drift_turbo)
	{
		switch (turbo_drift_lvl)
		{
		case 0:
			break;
		case 1:
			current_turbo = T_MINI;
			break;
		case 2:
			current_turbo = T_DRIFT_MACH_2;
			break;
		case 3:
			current_turbo = T_DRIFT_MACH_3;
			break;

		}

		turbo_drift_lvl = 0;
	}
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

void ComponentCar::UpdateTurnOver()
{
	float4x4 matrix;
	vehicle->GetRealTransform().getOpenGLMatrix(matrix.ptr());
	float3 up_vector = matrix.WorldY();

	if (up_vector.y < 0 && turned == false)
	{
		turned = true;
	}
	else if (turned = true)
	{
		if (up_vector.y < 0)
		{
			timer_start_turned += time->DeltaTime();
		}
		else if (up_vector.y > 0)
		{
			turned = false;
			timer_start_turned = 0.0f;
		}
		
		if (timer_start_turned >= turn_over_reset_time)
		{
			TurnOver();
			timer_start_turned = 0.0f;
			turned = false;
		}
	}
		
}

void ComponentCar::SetP2AnimationState(Player2_State state, float blend_ratio)
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
			p2_state = state;
			p2_animation->PlayAnimation(2, blend_ratio);
			break;
		}
		case(P2DRIFT_RIGHT):
		{
			p2_state = state;
			p2_animation->PlayAnimation(1, blend_ratio);
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
		case(P2LEANING):
		{
			if (p2_state != P2LEANING)
			{
				p2_state = state;
				p2_animation->PlayAnimation(7, blend_ratio);
			}
		}
	}
}

void ComponentCar::UpdateP2Animation()
{
	switch (p2_state)
	{
		case(P2IDLE):
		{
			if (drifting == true)
			{
				SetP2AnimationState(drift_dir_left ? P2DRIFT_LEFT : P2DRIFT_RIGHT);
			}
			else if (pushing == true)
			{
				SetP2AnimationState(P2PUSH_START);
			}
			else
			{
				p2_animation->current_animation->ticks_per_second = 8.0f + 24.0f * (GetVelocity() / (max_velocity + speed_boost));
			}
			break;
		}
		case(P2PUSH_START):
		{
			if (p2_animation->playing == false && pushing == true)
			{
				SetP2AnimationState(P2PUSH_LOOP);
			}
			else if (p2_animation->playing == false)
			{
				SetP2AnimationState(P2PUSH_END);
			}
			break;
		}
		case(P2PUSH_LOOP):
		{
			if (pushing == false)
			{
				SetP2AnimationState(P2PUSH_END);
			}
			break;
		}
		case (P2PUSH_END):
		{
			if (p2_animation->playing == false)
			{
				SetP2AnimationState(P2IDLE);
			}
			break;
		}
		case(P2DRIFT_LEFT):
		{
			if (drifting == false)
			{
				SetP2AnimationState(P2IDLE);
			}
			break;
		}
		case(P2DRIFT_RIGHT):
		{
			if (drifting == false)
			{
				SetP2AnimationState(P2IDLE);
			}
			break;
		}
		case(P2LEANING):
		{
			if (leaning == false)
			{
				SetP2AnimationState(P2IDLE);
			}
		}
	}
}

void ComponentCar::WentThroughCheckpoint(ComponentCollider* checkpoint)
{
	if (checkpoint->n == checkpoints + 1)
	{
		lastCheckpoint = checkpoint->GetGameObject();
		checkpoints = checkpoint->n;
	}
}

void ComponentCar::WentThroughEnd(ComponentCollider * end)
{
	if (checkpoints + 1 >= end->n)
	{
		checkpoints = 0;
		lap++;
		lastCheckpoint = end->GetGameObject();
	}
	if (lap >= 4)
	{
		TrueReset();
	}
}
//--------------------------------------

void ComponentCar::GameLoopCheck()
{
	if (game_object->transform->GetPosition().y <= lose_height)
		TurnOver();
}

void ComponentCar::TurnOver()
{
	Reset();
	/*float3 current_pos = vehicle->GetPos();
	current_pos.y += 2;
	float4x4 matrix = float4x4::identity;
	matrix.Translate(current_pos);
	vehicle->SetTransform(matrix.ptr());*/
}

void ComponentCar::Reset()
{
	if (lastCheckpoint == nullptr)
	{
		vehicle->SetPos(reset_pos.x, reset_pos.y, reset_pos.z);
		vehicle->SetRotation(reset_rot.x, reset_rot.y, reset_rot.z);
	}
	else
	{
		ComponentTransform* trs = (ComponentTransform*)lastCheckpoint->GetComponent(C_TRANSFORM);
		float3 pos = trs->GetPosition();
		vehicle->SetPos(pos.x, pos.y, pos.z);
		Quat rot = trs->GetRotation();
		vehicle->SetRotation(rot);
	}	
	vehicle->SetLinearSpeed(0.0f, 0.0f, 0.0f);
	vehicle->SetAngularSpeed(0.0f, 0.0f, 0.0f);
}

void ComponentCar::TrueReset()
{
	if (App->go_manager->current_scene_canvas != nullptr)
	{
		App->go_manager->current_scene_canvas->SetWin(true);
	}

	lastCheckpoint = nullptr;
	lap = 1;
	Reset();
}

void ComponentCar::LimitSpeed()
{
	//Tmp convertor
	float KmhToMs = 0.277;

	if (vehicle)
	{
		top_velocity = max_velocity + speed_boost;
		//Here went definition of top_velocity
		if (GetVelocity() > top_velocity)
		{
			vehicle->SetModularSpeed(top_velocity * KmhToMs);
		}
		else if (GetVelocity() < min_velocity)
		{
			vehicle->SetModularSpeed(-(min_velocity * KmhToMs));
		}
	}
}

float ComponentCar::GetVelocity()
{
	return vehicle->GetKmh();
}

float ComponentCar::GetMaxVelocity() const
{
	return max_velocity;
}

float ComponentCar::GetMinVelocity() const
{
	return min_velocity;
}

float ComponentCar::GetMaxTurnByCurrentVelocity(float sp)
{
	float max_t = base_turn_max;


	if (sp <= velocity_to_begin_change)
	{
		return max_t;
	}
	else
	{
		if (current_max_turn_change_mode == M_SPEED)
		{
			float velocity_dif = sp - velocity_to_begin_change;
			
			max_t += (velocity_dif * base_max_turn_change_speed);

			if (accelerated_change)
			{
				max_t += ((base_max_turn_change_accel / 2) * velocity_dif * velocity_dif);
			}

			if (max_t < turn_max_limit)
			{
				max_t = turn_max_limit;
			}

		}

		else if (current_max_turn_change_mode == M_INTERPOLATION)
		{
			float turn_max_change_dif = turn_max_limit - base_turn_max;
			float velocity_dif = max_velocity - velocity_to_begin_change;

			max_t += (turn_max_change_dif / velocity_dif) * (sp - velocity_to_begin_change);
		}

		
	}


	return max_t;
}

TURBO ComponentCar::GetCurrentTurbo() const
{
	return current_turbo;
}

void ComponentCar::CreateCar()
{
	std::vector<Component*> components;
	game_object->GetComponentsInChilds(C_ANIMATION, components);
	if (p1_animation == nullptr && components.size() > 0)
	{
		p1_animation = (ComponentAnimation*)components[0];
	}
	if (p2_animation == nullptr && components.size() > 1)
	{
		if ((p2_animation = (ComponentAnimation*)components[1]) != nullptr)
			p2_animation->PlayAnimation(3, 0.0f);
	}

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

	vehicle = App->physics->AddVehicle(*car, this);
}

void ComponentCar::OnTransformModified()
{}

void ComponentCar::UpdateGO()
{
	if (App->IsGameRunning() == false)
	{
		lastCheckpoint = nullptr;
		checkpoints = 0;
	}

	game_object->transform->Set(vehicle->GetTransform().Transposed());
	/*
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
	*/
	//Updating turn animation
	if (p1_animation != nullptr)
	{
		if (turn_current >= turn_max + turn_boost)
		{
			if (p1_animation->current_animation->index != 1)
			{
				p1_animation->PlayAnimation(1, 0.5f);
			}

		}
		else if (turn_current <= -turn_max - turn_boost)
		{
			if (p1_animation->current_animation->index != 2)
			{
				p1_animation->PlayAnimation(2, 0.5f);
			}
		}
		else
		{
			p1_animation->PlayAnimation((uint)0, 0.5f);
			float ratio = (-turn_current + turn_max + turn_boost) / (turn_max + turn_boost + (turn_max + turn_boost));
			p1_animation->LockAnimationRatio(ratio);
		}
	}

	//Player 2 animation
	if (p2_animation != nullptr)
	{
		UpdateP2Animation();
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

	//Chassis settings
	data.AppendFloat3("chasis_size", chasis_size.ptr());
	data.AppendFloat3("chasis_offset", chasis_offset.ptr());

	//Controls settings --------------
	//Turn over
	data.AppendFloat("turn_over_reset_time", turn_over_reset_time);

	//Acceleration
	data.AppendFloat("acceleration", accel_force);
	data.AppendFloat("max_speed", max_velocity);
	data.AppendFloat("min_speed", min_velocity);
	data.AppendFloat("fake_break", decel_brake);

	//Turn 
	data.AppendFloat("base_turn_max", base_turn_max);
	data.AppendFloat("turn_speed", turn_speed);

	//Max turn change
	data.AppendFloat("velocity_to_change", velocity_to_begin_change);
	data.AppendFloat("turn_max_limit", turn_max_limit);

	data.AppendFloat("base_max_turn_change_speed", base_max_turn_change_speed);
	data.AppendFloat("base_max_turn_change_accel", base_max_turn_change_accel);
	data.AppendBool("limit_to_a_turn_max", limit_to_a_turn_max);
	data.AppendBool("accelerated_change", accelerated_change);

	data.AppendInt("current_max_turn_change_mode", current_max_turn_change_mode);
	

	//Push
	data.AppendFloat("push_force", push_force);
	data.AppendFloat("push_speed_per", push_speed_per);

	//Brake
	data.AppendFloat("brakeForce", brake_force);
	data.AppendFloat("backForce", back_force);
	data.AppendFloat("full_brake_force", full_brake_force);

	//Leaning
	data.AppendFloat("lean_accel_boost", lean_top_acc);
	data.AppendFloat("lean_speed_boost", lean_top_sp);
	data.AppendFloat("lean_turn_strict", lean_red_turn);

	//Drift 
	data.AppendFloat("driftRatio", drift_ratio);
	data.AppendFloat("driftMult", drift_mult);
	data.AppendFloat("driftBoost", drift_boost);
	data.AppendFloat("driftMinSpeed", drift_min_speed);

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

		//Drift turbo 2

		data.AppendFloat("drift_turbo_2_accel_boost", drift_turbo_2.accel_boost);

		data.AppendFloat("drift_turbo_2_speed_boost", drift_turbo_2.speed_boost);
		data.AppendFloat("drift_turbo_2_turbo_speed", drift_turbo_2.turbo_speed);
		data.AppendFloat("drift_turbo_2_deceleration", drift_turbo_2.deceleration);
		data.AppendFloat("drift_turbo_2_time", drift_turbo_2.time);

		data.AppendBool("drift_turbo_2_accel_per", drift_turbo_2.per_ac);
		data.AppendBool("drift_turbo_2_speed_per", drift_turbo_2.per_sp);
		data.AppendBool("drift_turbo_2_speed_direct", drift_turbo_2.speed_direct);
		data.AppendBool("drift_turbo_2_speed_decrease", drift_turbo_2.speed_decrease);

		//Drift turbo 3

		data.AppendFloat("drift_turbo_3_accel_boost", drift_turbo_3.accel_boost);

		data.AppendFloat("drift_turbo_3_speed_boost", drift_turbo_3.speed_boost);
		data.AppendFloat("drift_turbo_3_turbo_speed", drift_turbo_3.turbo_speed);
		data.AppendFloat("drift_turbo_3_deceleration", drift_turbo_3.deceleration);
		data.AppendFloat("drift_turbo_3_time", drift_turbo_3.time);

		data.AppendBool("drift_turbo_3_accel_per", drift_turbo_3.per_ac);
		data.AppendBool("drift_turbo_3_speed_per", drift_turbo_3.per_sp);
		data.AppendBool("drift_turbo_3_speed_direct", drift_turbo_3.speed_direct);
		data.AppendBool("drift_turbo_3_speed_decrease", drift_turbo_3.speed_decrease);

		//Rocket turbo 

		data.AppendFloat("rocket_turbo_accel_boost", rocket_turbo.accel_boost);

		data.AppendFloat("rocket_turbo_speed_boost", rocket_turbo.speed_boost);
		data.AppendFloat("rocket_turbo_turbo_speed", rocket_turbo.turbo_speed);
		data.AppendFloat("rocket_turbo_deceleration", rocket_turbo.deceleration);
		data.AppendFloat("rocket_turbo_time", rocket_turbo.time);

		data.AppendBool("rocket_turbo_accel_per", rocket_turbo.per_ac);
		data.AppendBool("rocket_turbo_speed_per", rocket_turbo.per_sp);
		data.AppendBool("rocket_turbo_speed_direct", rocket_turbo.speed_direct);
		data.AppendBool("rocket_turbo_speed_decrease", rocket_turbo.speed_decrease);
	


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
	if (item) data.AppendUInt("Item", item->GetUUID());

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

	//Chassis settings
	chasis_size = conf.GetFloat3("chasis_size");
	chasis_offset = conf.GetFloat3("chasis_offset");

	//Gameplay settings-----------------
	//Turn over
	turn_over_reset_time = conf.GetFloat("turn_over_reset_time");
	if(turn_over_reset_time < 0.2f)
	{
		turn_over_reset_time = 4.0f;
	}

	//Acceleration
	accel_force = conf.GetFloat("acceleration"); 
	max_velocity = conf.GetFloat("max_speed"); 
	min_velocity = conf.GetFloat("min_speed");
	decel_brake = conf.GetFloat("fake_break");

	//Turn 
	base_turn_max = conf.GetFloat("base_turn_max"); 
	turn_speed = conf.GetFloat("turn_speed");

	//Max turn change
	velocity_to_begin_change = conf.GetFloat("velocity_to_change"); 
	turn_max_limit = conf.GetFloat("turn_max_limit"); 

	base_max_turn_change_speed = conf.GetFloat("base_max_turn_change_speed");
	base_max_turn_change_accel = conf.GetFloat("base_max_turn_change_accel"); 
	limit_to_a_turn_max = conf.GetBool("limit_to_a_turn_max");
	accelerated_change = conf.GetBool("accelerated_change");

	current_max_turn_change_mode = MAX_TURN_CHANGE_MODE(conf.GetInt("current_max_turn_change_mode"));

	//Push
	push_force = conf.GetFloat("push_force"); 
	push_speed_per = conf.GetFloat("push_speed_per");

	//Brake
	brake_force = conf.GetFloat("brakeForce"); 
	back_force = conf.GetFloat("backForce"); 
	full_brake_force = conf.GetFloat("full_brake_force");

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

	//Drift turbo 2
	drift_turbo_2.accel_boost = conf.GetFloat("drift_turbo_2_accel_boost");
	drift_turbo_2.speed_boost = conf.GetFloat("drift_turbo_2_speed_boost");
	drift_turbo_2.turbo_speed = conf.GetFloat("drift_turbo_2_turbo_speed");
	drift_turbo_2.deceleration = conf.GetFloat("drift_turbo_2_deceleration");
	drift_turbo_2.time = conf.GetFloat("drift_turbo_2_time");

	drift_turbo_2.per_ac = conf.GetBool("drift_turbo_2_accel_per");
	drift_turbo_2.per_sp = conf.GetBool("drift_turbo_2_speed_per");
	drift_turbo_2.speed_direct = conf.GetBool("drift_turbo_2_speed_direct");
	drift_turbo_2.speed_decrease = conf.GetBool("drift_turbo_2_speed_decrease");

	//Drift turbo 3
	drift_turbo_3.accel_boost = conf.GetFloat("drift_turbo_3_accel_boost");
	drift_turbo_3.speed_boost = conf.GetFloat("drift_turbo_3_speed_boost");
	drift_turbo_3.turbo_speed = conf.GetFloat("drift_turbo_3_turbo_speed");
	drift_turbo_3.deceleration = conf.GetFloat("drift_turbo_3_deceleration");
	drift_turbo_3.time = conf.GetFloat("drift_turbo_3_time");

	drift_turbo_3.per_ac = conf.GetBool("drift_turbo_3_accel_per");
	drift_turbo_3.per_sp = conf.GetBool("drift_turbo_3_speed_per");
	drift_turbo_3.speed_direct = conf.GetBool("drift_turbo_3_speed_direct");
	drift_turbo_3.speed_decrease = conf.GetBool("drift_turbo_3_speed_decrease");

	//Rocket
	rocket_turbo.accel_boost = conf.GetFloat("rocket_turbo_accel_boost");
	rocket_turbo.speed_boost = conf.GetFloat("rocket_turbo_speed_boost");
	rocket_turbo.turbo_speed = conf.GetFloat("rocket_turbo_turbo_speed");
	rocket_turbo.deceleration = conf.GetFloat("rocket_turbo_deceleration");
	rocket_turbo.time = conf.GetFloat("rocket_turbo_time");

	rocket_turbo.per_ac = conf.GetBool("rocket_turbo_accel_per");
	rocket_turbo.per_sp = conf.GetBool("rocket_turbo_speed_per");
	rocket_turbo.speed_direct = conf.GetBool("rocket_turbo_speed_direct");
	rocket_turbo.speed_decrease = conf.GetBool("rocket_turbo_speed_decrease");
	

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

	if (conf.GetUInt("Item") != 0)
	{
		EventLinkGos *ev = new EventLinkGos((GameObject**)&item, conf.GetUInt("Item"));
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
	drift_min_speed = conf.GetFloat("driftMinSpeed");
}


