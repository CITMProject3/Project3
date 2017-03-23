#include "Application.h"
#include "ComponentCanvas.h"
#include "ModuleResourceManager.h"
#include "ModuleGOManager.h"
#include "GameObject.h"
#include "imgui\imgui.h"
#include "ModuleInput.h"
#include "ComponentUiText.h"
#include "RaceTimer.h"
#include "ComponentCar.h"
#include "PhysVehicle3D.h"

ComponentCanvas::ComponentCanvas(ComponentType type, GameObject * game_object) : Component(type, game_object)
{
	r_timer = new RaceTimer();
}

ComponentCanvas::~ComponentCanvas()
{
	delete r_timer;
	if (App->go_manager->current_scene_canvas == this)
		App->go_manager->current_scene_canvas = nullptr;
}

void ComponentCanvas::Update()
{
	if (scene_to_change != current_scene)
		OnChangeScene();
	GameObject* obj;
	switch (current_scene)
	{
		//Main menu
	case 0:
		if (player_1_ready && player_2_ready)
			scene_to_change = 1;
		break;
		//GamePlayMenu
	case 1:
		if (win)
			scene_to_change = 2;
		if (current_car != nullptr)
		{
			if (play_timer != nullptr)
			{
				int min, sec, milsec = 0;
				if ((current_car->lap+1) != r_timer->GetCurrentLap())
					r_timer->AddLap();
				r_timer->GetCurrentLapTime(min, sec, milsec);
				string min_te = to_string(min);
				string sec_te = to_string(sec);
				string mil_te = to_string(milsec);
				if (min < 10)
					min_te = "0" + min_te;
				if (sec < 10)
					sec_te = "0" + sec_te;
				if (milsec < 100)
					mil_te = "0" + mil_te;
				string str = min_te + ":" + sec_te + ":" + mil_te;
				play_timer->SetDisplayText(str);
			}

			if (kmh_text != nullptr)
			{
				string str = to_string(int(current_car->GetVelocity())) + "k";
				kmh_text->SetDisplayText(str);
			}
		}
		
		
		break;
	//Win Menu
	case 2:
		if (App->input->GetJoystickButton(0, JOY_BUTTON::A) == KEY_DOWN)
		{
			restart = true;
		}
		if (App->input->GetJoystickButton(0, JOY_BUTTON::B) == KEY_DOWN)
		{
			//QUIT GAME
		}
		if (restart)
			scene_to_change = 0;
		break;
	}
}

void ComponentCanvas::OnPlay()
{
	GameObject* obj = (*game_object->GetChilds()).at(1);
	if (obj != nullptr)
	{
		GameObject* obj_child_time = (*game_object->GetChilds()).at(1)->GetChilds()->at(0);
		if (obj_child_time != nullptr)
		{
			play_timer = (ComponentUiText*)obj_child_time->GetComponent(C_UI_TEXT);
		}

		GameObject* obj_child_vel = (*game_object->GetChilds()).at(1)->GetChilds()->at(1);
		if (obj_child_vel != nullptr)
		{
			kmh_text = (ComponentUiText*)obj_child_vel->GetComponent(C_UI_TEXT);
			if (kmh_text != nullptr)
			{
				string str = to_string(kmh) + " k";
				kmh_text->SetDisplayText(str);
			}

		}
	}

	GameObject* obji = (*game_object->GetChilds()).at(2);
	if (obji != nullptr)
	{
		GameObject* obj_child_time = (*game_object->GetChilds()).at(2)->GetChilds()->at(0);
		if (obj_child_time != nullptr)
		{
			win_timer = (ComponentUiText*)obj_child_time->GetComponent(C_UI_TEXT);
		}
	}

	vector<GameObject*> all_objects;
	App->go_manager->root->CollectAllChilds(all_objects);
	for (vector<GameObject*>::const_iterator obj = all_objects.begin(); obj != all_objects.end(); ++obj)
	{
		current_car = (ComponentCar*)(*obj)->GetComponent(C_CAR);
		if (current_car != nullptr)
			return;
	}
}

void ComponentCanvas::OnInspector(bool debug)
{
	string str = (string("Canvas") + string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##canvas");
		}

		if (ImGui::BeginPopup("delete##canvas"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}
		int tmp = scene_to_change;
		if (ImGui::InputInt("Scene to change", &tmp))
		{
			ChangeScene(tmp);
		}
		bool tmp_win = win;
		if (ImGui::Checkbox("Scene to change", &tmp_win))
		{
			win = tmp_win;
		}
	}
}

void ComponentCanvas::Save(Data & file) const
{
	Data data;

	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);

	file.AppendArrayValue(data);
}

void ComponentCanvas::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
}

void ComponentCanvas::Remove()
{
	if(App->go_manager->current_scene_canvas == this)
		App->go_manager->current_scene_canvas = nullptr;
	game_object->RemoveComponent(this);
}

void ComponentCanvas::ChangeScene(int scene)
{
	if (scene < 0)
		scene = 0;
	else if (scene > 2)
		scene = 2;
	
	scene_to_change = scene;
}

void ComponentCanvas::SetPlayerReady(int num_player, bool ready)
{
	if (num_player == 0)
		player_1_ready = true;
	else if (num_player == 1)
		player_2_ready = true;
}

void ComponentCanvas::SetWin(bool win)
{
	this->win = win;
}

vector<GameObject*> ComponentCanvas::GetUI()
{
	vector<GameObject*> tmp_childs;
	for (vector<GameObject*>::const_iterator obj = (*GetGameObject()->GetChilds()).begin(); obj != (*GetGameObject()->GetChilds()).end(); ++obj)
	{
		
		if ((*obj)->IsActive())
		{
			tmp_childs.push_back(*obj);
			vector<GameObject*> obj_childs = GetGameObjectChilds(*obj);
			tmp_childs.insert(tmp_childs.end(), obj_childs.begin(), obj_childs.end());
		}
		
	}

	return tmp_childs;
}

void ComponentCanvas::OnChangeScene()
{
	switch (current_scene)
	{
	//Main menu
	case 0:
		if (scene_to_change == 1)
		{
			vector<GameObject*> tmp_childs = (*game_object->GetChilds());
			if(tmp_childs.size() > 2)
			{ 
				tmp_childs.at(0)->SetActive(false);
				tmp_childs.at(1)->SetActive(true);
				tmp_childs.at(2)->SetActive(false);	
			}
			player_1_ready = false;
			player_2_ready = false;
			r_timer->Start();
			//r_timer->AddLap();
		}
		current_scene = scene_to_change;
		
		break;
	//GamePlayMenu
	case 1:
		if (scene_to_change == 0)
		{
			vector<GameObject*> tmp_childs = (*game_object->GetChilds());
			if (tmp_childs.size() > 2)
			{
				tmp_childs.at(0)->SetActive(true);
				tmp_childs.at(1)->SetActive(false);
				tmp_childs.at(2)->SetActive(false);
			}
		}

		if (scene_to_change == 2)
		{
			if (current_car != nullptr)
			{
				if (win_timer != nullptr)
				{
					r_timer->AddLap();
					int min, sec, milsec = 0;
					if ((current_car->lap + 1) != r_timer->GetCurrentLap())
						r_timer->AddLap();
					if (r_timer->GetAllLapsTime(min, sec, milsec))
					{
						string min_te = to_string(min);
						string sec_te = to_string(sec);
						string mil_te = to_string(milsec);
						if (min < 10)
							min_te = "0" + min_te;
						if (sec < 10)
							sec_te = "0" + sec_te;
						if (milsec < 100)
							mil_te = "0" + mil_te;
						string str = min_te + ":" + sec_te + ":" + mil_te;
						win_timer->SetDisplayText(str);
					}
				}
			}
			vector<GameObject*> tmp_childs = (*game_object->GetChilds());
			if (tmp_childs.size() > 2)
			{
				tmp_childs.at(0)->SetActive(false);
				tmp_childs.at(1)->SetActive(false);
				tmp_childs.at(2)->SetActive(true);
			}
			
		}
		current_scene = scene_to_change;
		break;
	//Win Menu
	case 2:
		if (scene_to_change == 0)
		{
			vector<GameObject*> tmp_childs = (*game_object->GetChilds());
			if (tmp_childs.size() > 2)
			{
				tmp_childs.at(0)->SetActive(true);
				tmp_childs.at(1)->SetActive(false);
				tmp_childs.at(2)->SetActive(false);
				
			}
			win = false;
			restart = false;
		}
		current_scene = scene_to_change;
		break;
	}
}

vector<GameObject*> ComponentCanvas::GetGameObjectChilds(GameObject * go)
{
	vector<GameObject*> tmp_childs = (*go->GetChilds());

	for (vector<GameObject*>::const_iterator obj = (*go->GetChilds()).begin(); obj != (*go->GetChilds()).end(); ++obj)
	{
		if ((*obj)->IsActive())
		{
			vector<GameObject*> obj_childs = GetGameObjectChilds(*obj);
			tmp_childs.insert(tmp_childs.end(), obj_childs.begin(), obj_childs.end());
		}
	}

	return tmp_childs;
}
