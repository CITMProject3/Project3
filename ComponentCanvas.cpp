#include "Application.h"
#include "ComponentCanvas.h"
#include "ModuleResourceManager.h"
#include "ModuleGOManager.h"
#include "GameObject.h"
#include "imgui\imgui.h"
#include "ModuleInput.h"
ComponentCanvas::ComponentCanvas(ComponentType type, GameObject * game_object) : Component(type,game_object)
{

}

ComponentCanvas::~ComponentCanvas()
{
	if (App->go_manager->current_scene_canvas == this)
		App->go_manager->current_scene_canvas = nullptr;
}

void ComponentCanvas::Update()
{
	if (scene_to_change != current_scene)
		OnChangeScene();

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
