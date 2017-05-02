#include "stdafx.h"

#include <string>
#include <map>
#include <vector>
#include "../Application.h"
#include "../ModuleScripting.h"
#include "../GameObject.h"
#include "../ComponentScript.h"
#include "../ComponentCar.h"
#include "../ModuleGOManager.h"

//This script is basically made for setting active / non-active the GameObjects according to
//players car and character selection

//"Actualize publics" is not set by now, since scripts will only be executed once and there
//is no need to keep an update of the variables
enum CAR_TYPE;

namespace PlayerObjectSelector
{
	//Team 1 -----------------------------
	GameObject* car1 = nullptr;
	//Car
	GameObject* team1_car1 = nullptr;
	GameObject* team1_car2 = nullptr;

	//Player1 character
	GameObject* team1_p1_c1 = nullptr;
	GameObject* team1_p1_c2 = nullptr;

	//Player2 character
	GameObject* team1_p2_c1 = nullptr;
	GameObject* team1_p2_c2 = nullptr;
	//------------------------------------

	//Team 2 -----------------------------
	GameObject* car2 = nullptr;
	//Car
	GameObject* team2_car1 = nullptr;
	GameObject* team2_car2 = nullptr;

	//Player1 character
	GameObject* team2_p1_c1 = nullptr;
	GameObject* team2_p1_c2 = nullptr;

	//Player2 character
	GameObject* team2_p2_c1 = nullptr;
	GameObject* team2_p2_c2 = nullptr;
	//------------------------------------

	void PlayerObjectSelector_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(pair<const char*, GameObject*>("team1 component car", car1));

		public_gos->insert(pair<const char*, GameObject*>("team1_car1", team1_car1));
		public_gos->insert(pair<const char*, GameObject*>("team1_car2", team1_car2));

		public_gos->insert(pair<const char*, GameObject*>("team1_p1_c1", team1_p1_c1));
		public_gos->insert(pair<const char*, GameObject*>("team1_p1_c2", team1_p1_c2));

		public_gos->insert(pair<const char*, GameObject*>("team1_p2_c1", team1_p2_c1));
		public_gos->insert(pair<const char*, GameObject*>("team1_p2_c2", team1_p2_c2));

		public_gos->insert(pair<const char*, GameObject*>("team2 component car", car2));

		public_gos->insert(pair<const char*, GameObject*>("team2_car1", team2_car1));
		public_gos->insert(pair<const char*, GameObject*>("team2_car2", team2_car2));

		public_gos->insert(pair<const char*, GameObject*>("team2_p1_c1", team2_p1_c1));
		public_gos->insert(pair<const char*, GameObject*>("team2_p1_c2", team2_p1_c2));

		public_gos->insert(pair<const char*, GameObject*>("team2_p2_c1", team2_p2_c1));
		public_gos->insert(pair<const char*, GameObject*>("team2_p2_c2", team2_p2_c2));
	}

	void PlayerObjectSelector_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* script = (ComponentScript*)game_object->GetComponent(C_SCRIPT);

		car1 = script->public_gos.at("team1 component car");
		car2 = script->public_gos.at("team2 component car");

		team1_car1 = script->public_gos.at("team1_car1");
		team1_car2 = script->public_gos.at("team1_car2");

		team1_p1_c1 = script->public_gos.at("team1_p1_c1");
		team1_p1_c2 = script->public_gos.at("team1_p1_c2");

		team1_p2_c1 = script->public_gos.at("team1_p2_c1");
		team1_p2_c2 = script->public_gos.at("team1_p2_c2");

		team2_car1 = script->public_gos.at("team2_car1");
		team2_car2 = script->public_gos.at("team2_car2");

		team2_p1_c1 = script->public_gos.at("team2_p1_c1");
		team2_p1_c2 = script->public_gos.at("team2_p1_c2");

		team2_p2_c1 = script->public_gos.at("team2_p2_c1");
		team2_p2_c2 = script->public_gos.at("team2_p2_c2");
	}

	void PlayerObjectSelector_Start(GameObject* game_object)
	{
		ComponentCar* comp_car_1 = nullptr;
		ComponentCar* comp_car_2 = nullptr;

		//Team 1
		if (car1)
		{
			comp_car_1 = (ComponentCar*)car1->GetComponent(C_CAR);
			comp_car_1->SetFrontPlayer(App->go_manager->team1_front);
			comp_car_1->SetBackPlayer(App->go_manager->team1_back);
		}

		if (team1_car1)	team1_car1->SetActive(App->go_manager->team1_car == 0);
		if (team1_car2)	team1_car2->SetActive(App->go_manager->team1_car == 1);
		comp_car_1->SetCarType(CAR_TYPE(App->go_manager->team1_car));

		if (team1_p1_c1) team1_p1_c1->SetActive(App->go_manager->team1_p1_c == 0);
		if (team1_p1_c2) team1_p1_c2->SetActive(App->go_manager->team1_p1_c == 1);
		
		if (comp_car_1)
		{
			comp_car_1->p1_animation = (ComponentAnimation*)(App->go_manager->team1_p1_c == 0 ? team1_p1_c1 : team1_p1_c2)->GetComponent(C_ANIMATION);
		}

		if (team1_p2_c1) team1_p2_c1->SetActive(App->go_manager->team1_p2_c == 2);
		if (team1_p2_c2) team1_p2_c2->SetActive(App->go_manager->team1_p2_c == 3);

		if (comp_car_1) comp_car_1->p2_animation = (ComponentAnimation*)(App->go_manager->team1_p2_c == 2 ? team1_p2_c1 : team1_p2_c2)->GetComponent(C_ANIMATION);

		//Team 2
		if (car2)
		{
			comp_car_2 = (ComponentCar*)car2->GetComponent(C_CAR);
			comp_car_2->SetFrontPlayer(App->go_manager->team2_front);
			comp_car_2->SetBackPlayer(App->go_manager->team2_back);
		}

		if (team2_car1)	team2_car1->SetActive(App->go_manager->team2_car == 0);
		if (team2_car2)	team2_car2->SetActive(App->go_manager->team2_car == 1);
		comp_car_2->SetCarType(CAR_TYPE(App->go_manager->team2_car));

		if (team2_p1_c1) team2_p1_c1->SetActive(App->go_manager->team2_p1_c == 0);
		if (team2_p1_c2) team2_p1_c2->SetActive(App->go_manager->team2_p1_c == 1);

		if (comp_car_2) comp_car_2->p1_animation = (ComponentAnimation*)(App->go_manager->team2_p1_c == 0 ? team2_p1_c1 : team2_p1_c2)->GetComponent(C_ANIMATION);

		if (team2_p2_c1) team2_p2_c1->SetActive(App->go_manager->team2_p2_c == 2);
		if (team2_p2_c2) team2_p2_c2->SetActive(App->go_manager->team2_p2_c == 3);

		if (comp_car_1) comp_car_2->p2_animation = (ComponentAnimation*)(App->go_manager->team2_p2_c == 2 ? team2_p2_c1 : team2_p2_c2)->GetComponent(C_ANIMATION);
	}
}