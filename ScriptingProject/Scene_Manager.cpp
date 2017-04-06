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
#include "../ComponentUiImage.h"

#include "../ModuleGOManager.h"
#include "../Timer.h"
#include "../Time.h"

namespace Scene_Manager
{
	//Public variables
	GameObject* car_1_go = nullptr;
	ComponentCar* car_1 = nullptr;

	GameObject* car_2_go = nullptr;
	ComponentCar* car_2 = nullptr;

	GameObject* timer_text_go = nullptr;
	ComponentUiText* timer_text = nullptr;

	GameObject* lap1_go = nullptr;
	ComponentUiText* lap1_text = nullptr;

	GameObject* lap2_go = nullptr;
	ComponentUiText* lap2_text = nullptr;

	GameObject* start_timer_go = nullptr;
	ComponentUiText* start_timer_text = nullptr;

	GameObject* item_ui_1_go = nullptr;
	ComponentUiImage* item_ui_1 = nullptr;

	GameObject* item_ui_2_go = nullptr;
	ComponentUiImage* item_ui_2 = nullptr;

	GameObject* position_ui_1_go = nullptr;
	ComponentUiText* position_ui_1 = nullptr;

	GameObject* position_ui_2_go = nullptr;
	ComponentUiText* position_ui_2 = nullptr;

	GameObject* race_HUD = nullptr;

	//"Private" variables
	double start_timer;
	bool start_timer_on;

	RaceTimer timer;
	int race_timer_number = 3;

	void Scene_Manager_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("Car1", car_1_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Car2", car_2_go));

		public_gos->insert(std::pair<const char*, GameObject*>("Timer_Text", timer_text_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Lap_Player1_Text", lap1_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Lap_Player2_Text", lap2_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Start_Timer_Text", start_timer_go));

		public_gos->insert(std::pair<const char*, GameObject*>("Item_Player1", item_ui_1_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Item_Player2", item_ui_2_go));

		public_gos->insert(std::pair<const char*, GameObject*>("Item_Player1", item_ui_1_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Item_Player2", item_ui_2_go));

		public_gos->insert(std::pair<const char*, GameObject*>("Position_Player1", position_ui_1_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Position_Player2", position_ui_2_go));

		public_gos->insert(std::pair<const char*, GameObject*>("Race_HUD", race_HUD));

	}

	void Scene_Manager_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		car_1_go = script->public_gos["Car1"];
		car_2_go = script->public_gos["Car2"];
		timer_text_go = script->public_gos["Timer_Text"];
		lap1_go = script->public_gos["Lap_Player1_Text"];
		lap2_go = script->public_gos["Lap_Player2_Text"];
		start_timer_go = script->public_gos["Start_Timer_Text"];
		item_ui_1_go = script->public_gos["Item_Player1"];
		item_ui_2_go = script->public_gos["Item_Player2"];

		position_ui_1_go = script->public_gos["Position_Player1"];
		position_ui_2_go = script->public_gos["Position_Player2"];

		race_HUD = script->public_gos["Race_HUD"];
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
		if (car_1 != nullptr)
			car_1->BlockInput(false);
		if (car_2 != nullptr)
			car_2->BlockInput(false);
		if (car_1 == nullptr || car_2 == nullptr)
			LOG("Error: Could not find the cars in the scene!");

		timer.Start();
	}

	//WARNING: variables are only assigned in start: Two scripts in the same scene will cause problems
	void Scene_Manager_Start(GameObject* game_object)
	{
		race_timer_number = 4;
		Scene_Manager_UpdatePublics(game_object);
		start_timer = 0;
		start_timer_on = true;
		//timer.Start();
		if (car_1_go != nullptr)
		{
			car_1 = (ComponentCar*)car_1_go->GetComponent(C_CAR);
			if (car_1)
			{
				car_1->BlockInput(true);
				car_1->team = 0;
			}
		}
		if (car_2_go != nullptr)
		{
			car_2 = (ComponentCar*)car_2_go->GetComponent(C_CAR);
			if (car_2)
			{
				car_2->BlockInput(true);
				car_2->team = 1;
			}
		}
		if (timer_text_go != nullptr)
		{
			timer_text = (ComponentUiText*)timer_text_go->GetComponent(C_UI_TEXT);
		}
		if (lap1_go != nullptr)
		{
			lap1_text = (ComponentUiText*)lap1_go->GetComponent(C_UI_TEXT);
		}
		if (lap2_go != nullptr)
		{
			lap2_text = (ComponentUiText*)lap2_go->GetComponent(C_UI_TEXT);
		}
		if (start_timer_go)
		{
			start_timer_text = (ComponentUiText*)start_timer_go->GetComponent(C_UI_TEXT);
		}
		if (item_ui_1_go)
		{
			item_ui_1 = (ComponentUiImage*)item_ui_1_go->GetComponent(C_UI_IMAGE);
		}
		if (item_ui_2_go)
		{
			item_ui_2 = (ComponentUiImage*)item_ui_2_go->GetComponent(C_UI_IMAGE);
		}
		if (position_ui_1_go)
		{
			position_ui_1 = (ComponentUiText*)position_ui_1_go->GetComponent(C_UI_IMAGE);
		}
		if (position_ui_2_go)
		{
			position_ui_2 = (ComponentUiText*)position_ui_2_go->GetComponent(C_UI_IMAGE);
		}
	}

	void Scene_Manager_Update(GameObject* game_object)
	{
		//"3, 2, 1, GO!"
		if (start_timer_on == true)
		{
			start_timer += time->DeltaTime();
			if (start_timer >= 1)
			{
				race_timer_number--;
				if (race_timer_number > 0)
				{
					start_timer = 0;
					Scene_Manager_SetStartTimerText(race_timer_number - 1);
				}
				else
				{
					start_timer_on = false;
					if (start_timer_text) start_timer_text->GetGameObject()->SetActive(false);
					if (race_HUD != nullptr)
					{
						race_HUD->SetActive(true);
						if (item_ui_1) item_ui_1->GetGameObject()->SetActive(false);
						if (item_ui_2) item_ui_2->GetGameObject()->SetActive(false);
					}
				}
				if (race_timer_number == 1)
				{
					Scene_Manager_StartRace();
				}				
			}
		}

		if (race_timer_number == 0)
		{
			timer.Update(time->DeltaTime());
		}

		//Updating invidual HUD
		if (car_1 != nullptr)
		{
			//Update lap counter
			if (car_1->lap + 1 != timer.GetCurrentLap(0))
			{
				timer.AddLap(0);
				//Update current lap text
				if (lap1_text != nullptr)
				{
					string str = std::to_string(car_1->lap);
					lap1_text->SetDisplayText(str);
				}
			}
			//Update first//second position
			if (position_ui_1 != nullptr && std::to_string(car_1->place) != position_ui_1->GetText())
			{
				position_ui_1->SetText(std::to_string(car_1->place));
			}
		}

		//Updating invidual HUD
		if (car_2 != nullptr)
		{
			//Update lap counter
			if (car_2->lap + 1 != timer.GetCurrentLap(1))
			{
				timer.AddLap(1);
				//Update current lap text
				if (lap2_text != nullptr)
				{
					string str = std::to_string(car_2->lap);
					lap2_text->SetDisplayText(str);
				}
			}
			//Update first//second position
			if (position_ui_2 != nullptr && std::to_string(car_2->place) != position_ui_2->GetText())
			{
				position_ui_2->SetText(std::to_string(car_1->place));
			}
		}

		//Update UI race timer
		if (timer_text != nullptr)
		{
			int min, sec, milisec = 0;
			timer.GetRaceTime(min, sec, milisec);
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

	void Scene_Manager_UpdateItems(unsigned int team, bool has_item)
	{
		if (team == 0 && car_1 != nullptr)
		{
			if (item_ui_1 != nullptr)
			{
				item_ui_1->GetGameObject()->SetActive(has_item);
			}
		}
		else if (team == 1 && car_2 != nullptr)
		{
			if (item_ui_2 != nullptr)
			{
				item_ui_2->GetGameObject()->SetActive(has_item);
			}
		}
	}
}