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

ComponentCar::ComponentCar(GameObject* GO) : Component(C_CAR, GO), chasis_size(1.0f, 1.0f, 2.0f), chasis_offset(0.0f, 0.0f, 0.0f)
{
	car = new VehicleInfo();

	car->mass = 500.0f;
	car->suspensionStiffness = 15.88f;
	car->suspensionCompression = 0.83f;
	car->suspensionDamping = 0.88f;
	car->maxSuspensionTravelCm = 1000.0f;
	car->frictionSlip = 50.5;
	car->maxSuspensionForce = 6000.0f;
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
		}
		else
			CreateCar();
	}
	else 
	{
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

			if (ImGui::TreeNode("Control settings"))
			{
				ImGui::DragFloat("Wheel turn", &dturn, 0.1f, 0.0f, 2.0f);
				ImGui::DragFloat("Brake force", &brakeForce, 1.0f, 0.0f, 1000.0f);
				ImGui::DragFloat("Kick force", &force, 1.0f, 0.0f, floatMax);
				ImGui::DragFloat("Kick cooldown", &kickCooldown, 0.1f, 0.0f, 60.0f);
				ImGui::TreePop();
			}

			if (App->IsGameRunning() == false)
			{
				if (ImGui::TreeNode("Chasis settings"))
				{
					ImGui::DragFloat3("Chasis size", chasis_size.ptr(), 0.1f, 0.1f, 5.0f);
					ImGui::DragFloat3("Chasis offset", chasis_offset.ptr(), 0.1f, 0.1f, 5.0f);
					ImGui::DragFloat("Mass", &car->mass, 1.0f, 0.1f, floatMax);					
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Suspension"))
				{
					ImGui::DragFloat("Suspension rest length", &suspensionRestLength, 0.1f, 0.1f, floatMax);
					ImGui::DragFloat("Max suspension travel Cm", &car->maxSuspensionTravelCm, 1.0f, 0.1f, floatMax);
					ImGui::DragFloat("Suspension stiffness", &car->suspensionStiffness, 0.1f, 0.1f, floatMax);
					ImGui::DragFloat("Suspension Damping", &car->suspensionDamping, 1.0f, 0.1f, floatMax);					
					ImGui::DragFloat("Max suspension force", &car->maxSuspensionForce, 1.0f, 0.1f, floatMax);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Wheel settings"))
				{
					ImGui::DragFloat("Connection height", &connection_height, 0.1f, floatMin, floatMax);
					ImGui::DragFloat("Wheel radius", &wheel_radius, 0.1f, 0.1f, floatMax);
					ImGui::DragFloat("Wheel width", &wheel_width, 0.1f, 0.1f, floatMax);					
					ImGui::TreePop();
				}
				ImGui::DragFloat("Friction Slip", &car->frictionSlip, 1.0f, 0.1f, floatMax);
			}//Endof IsGameRunning() == false
			ImGui::TreePop();
		} //Endof Car settings
	}//Endof Collapsing header
}

void ComponentCar::HandlePlayerInput()
{

	float accel,turn,brake;

	accel = turn = brake = 0.0f;

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
			accel = force;
			kickTimer = 0.0f;
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
	{
		if (turn < dturn)
			turn -= dturn;
	}
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
	{
		if(turn > -dturn)
			turn += dturn;
	}
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
	{
		brake = brakeForce;
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
			turn = dturn * X_joy_input;
		}
		if (App->input->GetJoystickButton(0, JOY_BUTTON::DPAD_RIGHT))
		{
			if (turn < dturn)
				turn -= dturn;
		}
		if (App->input->GetJoystickButton(0, JOY_BUTTON::DPAD_LEFT))
		{
			if (turn < dturn)
				turn += dturn;
		}
	}


	if (vehicle)
	{
		vehicle->ApplyEngineForce(accel);
		vehicle->Turn(turn);
		vehicle->Brake(brake);
	}
}

void ComponentCar::CreateCar()
{

	ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
	car->transform.Set(trs->GetGlobalMatrix());

	// Car properties ----------------------------------------
	car->chassis_size.Set(chasis_size.x, chasis_size.y, chasis_size.z);
	car->chassis_offset.Set(chasis_offset.x, chasis_offset.y, chasis_offset.z);

	// Don't change anything below this line ------------------

	float half_width = car->chassis_size.x*0.5f;
	float half_length = car->chassis_size.z*0.5f;

	float3 direction(0, -1, 0);
	float3 axis(-1, 0, 0);

	car->num_wheels = 4;
	car->wheels = new Wheel[4];

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

	data.AppendFloat3("chasis_size", chasis_size.ptr());
	data.AppendFloat3("chasis_offset", chasis_offset.ptr());
	data.AppendFloat("kick_cooldown", kickCooldown);

	file.AppendArrayValue(data);
}

void ComponentCar::Load(Data& conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
}


