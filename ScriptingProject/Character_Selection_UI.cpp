#include "stdafx.h"

#include <string>
#include <map>
#include <vector>
#include "../Application.h"
#include "../ModuleScripting.h"
#include "../ModuleInput.h"
#include "../ModuleWindow.h"
#include "../ModuleGOManager.h"
#include "../ComponentTransform.h"
#include "../GameObject.h"
#include "../Component.h"
#include "../ComponentScript.h"
#include "../ComponentRectTransform.h"
#include "../SDL/include/SDL_scancode.h"
#include "../Globals.h"
#include "../ComponentGrid.h"
#include "../ComponentUiButton.h"
#include "../ComponentCanvas.h"

namespace Character_Selection_UI
{
	GameObject* grid_drivers = nullptr;
	GameObject* grid_sup = nullptr;
	GameObject* driver1 = nullptr;
	GameObject* support1 = nullptr;
	GameObject* driver2 = nullptr;
	GameObject* support2 = nullptr;

	ComponentGrid* grid_driv = nullptr;
	ComponentGrid* grid_s = nullptr;
	ComponentUiButton* but_driver1 = nullptr;
	ComponentUiButton* but_support1 = nullptr;
	ComponentUiButton* but_driver2 = nullptr;
	ComponentUiButton* but_support2 = nullptr;

	int player_order[4];

	void Character_Selection_UI_GetPublics(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools, map<const char*, GameObject*>* public_gos)
	{
		public_gos->insert(std::pair<const char*, GameObject*>("Drivers grid", grid_drivers));
		public_gos->insert(std::pair<const char*, GameObject*>("Support grid", grid_sup));
		public_gos->insert(std::pair<const char*, GameObject*>("Drivers 1", driver1));
		public_gos->insert(std::pair<const char*, GameObject*>("Support 1", support1));
		public_gos->insert(std::pair<const char*, GameObject*>("Drivers 2", driver2));
		public_gos->insert(std::pair<const char*, GameObject*>("Support 2", support2));

		public_ints->insert(std::pair<const char*, int>("Player1", player_order[0]));
		public_ints->insert(std::pair<const char*, int>("Player2", player_order[1]));
		public_ints->insert(std::pair<const char*, int>("Player3", player_order[2]));
		public_ints->insert(std::pair<const char*, int>("Player4", player_order[3]));
	}

	void Character_Selection_UI_UpdatePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		grid_drivers = test_script->public_gos.at("Drivers grid");
		grid_sup = test_script->public_gos.at("Support grid");
		driver1 = test_script->public_gos.at("Drivers 1");
		support1 = test_script->public_gos.at("Support 1");
		driver2 = test_script->public_gos.at("Drivers 2");
		support2 = test_script->public_gos.at("Support 2");

		player_order[0] = test_script->public_ints.at("Player1");
		player_order[1] = test_script->public_ints.at("Player2");
		player_order[2] = test_script->public_ints.at("Player3");
		player_order[3] = test_script->public_ints.at("Player4");

		grid_driv = (ComponentGrid*)grid_drivers->GetComponent(C_GRID);
		/*grid_s = (ComponentGrid*)grid_sup->GetComponent(C_GRID);
		but_driver1 = (ComponentUiButton*)driver1->GetComponent(C_UI_BUTTON);
		but_support1 = (ComponentUiButton*)support1->GetComponent(C_UI_BUTTON);
		but_driver2 = (ComponentUiButton*)driver2->GetComponent(C_UI_BUTTON);
		but_support2 = (ComponentUiButton*)support2->GetComponent(C_UI_BUTTON);*/

	}

	void Character_Selection_UI_ActualizePublics(GameObject* game_object)
	{
		ComponentScript* test_script = (ComponentScript*)game_object->GetComponent(ComponentType::C_SCRIPT);

		test_script->public_gos.at("Drivers grid") = grid_drivers;
		test_script->public_gos.at("Support grid") = grid_sup;
		test_script->public_gos.at("Drivers 1") = driver1;
		test_script->public_gos.at("Support 1") = support1;
		test_script->public_gos.at("Drivers 2") = driver2;
		test_script->public_gos.at("Support 2") = support2;

		test_script->public_ints.at("Player1") = player_order[0];
		test_script->public_ints.at("Player2") = player_order[1];
		test_script->public_ints.at("Player3") = player_order[2];
		test_script->public_ints.at("Player4") = player_order[2];

		grid_driv = (ComponentGrid*)grid_drivers->GetComponent(C_GRID);
		/*grid_s = (ComponentGrid*)grid_sup->GetComponent(C_GRID);
		but_driver1 = (ComponentUiButton*)driver1->GetComponent(C_UI_BUTTON);
		but_support1 = (ComponentUiButton*)support1->GetComponent(C_UI_BUTTON);
		but_driver2 = (ComponentUiButton*)driver2->GetComponent(C_UI_BUTTON);
		but_support2 = (ComponentUiButton*)support2->GetComponent(C_UI_BUTTON);*/
	}

	void Character_Selection_UI_UpdatePublics(GameObject* game_object);

	void Character_Selection_UI_Start(GameObject* game_object)
	{
		//Character_Selection_UI_ActualizePublics(game_object);
		App->go_manager->current_scene_canvas->AddGoFocus(grid_drivers);
		grid_driv->SetPlayerOrder(player_order[0], player_order[1], player_order[2], player_order[3]);
	}

	void Character_Selection_UI_Update(GameObject* game_object)
	{

	}

	void Character_Selection_UI_OnFocus(GameObject* game_object)
	{

	}
}