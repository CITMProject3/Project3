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
#include "../ComponentMaterial.h"
#include "../ComponentAudioSource.h"
#include "../ComponentUiButton.h"
#include "../ComponentRectTransform.h"

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

	GameObject* topdriver_number = nullptr;
	GameObject* topgunner_number = nullptr;
	GameObject* botdriver_number = nullptr;
	GameObject* botgunner_number = nullptr;
	ComponentMaterial* td_number = nullptr;
	ComponentMaterial* tg_number = nullptr;
	ComponentMaterial* bd_number = nullptr;
	ComponentMaterial* bg_number = nullptr;

	GameObject* focus_result;
	ComponentAudioSource *audio_source = nullptr;

	GameObject* top_wrongdirection = nullptr;
	GameObject* bot_wrongdirection = nullptr;
	GameObject* credits = nullptr;
	
	string assets_main_menu_scene = "/Assets/Main_menu.ezx"; // On Assets
	string library_main_menu_scene = "/Library/3680778901/3680778901.ezx"; // On Library

	string assets_scene_1_reload = "/Assets/Scene_Map_1/Map_1_rematch.ezx"; // On Assets
	string library_scene_1_reload = "/Library/1441726200/554669074/554669074.ezx"; // On Library

	string assets_scene_2_reload = "/Assets/Scene_Map_2_NEW/Map_2_rematch.ezx"; // On Assets
	string library_scene_2_reload = "/Library/2159977326/1069121338/1069121338.ezx"; // On Library

	//"Private" variables
	float delay_to_start;
	double number_pos_timer;
	double start_timer;
	bool start_timer_on = false;
	uint countdown_sound = 4;

	float wrong_dir_timer_1;
	float wrong_dir_timer_2;

	RaceTimer timer;
	int race_timer_number = 4;
	bool race_finished = false;

	// Disqualification
	int goingToDisqualify = 0;
	double second_position_timer = 0;

	double finish_timer = 0;
	bool finish_timer_on = false;
	bool team1_finished = false;
	bool team2_finished = false;
	// Bool for music playing
	bool music_played = false;
	string team1_text = "";
	string team2_text = "";

	//Result Window

	int result_focus_pos = 0;

	bool triggers_pressed[4];
	bool quit = false;
	float quit_timer = 0;
	float quit_max_timer = 5;

	void Scene_Manager_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_chars->insert(std::pair<const char*, string>("Main_Menu_Scene", assets_main_menu_scene));

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

		public_gos->insert(std::pair<const char*, GameObject*>("TopDriver_Number", topdriver_number));
		public_gos->insert(std::pair<const char*, GameObject*>("TopGunner_Number", topgunner_number));
		public_gos->insert(std::pair<const char*, GameObject*>("BotDriver_Number", botdriver_number));
		public_gos->insert(std::pair<const char*, GameObject*>("BotGunner_Number", botgunner_number));

		public_gos->insert(std::pair<const char*, GameObject*>("Top_Wrong", top_wrongdirection));
		public_gos->insert(std::pair<const char*, GameObject*>("Bot_Wrong", bot_wrongdirection));
		public_gos->insert(std::pair<const char*, GameObject*>("Focus_result", focus_result));
		public_gos->insert(std::pair<const char*, GameObject*>("credits", credits));
	}

	void Scene_Manager_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		assets_main_menu_scene.copy(script->public_chars["Main_Menu_Scene"]._Myptr(), script->public_chars["Main_Menu_Scene"].size());

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

		topdriver_number = script->public_gos["TopDriver_Number"];
		topgunner_number = script->public_gos["TopGunner_Number"];
		botdriver_number = script->public_gos["BotDriver_Number"];
		botgunner_number = script->public_gos["BotGunner_Number"];

		top_wrongdirection = script->public_gos["Top_Wrong"];
		bot_wrongdirection = script->public_gos["Bot_Wrong"];

		focus_result = script->public_gos["Focus_result"];
		credits = script->public_gos["credits"];
	}

	void Scene_Manager_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);
		script->public_chars.at("Main_Menu_Scene") = assets_main_menu_scene;
	}

	//Call for 3 2 1 audio in this function. When number is 0, "GO" is displayed
	void Scene_Manager_SetStartTimerText(unsigned int number, GameObject* game_object)
	{		
		if (start_timer_text != nullptr && start_timer_text2 != nullptr)
		{
			start_timer_text->SetDisplayText(std::to_string(number));
			start_timer_text2->SetDisplayText(std::to_string(number));
		}
	}

	void Scene_Manager_StartRace(GameObject* game_object)
	{
		if (car_1 != nullptr)
			car_1->BlockInput(false);
		if (car_2 != nullptr)
			car_2->BlockInput(false);
		if (car_1 == nullptr || car_2 == nullptr)
			LOG("Error: Could not find the cars in the scene!");

		if (audio_source) audio_source->PlayAudio(2);  // GO

		timer.Start();
	}

	//WARNING: variables are only assigned in start: Two scripts in the same scene will cause problems
	void Scene_Manager_Start(GameObject* game_object)
	{
		quit = false;
		result_focus_pos = 0;
		music_played = false;
		race_timer_number = 4;
		countdown_sound = 3;
		finish_timer = 0;
		second_position_timer = 0;
		goingToDisqualify = 0;
		wrong_dir_timer_1 = 0.0f;
		wrong_dir_timer_2 = 0.0f;
		finish_timer_on = false;
		Scene_Manager_UpdatePublics(game_object);
		start_timer = 0;
		if (topdriver_number)
			delay_to_start = 0.0f;
		else
			delay_to_start = 4.0f;
		start_timer_on = false;
		race_finished = false;
		team1_finished = false;
		team2_finished = false;
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
			if (start_timer_text) start_timer_text->GetGameObject()->SetActive(false);
		}
		if (start_timer_go2)
		{
			start_timer_text2 = (ComponentUiText*)start_timer_go2->GetComponent(C_UI_TEXT);
			if(start_timer_text2) start_timer_text2->GetGameObject()->SetActive(false);
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
		if (credits)credits->SetActive(false);
		if (topdriver_number && topgunner_number && botdriver_number && botgunner_number) //
		{
			number_pos_timer = 1.0f;

			td_number = ((ComponentUiImage*)topdriver_number->GetComponent(C_UI_IMAGE))->UImaterial;
			tg_number = ((ComponentUiImage*)topgunner_number->GetComponent(C_UI_IMAGE))->UImaterial;
			bd_number = ((ComponentUiImage*)botdriver_number->GetComponent(C_UI_IMAGE))->UImaterial;
			bg_number = ((ComponentUiImage*)botgunner_number->GetComponent(C_UI_IMAGE))->UImaterial;

			topdriver_number->SetActive(true);
			topgunner_number->SetActive(true);
			botdriver_number->SetActive(true);
			botgunner_number->SetActive(true);

			td_number->SetIdToRender(App->go_manager->team1_front);

			tg_number->SetIdToRender(App->go_manager->team1_back);

			bd_number->SetIdToRender(App->go_manager->team2_front);

			bg_number->SetIdToRender(App->go_manager->team2_back);
		}

		if (top_wrongdirection && bot_wrongdirection)
		{
			top_wrongdirection->SetActive(false);
			bot_wrongdirection->SetActive(false);
		}
		if (result_window) result_window->SetActive(false);
		audio_source = (ComponentAudioSource*)game_object->GetComponent(ComponentType::C_AUDIO_SOURCE);
	}

	void Scene_Manager_UpdateDuringRace(GameObject* game_object);
	void Scene_Manager_ResultWindow(GameObject* game_object);

	void Scene_Manager_Update(GameObject* game_object)
	{
		if (!music_played)
		{
			if (audio_source) audio_source->PlayAudio(0);   // Playing Music
			music_played = true;
		}

		if (race_finished == false)
		{
			Scene_Manager_UpdateDuringRace(game_object);
		}
		else
		{
			Scene_Manager_ResultWindow(game_object);
		}
	}

	void Scene_Manager_UpdateItems(unsigned int team, int item_id, int item_size)
	{
		//Choosing image to display
		bool active = (item_id != -1);
		int image_to_display = 0;

		if (active == true)
		{
			if (item_id > 0)
			{
				image_to_display = item_id + 2;
			}
			else
			{
				image_to_display = item_size - 1;
			}
		}

		if (team == 0 && car_1 != nullptr)
		{
			if (item_ui_1 != nullptr)
			{
				item_ui_1->GetGameObject()->SetActive(active);
				if (active == true)
				{
					item_ui_1->UImaterial->SetIdToRender(image_to_display);
				}
			}
		}
		else if (team == 1 && car_2 != nullptr)
		{
			if (item_ui_2 != nullptr)
			{
				item_ui_2->GetGameObject()->SetActive(active);
				if (active == true)
				{
					item_ui_2->UImaterial->SetIdToRender(image_to_display);
				}
			}
		}
	}

	void Scene_Manager_FinishRace()
	{
		if (audio_source) audio_source->PlayAudio(6);  // Mutting music and lowering audio

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
		//LOG("Text set: %s", timer_string.c_str());
	}

	void Scene_Manager_UpdateStartCountDown(GameObject* game_object)
	{
		// Sound management
		if (countdown_sound != race_timer_number && countdown_sound != 2) 
		{
			if (audio_source) audio_source->PlayAudio(1); // Countdowns
			countdown_sound = race_timer_number;
		}

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
				Scene_Manager_StartRace(game_object);
			}
		}

	}

	void Scene_Manager_MoveElementsUI(ComponentRectTransform* go, double timeLeft, float destination, float margin)
	{
		float distance = destination - go->GetLocalPos().y;

		double relation = timeLeft / time->DeltaTime();
		
		float fMovement = distance / relation;

		if ((distance >= margin && distance > 0) || (-distance >= margin && distance < 0))
			go->Move(float3(0, fMovement, 0));
	}

	void Scene_Manager_UpdateDuringRace(GameObject* game_object)
	{
		if (goingToDisqualify != 0)
		{
			second_position_timer += time->DeltaTime();
		}

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
			if (race_timer_number != 0) // When race_timer_number is 1, the race has begun!
			{
				if (delay_to_start > 2.0f && delay_to_start < 3.0f)
				{
					Scene_Manager_MoveElementsUI((ComponentRectTransform*)topdriver_number->GetComponent(C_RECT_TRANSFORM), number_pos_timer, -500.0f, 1);
					Scene_Manager_MoveElementsUI((ComponentRectTransform*)topgunner_number->GetComponent(C_RECT_TRANSFORM), number_pos_timer, -500.0f, 1);
					Scene_Manager_MoveElementsUI((ComponentRectTransform*)botdriver_number->GetComponent(C_RECT_TRANSFORM), number_pos_timer, 1220.0f, 1);
					Scene_Manager_MoveElementsUI((ComponentRectTransform*)botgunner_number->GetComponent(C_RECT_TRANSFORM), number_pos_timer, 1220.0f, 1);
				}
				if (delay_to_start > 3.0f && !start_timer_on)
				{
					if (topdriver_number && topgunner_number && botdriver_number && botgunner_number)
					{
						td_number->SetIdToRender(App->go_manager->team1_front);
						topdriver_number->SetActive(false);

						tg_number->SetIdToRender(App->go_manager->team1_back);
						topgunner_number->SetActive(false);

						bd_number->SetIdToRender(App->go_manager->team2_front);
						botdriver_number->SetActive(false);

						bg_number->SetIdToRender(App->go_manager->team2_back);
						botgunner_number->SetActive(false);
					}

					if (start_timer_text) start_timer_text->GetGameObject()->SetActive(true);
					if (start_timer_text2) start_timer_text2->GetGameObject()->SetActive(true);
					start_timer_on = true;
				}
				else
					delay_to_start += time->DeltaTime();
			}			

			if (start_timer_on)
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
				//Disqualification
				if (second_position_timer >= 20 && goingToDisqualify == 1)
				{
					car_1->finished = true;
				}

				//Wrong Direction
				if (top_wrongdirection && car_1->wrongDirection == true)
				{
					if (wrong_dir_timer_1 <= 0.6f)
						top_wrongdirection->SetActive(true);
					else
						top_wrongdirection->SetActive(false);

					wrong_dir_timer_1 += time->DeltaTime();

					if (wrong_dir_timer_1 >= 1.2f)
						wrong_dir_timer_1 = 0.0f;
				}
				else
				{
					top_wrongdirection->SetActive(false);
					wrong_dir_timer_1 = 0.0f;
				}

				//Update lap counter
				if (car_1->lap + 1 > timer.GetCurrentLap(0))
				{
					if (car_1->lap > 3)
					{
						car_1->finished = true;

						if (car_1->place == 1)
						{
							if (audio_source)
							{
								audio_source->PlayAudio(4);
							}								
						}
						else
						{
							if (audio_source)
							{
								audio_source->PlayAudio(5);
							}
						}

						if (goingToDisqualify == 0)
						{
							goingToDisqualify = 2;
						}
					}

					if (car_1->finished == true )
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
						string str;
						if (car_1->lap >= 3)
						{
							str = std::to_string(3);
						}
						else
						{
							str = std::to_string(car_1->lap);
						}
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
				//Disqualification
				if (second_position_timer >= 20 && goingToDisqualify == 2)
				{
					car_2->finished = true;
				}

				//Wrong Direction
				if (bot_wrongdirection && car_2->wrongDirection == true)
				{
					if (wrong_dir_timer_2 <= 0.6f)
						bot_wrongdirection->SetActive(true);
					else
						bot_wrongdirection->SetActive(false);

					wrong_dir_timer_2 += time->DeltaTime();

					if (wrong_dir_timer_2 >= 1.2f)
						wrong_dir_timer_2 = 0.0f;
				}
				else
				{
					bot_wrongdirection->SetActive(false);
					wrong_dir_timer_2 = 0.0f;
				}

				//Update lap counter
				if (car_2->lap + 1 > timer.GetCurrentLap(1))
				{
					if (car_2->lap > 3)
					{
						car_2->finished = true;

						if (car_2->place == 1)
						{
							if (audio_source)
							{
								audio_source->PlayAudio(4);
							}
						}
						else
						{
							if (audio_source)
							{
								audio_source->PlayAudio(5);
							}
						}

						if (goingToDisqualify == 0)
						{
							goingToDisqualify = 1;
						}
					}

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
						string str;
						if (car_2->lap >= 3)
						{
							str = std::to_string(3);
						}
						else
						{
							str = std::to_string(car_2->lap);
						}
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

	void Scene_Manager_ResultWindow(GameObject* game_object)
	{
		if (quit == true)
		{
			if (quit_timer >= quit_max_timer)
				App->input->SetQuit();
			else
				quit_timer += time->DeltaTime();
		}
		if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_DOWN)
		{
			if (audio_source) audio_source->PlayAudio(7);   // Movement
			if (result_focus_pos == 2)
			{
				result_focus_pos = 0;
				if (focus_result)((ComponentRectTransform*)focus_result->GetComponent(C_RECT_TRANSFORM))->Move(float3(0, -200, 0));
			}
			else
			{
				result_focus_pos++;
				if (focus_result)((ComponentRectTransform*)focus_result->GetComponent(C_RECT_TRANSFORM))->Move(float3(0, 100, 0));
			}
		}

		if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_DOWN)
		{
			if (audio_source) audio_source->PlayAudio(7);   // Movement
			if (result_focus_pos == 0)
			{
				result_focus_pos = 2;
				if (focus_result)((ComponentRectTransform*)focus_result->GetComponent(C_RECT_TRANSFORM))->Move(float3(0, 200, 0));
			}
			else
			{
				result_focus_pos--;
				if(focus_result)((ComponentRectTransform*)focus_result->GetComponent(C_RECT_TRANSFORM))->Move(float3(0, -100, 0));
			}
		}

		if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
		{
			if (audio_source) audio_source->PlayAudio(3);   // Stopping all sounds and playing Selection FX
			switch (result_focus_pos)
			{
			case 0: 

			// Selecting Assets or Library version depending on Game mode
				if (App->StartInGame())
					App->LoadScene(library_main_menu_scene.c_str());	// Using Library Scene files
				else
					App->LoadScene(assets_main_menu_scene.c_str());		// Using Assets Scene files
				break;
			case 1:
																// Selecting Assets or Library version depending on Game mode
				if (App->go_manager->current_scene == 0)
				{
					if (App->StartInGame())
						App->LoadScene(library_scene_1_reload.c_str());	// Using Library Scene files
					else
						App->LoadScene(assets_scene_1_reload.c_str());		// Using Assets Scene files
					
				}
				else if (App->go_manager->current_scene == 1)
				{
					if (App->StartInGame())
						App->LoadScene(library_scene_2_reload.c_str());	// Using Library Scene files
					else
						App->LoadScene(assets_scene_2_reload.c_str());		// Using Assets Scene file
				}
				break;
			case 2:
				quit = true;
				if (credits)credits->SetActive(true);
				break;
			}
		}

		for (uint joystick = 0; joystick < 4; joystick++)
		{
			if (App->input->GetJoystickAxis(joystick, JOY_AXIS::LEFT_STICK_Y) < 0.2f && App->input->GetJoystickAxis(joystick, JOY_AXIS::LEFT_STICK_Y) > -0.2f && triggers_pressed[joystick] == true)
			{
				triggers_pressed[joystick] = false;
			}

			if (App->input->GetJoystickButton(joystick, JOY_BUTTON::DPAD_DOWN) == KEY_DOWN || (App->input->GetJoystickAxis(joystick, JOY_AXIS::LEFT_STICK_Y) > 0.75f && triggers_pressed[joystick] == false))
			{
				triggers_pressed[joystick] = true;
				if (result_focus_pos == 2)
				{
					result_focus_pos = 0;
					if (focus_result)((ComponentRectTransform*)focus_result->GetComponent(C_RECT_TRANSFORM))->Move(float3(0, -200, 0));
				}
				else
				{
					result_focus_pos++;
					if (focus_result)((ComponentRectTransform*)focus_result->GetComponent(C_RECT_TRANSFORM))->Move(float3(0, 100, 0));
				}
			}

			if (App->input->GetJoystickButton(joystick, JOY_BUTTON::DPAD_UP) == KEY_DOWN || (App->input->GetJoystickAxis(joystick, JOY_AXIS::LEFT_STICK_Y) < -0.75f && triggers_pressed[joystick] == false))
			{
				triggers_pressed[joystick] = true;
				if (result_focus_pos == 0)
				{
					result_focus_pos = 2;
					if (focus_result)((ComponentRectTransform*)focus_result->GetComponent(C_RECT_TRANSFORM))->Move(float3(0, 200, 0));
				}
				else
				{
					result_focus_pos--;
					if (focus_result)((ComponentRectTransform*)focus_result->GetComponent(C_RECT_TRANSFORM))->Move(float3(0, -100, 0));
				}
			}
			if (App->input->GetJoystickButton(joystick, JOY_BUTTON::A) == KEY_DOWN)
			{
				if (audio_source) audio_source->PlayAudio(3);   // Stopping Music
				switch (result_focus_pos)
				{
				case 0:

					// Selecting Assets or Library version depending on Game mode
					if (App->StartInGame())
						App->LoadScene(library_main_menu_scene.c_str());	// Using Library Scene files
					else
						App->LoadScene(assets_main_menu_scene.c_str());		// Using Assets Scene files
					break;
				case 1:
					// Selecting Assets or Library version depending on Game mode
					if (App->go_manager->current_scene == 0)
					{
						if (App->StartInGame())
							App->LoadScene(library_scene_1_reload.c_str());	// Using Library Scene files
						else
							App->LoadScene(assets_scene_1_reload.c_str());		// Using Assets Scene files

					}
					else if (App->go_manager->current_scene == 1)
					{
						if (App->StartInGame())
							App->LoadScene(library_scene_2_reload.c_str());	// Using Library Scene files
						else
							App->LoadScene(assets_scene_2_reload.c_str());		// Using Assets Scene file
					}
					break;
				case 2:
					quit = true;
					if (credits)credits->SetActive(true);
					break;
				}
			}
		}
	}
}