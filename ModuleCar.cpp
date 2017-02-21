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
#pragma region loading_the_kart
		AssetFile* kartFile = App->editor->assets->FindAssetFile("/Assets/kart.fbx");
		if (kartFile != nullptr)
		{
			App->resource_manager->LoadFile(kartFile->content_path, FileType::MESH);

			const vector<GameObject*>* rootChilds = App->go_manager->root->GetChilds();
			for (vector<GameObject*>::const_iterator it = rootChilds->cbegin(); it != rootChilds->cend(); it++)
			{
				if ((*it)->name == "pivot")
				{
					kart = *it;
				}
			}
		}

		if (kart != nullptr)
		{
			kart_trs = (ComponentTransform*)kart->GetComponent(ComponentType::C_TRANSFORM);

			const vector<GameObject*>* childs = kart->GetChilds();
			for (vector<GameObject*>::const_iterator it = childs->cbegin(); it != childs->cend(); it++)
			{
				if ((*it)->name == "chasis")
				{
					chasis = *it;
				}
				else if ((*it)->name == "camera")
				{
					cam = *it;
				}
			}
		}

		if (chasis != nullptr)
		{
			const vector<GameObject*>* childs = chasis->GetChilds();
			for (vector<GameObject*>::const_iterator it = childs->cbegin(); it != childs->cend(); it++)
			{
				if ((*it)->name == "F_wheel")
				{
					frontWheel = *it;
				}
				else if ((*it)->name == "B_wheel")
				{
					backWheel = *it;
				}
			}
		}
#pragma endregion

#pragma region loading_the_test_track
		AssetFile* trackFile = App->editor->assets->FindAssetFile("/Assets/track_test.fbx");
		if (trackFile != nullptr)
		{
			App->resource_manager->LoadFile(trackFile->content_path, FileType::MESH);

			const vector<GameObject*>* rootChilds = App->go_manager->root->GetChilds();
			for (vector<GameObject*>::const_iterator it = rootChilds->cbegin(); it != rootChilds->cend(); it++)
			{
				if ((*it)->name == "track_test")
				{
					track = *it;
				}
			}
		}
#pragma endregion

		GameObject* light = App->go_manager->CreateGameObject(NULL);
		light->AddComponent(ComponentType::C_LIGHT);
		ComponentTransform* tmp = (ComponentTransform*) light->GetComponent(C_TRANSFORM);
		tmp->SetRotation(float3(50, -10, -50));

		cam->AddComponent(C_CAMERA);

		loaded = true;
	}

	return UPDATE_CONTINUE;
}

update_status ModuleCar::Update()
{
	if (App->IsGameRunning())
	{
		Car_Debug_Ui();

		if (kart && kart_trs)
		{
			/*math::Ray ray;
			//ray.dir = -kart_trs->GetGlobalMatrix().WorldY();
			ray.dir = float3(0, -1, 0);
			ray.pos = kart_trs->GetPosition();
			ray.pos -= kart_trs->GetGlobalMatrix().WorldY();

			RaycastHit hit;
			App->go_manager->Raycast(ray);

			if (hit.object != nullptr && hit.distance < 5)
			{
				LOG("\nHit object: %s\nNormal: %f, %f, %f", hit.object->name.data(), hit.point.x, hit.point.y, hit.point.z);
				Quat normal_rot = Quat::RotateFromTo(kart_trs->GetRotation().WorldY(), hit.normal);
				//kart_trs->SetRotation(kart_trs->GetRotation() * normal_rot);
			}
			else
			{
				if (hit.distance > 5) { LOG("\nToo far for hit"); }
				else { LOG("\nHit no object"); }
				Quat normal_rot = Quat::RotateFromTo(kart_trs->GetRotation().WorldY(), float3(0, 1, 0));
				//kart_trs->SetRotation(kart_trs->GetRotation() * normal_rot);
			}*/

			float3 pos = kart_trs->GetPosition();
			float3 newPos = pos;
			float acceleration = 0.0f;

			if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
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
			else if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
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

			speed += acceleration;
			speed = math::Clamp(speed, -maxSpeed, maxSpeed);


			bool steering = false;
			if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
			{
				steering = true;
				currentSteer += maneuverability * time->DeltaTime();
			}

			if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
			{
				currentSteer -= maneuverability * time->DeltaTime();
				steering = true;
			}
			currentSteer = math::Clamp(currentSteer, -1.0f, 1.0f);

			if (steering == false)
			{
				if (currentSteer > maneuverability * time->DeltaTime())
				{
					currentSteer -= maneuverability * time->DeltaTime();
				}
				else if (currentSteer < -maneuverability * time->DeltaTime())
				{
					currentSteer += maneuverability * time->DeltaTime();
				}
				else
				{
					currentSteer = 0;
				}
			}


			float rotateAngle = maxSteer * math::Clamp(speed / (maxSpeed / 3), -1.0f, 1.0f) * currentSteer * time->DeltaTime();
			Quat tmp = kart_trs->GetRotation().RotateAxisAngle(float3(0, 1, 0), -rotateAngle * DEGTORAD);
			kart_trs->SetRotation(kart_trs->GetRotation() * tmp);

			ComponentTransform* chasis_trs = (ComponentTransform*) chasis->GetComponent(C_TRANSFORM);
			chasis_trs->SetRotation(float3(0, -currentSteer * 15 * math::Clamp(speed / (maxSpeed / 3), -1.0f, 1.0f), 0));

			ComponentTransform* wheel_trs = (ComponentTransform*)frontWheel->GetComponent(C_TRANSFORM);
			wheel_trs->SetRotation(float3(0, -currentSteer * 15, 0));


			newPos += kart_trs->GetGlobalMatrix().WorldZ() * speed;

			kart_trs->SetPosition(newPos);
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

void ModuleCar::Car_Debug_Ui()
{
	ImGui::SetNextWindowSize(ImVec2(350, 400));
	if (ImGui::Begin("Car_Debug"))
	{
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
		ImGui::End();
	}
}