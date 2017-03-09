#include "ComponentCar.h"
#include "Application.h"
#include "GameObject.h"
#include "imgui/imgui.h"
#include "Globals.h"
#include "ModulePhysics3D.h"
#include "ModuleInput.h"
#include "ComponentTransform.h"
#include "Primitive.h"
#include "PhysVehicle3D.h"
#include <string>

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

		if (ImGui::TreeNode("Car settings"))
		{
			if (ImGui::TreeNode("Game loop settings"))
			{
				ImGui::Text("Lose height");
				ImGui::SameLine();
				ImGui::DragFloat("##Lheight", &lose_height, 0.1f, 0.0f, 2.0f);

				ImGui::Text("Reset position");
				ImGui::SameLine();
				ImGui::DragFloat3("##Rpoint", reset_pos.ptr());

				ImGui::Text("Reset rotation");
				ImGui::SameLine();
				ImGui::DragFloat3("##Rrot", reset_rot.ptr());

			
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Control settings"))
			{
				ImGui::Text("Turn max");
				ImGui::SameLine();
				if(ImGui::DragFloat("##Turnmax", &turn_max, 0.1f, 0.0f, 2.0f)){}
				

				ImGui::Text("Wheel turn speed");
				ImGui::SameLine();
				if(ImGui::DragFloat("##Wheel_turn_speed", &turn_speed, 0.01f, 0.0f, 2.0f)){}


				ImGui::Text("Brake force");
				ImGui::SameLine();
				if (ImGui::DragFloat("##Brake_force", &brakeForce, 1.0f, 0.0f, 1000.0f)){}

				ImGui::Text("Kick force");
				ImGui::SameLine();
				if(ImGui::DragFloat("##Kick_force", &force, 1.0f, 0.0f, floatMax)){}

				ImGui::Text("Kick cooldown");
				ImGui::SameLine();
				if(ImGui::DragFloat("##Kick_cooldown", &kickCooldown, 0.1f, 0.0f, 60.0f)){}

				ImGui::Text("Kick force time");
				ImGui::SameLine();
				if(ImGui::DragFloat("##Kick_force_time", &kick_force_time, 0.025f, 0.0f, 20.0f)){}

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
			ImGui::TreePop();
		} //Endof Car settings
	}//Endof Collapsing header
}

void ComponentCar::HandlePlayerInput()
{

	float accel,brake;
	bool turning = false;
	float extra_force = 1000.0f;

	accel = brake = 0.0f;

	// DEBUG CONTROLS  ///////////////////////////////////////////////////////////////////////////////
	if (App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN || (App->input->GetNumberJoysticks() > 0 && App->input->GetJoystickButton(0, JOY_BUTTON::START) == KEY_DOWN))
	{
		vehicle->SetRotation(0, 0, 0);
	}

	//  KEYBOARD CONTROLS__P1  ///////////////////////////////////////////////////////////////////////////////
	if (kickTimer < kickCooldown) { kickTimer += time->DeltaTime(); }
	if (kickTimer >= kickCooldown)
	{
		if (App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
		{
			//accel = force;
			on_kick = true;
			kickTimer = 0.0f;
		}
	}
	if (on_kick && kickTimer < kick_force_time)
	{
		accel = force;
	}

	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		turning = true;

			turn_current -= turn_speed;
			if (turn_current < -turn_max)
				turn_current = -turn_max;
			
	}
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		turning = true;
		
		turn_current += turn_speed;
		if (turn_current > turn_max)
			turn_current = turn_max;
	}
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		brake = brakeForce;
	}

	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
	{
		Reset();
	}

	//fOR DEBUG FOR NOW
	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT)
	{
		accel += extra_force;
	}


	//  JOYSTICK CONTROLS__P1  //////////////////////////////////////////////////////////////////////////////////
	if (App->input->GetNumberJoysticks() > 0)
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
			brake = brakeForce;
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
	}


	//---------------------
	if (!turning)
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

	if (vehicle)
	{
		vehicle->ApplyEngineForce(accel);
		vehicle->Turn(turn_current);
		vehicle->Brake(brake);
	}
}

void ComponentCar::GameLoopCheck()
{
	if (((ComponentTransform*)game_object->GetComponent(C_TRANSFORM))->GetPosition().y <= lose_height)
	{
		Reset();
	}
}

void ComponentCar::Reset()
{
	vehicle->SetPos(reset_pos.x, reset_pos.y, reset_pos.z);
	vehicle->SetRotation(reset_rot.x, reset_rot.y, reset_rot.z);
	vehicle->SetLinearSpeed(0.0f, 0.0f, 0.0f);
}

void ComponentCar::CreateCar()
{
	ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
	car->transform.Set(trs->GetGlobalMatrix());

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
	ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
	trs->Set(vehicle->GetTransform().Transposed());
}

void ComponentCar::RenderWithoutCar()
{
	ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);

	//RENDERING CHASIS

	Cube_P chasis;
	chasis.size = chasis_size;
	chasis.transform = trs->GetGlobalMatrix().Transposed();
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

		wheel.transform = trs->GetGlobalMatrix().Transposed();
		if (i == 0) { _x = 1; _z = 1; }
		else if (i == 1) { _x = -1; _z = -1; }
		else if (i == 2) { _x = -1; _z = 1; }
		else { _x = 1; _z = -1; }

		wheelOffset = chasis_offset;
		wheelOffset += float3((-chasis_size.x / 2.0f + 0.1f * wheel_width) * _x, connection_height - chasis_size.y/2.0f,( -chasis_size.z / 2.0f + wheel_radius) * _z);

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


	data.AppendFloat3("chasis_size", chasis_size.ptr());
	data.AppendFloat3("chasis_offset", chasis_offset.ptr());

	data.AppendFloat("kick_cooldown", kickCooldown);
	data.AppendFloat("connection_height", connection_height);
	data.AppendFloat("wheel_radius", wheel_radius);
	data.AppendFloat("wheel_width", wheel_width);
	data.AppendFloat("suspensionRestLength", suspensionRestLength);
	data.AppendFloat("turn_max", turn_max);
	data.AppendFloat("turn_speed", turn_speed);
	data.AppendFloat("force", force);
	data.AppendFloat("brakeForce", brakeForce);

	data.AppendFloat("mass", car->mass);
	data.AppendFloat("suspensionStiffness", car->suspensionStiffness);
	data.AppendFloat("suspensionCompression", car->suspensionCompression);
	data.AppendFloat("suspensionDamping", car->suspensionDamping);
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

	chasis_size = conf.GetFloat3("chasis_size");
	chasis_offset = conf.GetFloat3("chasis_offset");

	kickCooldown = conf.GetFloat("kick_cooldown");
	connection_height  = conf.GetFloat("connection_height");
	wheel_radius = conf.GetFloat("wheel_radius");
	wheel_width = conf.GetFloat("wheel_width");
	suspensionRestLength = conf.GetFloat("suspensionRestLength");
	turn_max = conf.GetFloat("turn_max");
	turn_speed = conf.GetFloat("turn_speed");
	force = conf.GetFloat("force");
	brakeForce = conf.GetFloat("brakeForce");

	car->mass = conf.GetFloat("mass");
	car->suspensionStiffness = conf.GetFloat("suspensionStiffness");
	car->suspensionCompression = conf.GetFloat("suspensionCompression");
	car->suspensionDamping = conf.GetFloat("suspensionDamping");
	car->maxSuspensionTravelCm = conf.GetFloat("maxSuspensionTravelCm");
	car->frictionSlip = conf.GetFloat("frictionSlip");
	car->maxSuspensionForce = conf.GetFloat("maxSuspensionForce");
}


