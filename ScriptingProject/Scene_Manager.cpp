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
#include "../ComponentAudioSource.h"
#include "../ComponentUiButton.h"

#include "../ModuleGOManager.h"
#include "../ModuleResourceManager.h"

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

	GameObject* start_timer_go2 = nullptr;
	ComponentUiText* start_timer_text2 = nullptr;

	GameObject* item_ui_1_go = nullptr;
	ComponentUiImage* item_ui_1 = nullptr;

	GameObject* item_ui_2_go = nullptr;
	ComponentUiImage* item_ui_2 = nullptr;

	GameObject* position_ui_1_go = nullptr;
	ComponentUiText* position_ui_1 = nullptr;

	GameObject* position_ui_2_go = nullptr;
	ComponentUiText* position_ui_2 = nullptr;

	GameObject* race_HUD = nullptr;

	GameObject* result_window = nullptr;

	GameObject* player1_finish = nullptr;
	GameObject* player2_finish = nullptr;

	GameObject* text_1_finish = nullptr;
	ComponentUiText* timer_1_text = nullptr;

	GameObject* text_2_finish = nullptr;
	ComponentUiText* timer_2_text = nullptr;

	GameObject* win1_finish = nullptr;
	ComponentUiButton* win1_button = nullptr;
	GameObject* win2_finish = nullptr;
	ComponentUiButton* win2_button = nullptr;

	string main_menu_scene = "/Assets/Main_menu.ezx";

	//"Private" variables
	double start_timer;
	bool start_timer_on;

	RaceTimer timer;
	int race_timer_number = 4;
	bool race_finished = false;

	double finish_timer = 0;
	bool finish_timer_on = false;
	bool team1_finished = false;
	bool team2_finished = false;
	// Bool for music playing
	bool music_played = false;
	string team1_text = "";
	string team2_text = "";

	void Scene_Manager_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_chars->insert(std::pair<const char*, string>("Main_Menu_Scene", main_menu_scene));

		public_gos->insert(std::pair<const char*, GameObject*>("Car1", car_1_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Car2", car_2_go));

		public_gos->insert(std::pair<const char*, GameObject*>("Timer_Text", timer_text_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Lap_Player1_Text", lap1_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Lap_Player2_Text", lap2_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Start_Timer_Text", start_timer_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Start_Timer_Text2", start_timer_go2));

		public_gos->insert(std::pair<const char*, GameObject*>("Item_Player1", item_ui_1_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Item_Player2", item_ui_2_go));

		public_gos->insert(std::pair<const char*, GameObject*>("Item_Player1", item_ui_1_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Item_Player2", item_ui_2_go));

		public_gos->insert(std::pair<const char*, GameObject*>("Position_Player1", position_ui_1_go));
		public_gos->insert(std::pair<const char*, GameObject*>("Position_Player2", position_ui_2_go));

		public_gos->insert(std::pair<const char*, GameObject*>("Race_HUD", race_HUD));
		public_gos->insert(std::pair<const char*, GameObject*>("Result_Window", result_window));
		public_gos->insert(std::pair<const char*, GameObject*>("Player1_Finish_Text", player1_finish));
		public_gos->insert(std::pair<const char*, GameObject*>("Player2_Finish_Text", player2_finish));

		public_gos->insert(std::pair<const char*, GameObject*>("Team1_timer_Text", text_1_finish));
		public_gos->insert(std::pair<const char*, GameObject*>("Team2_timer_Text", text_2_finish));

		public_gos->insert(std::pair<const char*, GameObject*>("Win_team1_button", win1_finish));
		public_gos->insert(std::pair<const char*, GameObject*>("Win_team2_button", win2_finish));
	}

	void Scene_Manager_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		main_menu_scene.copy(script->public_chars["Main_Menu_Scene"]._Myptr(), script->public_chars["Main_Menu_Scene"].size());

		car_1_go = script->public_gos["Car1"];
		car_2_go = script->public_gos["Car2"];
		timer_text_go = script->public_gos["Timer_Text"];
		lap1_go = script->public_gos["Lap_Player1_Text"];
		lap2_go = script->public_gos["Lap_Player2_Text"];
		start_timer_go = script->public_gos["Start_Timer_Text"];
		start_timer_go2 = script->public_gos["Start_Timer_Text2"];
		item_ui_1_go = script->public_gos["Item_Player1"];
		item_ui_2_go = script->public_gos["Item_Player2"];

		position_ui_1_go = script->public_gos["Position_Player1"];
		position_ui_2_go = script->public_gos["Position_Player2"];

		race_HUD = script->public_gos["Race_HUD"];

		player1_finish = script->public_gos["Player1_Finish_Text"];
		player2_finish = script->public_gos["Player2_Finish_Text"];
		text_1_finish = script->public_gos["Team1_timer_Text"];
		text_2_finish = script->public_gos["Team2_timer_Text"];
		win1_finish = script->public_gos["Win_team1_button"];
		win2_finish = script->public_gos["Win_team2_button"];
		result_window = script->public_gos["Result_Window"];
	}

	void Scene_Manager_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		script->public_chars.at("Main_Menu_Scene") = main_menu_scene;
	}

	//Call for 3 2 1 audio in this function. When number is 0, "GO" is displayed
	void Scene_Manager_SetStartTimerText(unsigned int number, GameObject* game_object)
	{
		ComponentAudioSource* a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);

		if (a_comp)
		{
			switch (number)
			{
			case(2): a_comp->PlayAudio(2); break;
			case(1): a_comp->PlayAudio(1); break;
			case(0): a_comp->PlayAudio(0); break;
			}
		}

		if (start_timer_text != nullptr)
		{
			start_timer_text->SetDisplayText(std::to_string(number));
			start_timer_text2->SetDisplayText(std::to_string(number));
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
		music_played = false;
		ComponentAudioSource* a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
		if (a_comp)	a_comp->PlayAudio(3);

		race_timer_number = 4;
		finish_timer = 0;
		finish_timer_on = false;
		Scene_Manager_UpdatePublics(game_object);
		start_timer = 0;
		start_timer_on = true;
		race_finished = false;
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
		if (start_timer_go2)
		{
			start_timer_text2 = (ComponentUiText*)start_timer_go2->GetComponent(C_UI_TEXT);
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
			position_ui_1 = (ComponentUiText*)position_ui_1_go->GetComponent(C_UI_TEXT);
		}
		if (position_ui_2_go)
		{
			position_ui_2 = (ComponentUiText*)position_ui_2_go->GetComponent(C_UI_TEXT);
		}

		if (text_1_finish)
		{
			timer_1_text = (ComponentUiText*)text_1_finish->GetComponent(C_UI_TEXT);
		}

		if (text_2_finish)
		{
			timer_2_text = (ComponentUiText*)text_2_finish->GetComponent(C_UI_TEXT);
		}

		if (win1_finish)
		{
			win1_button = (ComponentUiButton*)win1_finish->GetComponent(C_UI_BUTTON);
		}

		if (win2_finish)
		{
			win2_button = (ComponentUiButton*)win2_finish->GetComponent(C_UI_BUTTON);
		}
	}

	void Scene_Manager_UpdateDuringRace(GameObject* game_object);

	void Scene_Manager_Update(GameObject* game_object)
	{
		if (!music_played)
		{
			ComponentAudioSource* a_comp = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
			if (a_comp)	a_comp->PlayAudio(4);
			music_played = true;
		}

		if (race_finished == false)
		{
			Scene_Manager_UpdateDuringRace(game_object);
		}
		else
		{
			for (uint joystick = 0; joystick < 4; joystick++)
			{
				if (App->input->GetJoystickButton(joystick, JOY_BUTTON::A) == KEY_DOWN || App->input->GetJoystickButton(joystick, JOY_BUTTON::A) || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
				{
					App->LoadScene(main_menu_scene.c_str());
					return;
				}
			}
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

	void Scene_Manager_FinishRace()
	{
		race_finished = true;
		if (race_HUD) race_HUD->SetActive(false);
		if (result_window) result_window->SetActive(true);
		if (timer_1_text)timer_1_text->SetDisplayText(team1_text);
		if (timer_2_text)timer_2_text->SetDisplayText(team2_text);
	}

	void Scene_Manager_UpdateUIRaceTimer()
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

	void Scene_Manager_UpdateStartCountDown(GameObject* game_object)
	{
		start_timer += time->DeltaTime();
		if (start_timer >= 1)
		{
			race_timer_number--;
			if (race_timer_number > 0)
			{
				start_timer = 0;
				Scene_Manager_SetStartTimerText(race_timer_number - 1, game_object);
			}
			else
			{
				start_timer_on = false;
				if (start_timer_text) start_timer_text->GetGameObject()->SetActive(false);
				if (start_timer_text2) start_timer_text2->GetGameObject()->SetActive(false);
				if (race_HUD != nullptr)
				{
					race_HUD->SetActive(true);
					if (item_ui_1) item_ui_1->GetGameObject()->SetActive(false);
					if (item_ui_2) item_ui_2->GetGameObject()->SetActive(false);
					if (player1_finish) player1_finish->SetActive(false);
					if (player2_finish) player2_finish->SetActive(false);
				}
			}
			if (race_timer_number == 1)
			{
				Scene_Manager_StartRace();
			}
		}

	}

	void Scene_Manager_UpdateDuringRace(GameObject* game_object)
	{
		if (finish_timer_on == true)
		{
			finish_timer += time->DeltaTime();
			if (finish_timer >= 3)
			{
				Scene_Manager_FinishRace();
			}
		}
		else
		{
			if (start_timer_on == true)
			{
				Scene_Manager_UpdateStartCountDown(game_object);
			}

			if (race_timer_number == 0)
			{
				timer.Update(time->DeltaTime());
			}

			//Updating invidual HUD
			if (car_1 != nullptr)
			{
				//Update lap counter
				if (car_1->lap + 1 > timer.GetCurrentLap(0))
				{
					if (car_1->finished == true)
					{
						if (team1_finished == false && timer_1_text != nullptr && timer_text != nullptr)
						{
							team1_text = timer_text->GetText();
							if (team2_finished == false && win2_button != nullptr)
							{
								win2_button->OnPress();
							}
							team1_finished = true;
						}

						if (player1_finish) player1_finish->SetActive(true);
						if (car_2 && car_2->finished == true)
						{
							finish_timer_on = true;
							return;
						}
					}
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
					position_ui_1->SetDisplayText(std::to_string(car_1->place));
				}
			}

			//Updating invidual HUD
			if (car_2 != nullptr)
			{
				//Update lap counter
				if (car_2->lap + 1 > timer.GetCurrentLap(1))
				{
					if (car_2->finished == true)
					{
						if (team2_finished == false && timer_2_text != nullptr && timer_text != nullptr)
						{

							team2_text = timer_text->GetText();
							if (team1_finished == false && win1_button != nullptr)
							{
								win1_button->OnPress();
							}
							team2_finished = true;
						}

						if (player2_finish) player2_finish->SetActive(true);
						if (car_1 && car_1->finished == true)
						{
							finish_timer_on = true;
							return;
						}
					}
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
					position_ui_2->SetDisplayText(std::to_string(car_2->place));
				}
			}

			//Update UI race timer
			if (timer_text != nullptr)
			{
				Scene_Manager_UpdateUIRaceTimer();
			}
		}
	}
}