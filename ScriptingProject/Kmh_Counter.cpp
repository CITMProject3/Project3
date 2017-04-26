#include "stdafx.h"
#include <string>
#include <map>
#include "../ModuleScripting.h"
#include "../GameObject.h"
#include "../ComponentScript.h"
#include "../ComponentCar.h"
#include "../ComponentUiText.h"
#include "../Globals.h"


GameObject* car_go = nullptr;
ComponentCar* car = nullptr;
ComponentUiText* text = nullptr;

void Kmh_Counter_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
{
	public_gos->insert(pair<const char*, GameObject*>("Car", car_go));
}

void Kmh_Counter_UpdatePublics(GameObject* game_object)
{
	ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
	car_go = script->public_gos["Car"];
}

void Kmh_Counter_Start(GameObject* game_object)
{
	if (car_go != nullptr)
	{
		car = (ComponentCar*)car_go->GetComponent(C_CAR);
	}
	text = (ComponentUiText*)game_object->GetComponent(C_UI_TEXT);
}

void Kmh_Counter_Update(GameObject* game_object)
{
	if (car != nullptr)
	{
		int vel = car->GetVelocity();
		LOG("Scripting vel %i", vel);
		string str;
		if (vel < 10 && vel != 0)
			str = "0" + to_string(vel) + "k";
		else
			str = to_string(vel) + "k";
		text->SetDisplayText(str);
	}
}

void Kmh_Counter_ActualizePublics(GameObject* game_object)
{
	/*
	ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

	test_script->public_chars.at("Title") = test_title;
	test_script->public_gos.at("Test_go") = test_go;
	*/
}