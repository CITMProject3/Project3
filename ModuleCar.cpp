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

#define DISTANCE_FROM_GROUND 1

ModuleCar::ModuleCar(const char* name, bool start_enabled) : Module(name, start_enabled)
{
}

// Destructor
ModuleCar::~ModuleCar()
{
}

// Called before render is available
bool ModuleCar::Init(Data& config)
{
	bool ret = true;
	return ret;
}

bool ModuleCar::Start()
{
	return true;
}

// Called every draw update
update_status ModuleCar::PreUpdate()
{
	if (loaded == false)
	{
		FindKartGOs();

		if (kart == nullptr || kart_trs == nullptr)
		{
			AssetFile* kartFile = App->editor->assets->FindAssetFile("/Assets/kart.fbx");
			if (kartFile != nullptr)
			{
				App->resource_manager->LoadFile(kartFile->content_path, FileType::MESH);
			}
		}
		if (track == nullptr)
		{
			AssetFile* trackFile = App->editor->assets->FindAssetFile("/Assets/track_test.fbx");
			if (trackFile != nullptr)
			{
				App->resource_manager->LoadFile(trackFile->content_path, FileType::MESH);
			}
		}

		if (light == nullptr)
		{
			light = App->go_manager->CreateGameObject(NULL);
			light->name = "Directional_Light";
			light->AddComponent(ComponentType::C_LIGHT);
			ComponentTransform* tmp = (ComponentTransform*)light->GetComponent(C_TRANSFORM);
			tmp->SetRotation(float3(50, -10, -50));
		}
		if (cam != nullptr)
		{
			if (cam->GetComponent(C_CAMERA) == nullptr)
			{
				camera = (ComponentCamera*)cam->AddComponent(C_CAMERA);
			}
		}
		loaded = true;
	}

	return UPDATE_CONTINUE;
}

update_status ModuleCar::Update()
{
	if (App->IsGameRunning())
	{
		if (firstFrameOfExecution)
		{
			App->renderer3D->SetCamera(camera);
			firstFrameOfExecution = false;
		}

		Car_Debug_Ui();

		if (kart && kart_trs)
		{
			KartLogic();
		}
	}
	else
	{
		if (firstFrameOfExecution == false)
		{
			App->renderer3D->SetCamera(App->camera->GetEditorCamera());
			FindKartGOs();
			firstFrameOfExecution = true;
		}
	}
	return UPDATE_CONTINUE;
}

update_status ModuleCar::PostUpdate()
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleCar::CleanUp()
{
	return true;
}

void ModuleCar::KartLogic()
{
	float3 pos = kart_trs->GetPosition();
	float3 newPos = pos;
	float3 kartY = kart_trs->GetGlobalMatrix().WorldY();

	//Setting the two rays, Front and Back
	math::Ray rayF, rayB;
	rayB.dir = rayF.dir = -kartY;
	rayB.pos = rayF.pos = kart_trs->GetPosition() + kartY;
	rayF.pos += kart_trs->GetGlobalMatrix().WorldZ();
	rayB.pos -= kart_trs->GetGlobalMatrix().WorldZ();

	//Raycasting, checking only for the NavMesh layer
	RaycastHit hitF = App->go_manager->Raycast(rayF, std::vector<int>(1, track->layer));
	RaycastHit hitB = App->go_manager->Raycast(rayB, std::vector<int>(1, track->layer));

	//Setting the "desired up" value, taking in account if both rays are close enough to the ground or none of them
	float3 desiredUp = float3(0, 1, 0);
	onTheGround = false;
	if ((hitF.object != nullptr && hitF.distance < DISTANCE_FROM_GROUND + 1) && (hitB.object != nullptr && hitB.distance < DISTANCE_FROM_GROUND + 1))
	{
		onTheGround = true;
		desiredUp = hitF.normal.Lerp(hitB.normal, 0.5f);
		newPos = hitB.point + (hitF.point - hitB.point) / 2;
	}
	else if ((hitF.object != nullptr && hitF.distance < DISTANCE_FROM_GROUND + 0.8) && !(hitB.object != nullptr && hitB.distance < DISTANCE_FROM_GROUND + 0.8))
	{
		onTheGround = true;
		desiredUp = hitF.normal;
		newPos -= kart_trs->GetGlobalMatrix().WorldZ() * speed;
		speed = -speed / 4;
	}
	else if (!(hitF.object != nullptr && hitF.distance < DISTANCE_FROM_GROUND + 0.8) && (hitB.object != nullptr && hitB.distance < DISTANCE_FROM_GROUND + 0.8))
	{
		onTheGround = true;
		desiredUp = hitB.normal;
		newPos -= kart_trs->GetGlobalMatrix().WorldZ() * speed;
		speed = -speed / 4;
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
		Quat normal_rot = Quat::RotateFromTo(kart_trs->GetRotation().WorldY(), nextStep);
		kart_trs->Rotate(normal_rot);
	}


	//Manage Input to accelerate/brake. Returns acceleration
	if (onTheGround)
	{
		speed += AccelerationInput();
		speed = math::Clamp(speed, -maxSpeed, maxSpeed);
	}

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
		float rotateAngle = maxSteer * math::Clamp(speed / (maxSpeed / 3), -1.0f, 1.0f) * currentSteer * time->DeltaTime();
		Quat tmp = kart_trs->GetRotation().RotateAxisAngle(kartY, -rotateAngle * DEGTORAD);
		kart_trs->Rotate(tmp);

		fallSpeed = 0.0f;
	}
	else
	{
		fallSpeed += CAR_GRAVITY * time->DeltaTime();
		newPos.y -= fallSpeed;
	}
#pragma region Esthetic_rotation_chasis_wheels
	ComponentTransform* chasis_trs = (ComponentTransform*)chasis->GetComponent(C_TRANSFORM);
	chasis_trs->SetRotation(float3(0, -currentSteer * 15 * math::Clamp(speed / (maxSpeed / 3), -1.0f, 1.0f), 0));

	ComponentTransform* wheel_trs = (ComponentTransform*)frontWheel->GetComponent(C_TRANSFORM);
	wheel_trs->SetRotation(float3(0, -currentSteer * 15, 0));
#pragma endregion


	newPos += kart_trs->GetGlobalMatrix().WorldZ() * speed;
	kart_trs->SetPosition(newPos);
}

float ModuleCar::AccelerationInput()
{
	float acceleration = 0.0f;
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

void ModuleCar::Car_Debug_Ui()
{
	ImGui::SetNextWindowSize(ImVec2(350, 400));
	if (ImGui::Begin("Car_Debug"))
	{
		if (ImGui::Button("Set car cam"))
		{
			App->renderer3D->SetCamera(camera);
		}
		ImGui::SameLine();
		if (ImGui::Button("Set editor cam"))
		{
			App->renderer3D->SetCamera(App->camera->GetEditorCamera());
		}
		ImGui::NewLine();
		ImGui::Separator();

		ImGui::DragFloat("Max Speed", &maxSpeed, 0.1f, 0.1f, 20.0f);
		ImGui::DragFloat("Max Acceleration", &maxAcceleration, 0.01f, 0.01f, 20.0f);
		ImGui::DragFloat("Brake Power", &brakePower, 0.1f, 0.1f, 20.0f);
		ImGui::DragFloat("Maneuverability", &maneuverability, 0.1f, 0.1f, 20.0f);
		ImGui::DragFloat("Max Steer", &maxSteer, 1.0f, 0.0f, 300.0f);
		ImGui::DragFloat("Drag", &drag, 0.01f, 0.01f, 20.0f);
		ImGui::NewLine();
		ImGui::Separator();
		ImGui::Text("Just for display, do not touch");
		ImGui::DragFloat("Speed", &speed);
		ImGui::DragFloat("Current Steer", &currentSteer);
		ImGui::Checkbox("Steering", &steering);
		ImGui::NewLine();
		ImGui::Separator();
		ImGui::Text(tmpOutput);
		ImGui::End();
	}
}

void ModuleCar::FindKartGOs()
{
	kart = nullptr;
	chasis = nullptr;
	frontWheel = nullptr;
	backWheel = nullptr;
	cam = nullptr;	
	track = nullptr;
	kart_trs = nullptr;
	camera = nullptr;
	light = nullptr;

	const vector<GameObject*>* rootChilds = App->go_manager->root->GetChilds();
	for (vector<GameObject*>::const_iterator it = rootChilds->cbegin(); it != rootChilds->cend(); it++)
	{
		if ((*it)->name == "pivot")
		{
			kart = *it;
			kart_trs = (ComponentTransform*)kart->GetComponent(ComponentType::C_TRANSFORM);
			const vector<GameObject*>* kartChilds = kart->GetChilds();
			for (vector<GameObject*>::const_iterator it = kartChilds->cbegin(); it != kartChilds->cend(); it++)
			{
				if ((*it)->name == "chasis")
				{
					chasis = *it;
					const vector<GameObject*>* chasisChilds = chasis->GetChilds();
					for (vector<GameObject*>::const_iterator it = chasisChilds->cbegin(); it != chasisChilds->cend(); it++)
					{
						if ((*it)->name == "F_wheel") {	frontWheel = *it; }
						else if ((*it)->name == "B_wheel") { backWheel = *it; }
					}
				}
				else if ((*it)->name == "camera")
				{
					cam = *it;
					camera = (ComponentCamera*) cam->GetComponent(C_CAMERA);
				}
			}
		}

		if ((*it)->name == "track_test")
		{
			track = *it;
			track->layer = 20;
		}
		if ((*it)->name == "Directional_Light")
		{
			light = *it;
		}		
	}
}
