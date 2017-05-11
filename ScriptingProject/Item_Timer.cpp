#include "stdafx.h"
#include <string>
#include <map>
#include "../ModuleScripting.h"
#include "../GameObject.h"
#include "../ComponentScript.h"
#include "../ComponentCollider.h"
#include "../ComponentCar.h"
#include "../ComponentMesh.h"
#include "../ComponentParticleSystem.h"
#include "../Time.h"
#include "../Globals.h"
#include "../PhysBody3D.h"

bool isHitodama = false;
float timer = 0.0f;
float max_time = 0.0f;
ComponentCollider* go_col = nullptr;
ComponentMesh* go_mesh = nullptr;
ComponentParticleSystem* go_part = nullptr;
bool taken = false;

void Item_Timer_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
{
	public_float->insert(pair<const char*, float>("max_time",max_time));
	public_bools->insert(pair<const char*, bool>("isHitodama", isHitodama));
	public_bools->insert(pair<const char*, bool>("taken", taken));
}

void Item_Timer_UpdatePublics(GameObject* game_object)
{
	ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
	go_col = (ComponentCollider*)game_object->GetComponent(C_COLLIDER);
	go_part = (ComponentParticleSystem*)game_object->GetComponent(C_PARTICLE_SYSTEM);
	go_mesh = (ComponentMesh*)game_object->GetComponent(C_MESH);
	isHitodama = script->public_bools.at("isHitodama");
	taken = script->public_bools.at("taken");
	max_time = script->public_floats.at("max_time");
}

void Item_Timer_ActualizePublics(GameObject* game_object)
{
	ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
	script->public_bools.at("taken") = taken;
	
}

void Item_Timer_Start(GameObject* game_object)
{
	go_col = (ComponentCollider*)game_object->GetComponent(C_COLLIDER);
	if (isHitodama)
	{
		go_mesh = (ComponentMesh*)game_object->GetComponent(C_MESH);
	}
	else
	{
		go_mesh = (ComponentMesh*)game_object->GetComponent(C_MESH);
	}
	
	taken = false;
}

void Item_Timer_Update(GameObject* game_object)
{
	Item_Timer_UpdatePublics(game_object);
	if (taken)
	{
		if (isHitodama)
		{
			timer += time->DeltaTime();
			if (timer >= max_time)
			{
				go_col->SetActive(true);
				//Particle sytem here no mesh
				go_mesh->SetActive(true);
				taken = false;
				timer = 0.0f;
			}
		}
		else
		{
			timer += time->DeltaTime();
			if (timer >= max_time)
			{
				go_col->SetActive(true);
				go_mesh->SetActive(true);
				taken = false;
				timer = 0.0f;
			}
		}
	}
}

void Item_Timer_OnCollision(GameObject* game_object, PhysBody3D* col)
{
	ComponentCar* car = col->GetCar();
	ComponentTransform* trs = (ComponentTransform*)game_object->GetComponent(C_TRANSFORM);
	if (car && trs && taken == false)
	{
		Item_Timer_UpdatePublics(game_object);
		if (go_col->IsActive())
		{
			if (isHitodama)
			{
 				taken = car->AddHitodama();
				if (taken)
				{
					go_col->SetActive(false);
					//Particle sytem here no mesh
					go_mesh->SetActive(false);
				}
			}
			else
			{
				taken = true;
				//Pep do your magic
				if (taken)
				{
					go_col->SetActive(false);
					go_mesh->SetActive(false);
				}
			}
		}
	}
}