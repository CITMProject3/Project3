#include "stdafx.h"

#include <string>
#include <map>
#include <vector>
#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ModuleInput.h"
#include "../ModuleWindow.h"
#include "../GameObject.h"
#include "../ComponentScript.h"
#include "../ComponentTransform.h"
#include "../SDL/include/SDL_scancode.h"
#include "../PhysBody3D.h"
#include "../ComponentCollider.h"
#include "../Globals.h"
#include "../RaceTimer.h"
#include "../ComponentCar.h"
#include "../ComponentUiText.h"
#include "../ModuleGOManager.h"
#include "../Timer.h"

namespace Scene_Manager
{
	//Public variables
	GameObject* car_1_go = nullptr;
	ComponentCar* car_1 = nullptr;

	GameObject* car_2_go = nullptr;
	ComponentCar* car_2 = nullptr;

	GameObject* timer_text_go = nullptr;
	ComponentUiText* timer_text = nullptr;

	GameObject* lap_go = nullptr;
	ComponentUiText* lap_text = nullptr;

	GameObject* start_timer_go = nullptr;
	ComponentUiText* start_timer_text = nullptr;

	//"Private" variables
	Timer start_timer;
	RaceTimer timer;
	int race_timer_number = 3;

	void Scene_Manager_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("Car1", car_1_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Car2", car_2_go));

		public_gos->insert(std::pair<const char*, GameObject*>("Timer_Text", timer_text_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Lap_Text", lap_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Start_Timer_Text", start_timer_go));
	}

	void Scene_Manager_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		car_1_go = script->public_gos["Car1"];
		car_2_go = script->public_gos["Car2"];
		timer_text_go = script->public_gos["Timer_Text"];
		lap_go = script->public_gos["Lap_Text"];
		start_timer_go = script->public_gos["Start_Timer_Text"];
	}

	void Scene_Manager_SetStartTimerText(unsigned int number)
	{
		if (start_timer_text != nullptr)
		{
			start_timer_text->SetDisplayText(std::to_string(number));
		}
	}

	void Scene_Manager_StartRace()
	{
		start_timer.Stop();
		if (car_1 != nullptr)
			car_1->BlockInput(false);
		if (car_2 != nullptr)
			car_2->BlockInput(false);
		if (start_timer_text != nullptr)
			start_timer_text->GetGameObject()->SetActive(false);
	}

	void Scene_Manager_Start(GameObject* game_object)
	{
		race_timer_number = 3;
		Scene_Manager_UpdatePublics(game_object);
		start_timer.Start();
		//timer.Start();
		if (car_1_go != nullptr)
		{
			car_1 = (ComponentCar*)car_1_go->GetComponent(C_CAR);
			if (car_1)
				car_1->BlockInput(true);
		}
		if (car_2_go != nullptr)
		{
			car_2 = (ComponentCar*)car_2_go->GetComponent(C_CAR);
			if (car_2)
				car_2->BlockInput(true);
		}
		if (timer_text_go != nullptr)
		{
			timer_text = (ComponentUiText*)timer_text_go->GetComponent(C_UI_TEXT);
		}
		if (lap_go != nullptr)
		{
			lap_text = (ComponentUiText*)lap_go->GetComponent(C_UI_TEXT);
		}
		if (start_timer_go)
		{
			start_timer_text = (ComponentUiText*)start_timer_go->GetComponent(C_UI_TEXT);
		}
	}

	void Scene_Manager_Update(GameObject* game_object)
	{
		//"3, 2, 1, GO!"
		if (start_timer.IsRunning() == true)
		{
			if (start_timer.ReadSec() >= 1)
			{
				race_timer_number--;
				if (race_timer_number > 0)
				{
					start_timer.Start();
					Scene_Manager_SetStartTimerText(race_timer_number);
				}
				else
				{
					Scene_Manager_StartRace();
				}				
			}
		}

		//Updating car laps
		if (car_1 != nullptr)
		{
			if (car_1->lap + 1 != timer.GetCurrentLap())
			{
				timer.AddLap();
				//Update lap text
				if (lap_text != nullptr)
				{
					string str = std::to_string(car_1->lap);
					lap_text->SetDisplayText(str);
				}
			}
		}

		//Update UI race timer
		if (timer_text != nullptr)
		{
			int min, sec, milisec = 0;
			timer.GetCurrentLapTime(min, sec, milisec);
			string min_str = to_string(min);
			string sec_str = to_string(sec);
			string mil_str = to_string(milisec);
			if (min < 10)
				min_str = "0" + min_str;
			if (sec < 10)
				sec_str = "0" + sec_str;
			if (milisec < 100)
				mil_str = "0" + mil_str;

			string timer_string = min_str + ":" + sec_str + ":" + mil_str;
			timer_text->SetDisplayText(timer_string);
			LOG("Text set: %s", timer_string.c_str());
		}	
	}
}