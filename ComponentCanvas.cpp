#include "Application.h"
#include "ComponentCanvas.h"
#include "ModuleResourceManager.h"
#include "ModuleGOManager.h"
#include "GameObject.h"
#include "imgui\imgui.h"

#include "ComponentRectTransform.h"
#include "ModuleWindow.h"

#include "ModuleInput.h"
#include "ComponentUiText.h"
#include "ComponentUiButton.h"
#include "RaceTimer.h"
#include "ComponentCar.h"

ComponentCanvas::ComponentCanvas(ComponentType type, GameObject * game_object) : Component(type, game_object)
{
	OnTransformModified();
	current_width = 1600;
	current_height = 900;
}

ComponentCanvas::~ComponentCanvas()
{
	if (App->go_manager->current_scene_canvas == this)
		App->go_manager->current_scene_canvas = nullptr;
}

void ComponentCanvas::Update()
{
	if (go_focus.size() > 0)
	{
		for (vector<GameObject*>::const_iterator obj = go_focus.begin(); obj != go_focus.end(); ++obj)
		{
			vector<Component*> focus_components = *(*obj)->GetComponents();

			for (vector<Component*>::const_iterator comp = focus_components.begin(); comp != focus_components.end(); ++comp)
			{
				if ((*comp) != nullptr)
				{
					(*comp)->OnFocus();
				}
			}
		}
	}
}

void ComponentCanvas::OnPlay()
{
	

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

	}
	if (go_focus.size() > 0)
	{
		for (vector<GameObject*>::const_iterator obj = go_focus.begin(); obj != go_focus.end(); ++obj)
		{
			ImGui::Text(string("Current Focus: " + (*obj)->name).data());
		}
	}
		
}

void ComponentCanvas::OnTransformModified()
{
	ComponentRectTransform* c = (ComponentRectTransform*)game_object->GetComponent(C_RECT_TRANSFORM);

	
	c->SetScale(float2(float(App->window->GetScreenWidth())/float(current_width), float(App->window->GetScreenHeight())/ float(current_height)));
	c->SetSize(float2(App->window->GetScreenWidth(), App->window->GetScreenHeight()));
	c->SetLocalPos(float2(App->window->GetScreenWidth(), 0.0f));
}

void ComponentCanvas::ResizeScreen()
{
	game_object->TransformModified();
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

vector<GameObject*> ComponentCanvas::GetUI()
{
	vector<GameObject*> tmp_childs;
	vector<GameObject*> tmp_unorganized;

	tmp_unorganized = tmp_childs = *GetGameObject()->GetChilds();

	for (vector<GameObject*>::const_iterator obj = tmp_childs.begin(); obj != tmp_childs.end(); ++obj)
	{
		vector<GameObject*> obj_childs = GetGameObjectChilds(*obj);
		tmp_unorganized.insert(tmp_unorganized.end(), obj_childs.begin(), obj_childs.end());
	}

	tmp_childs.clear();

	for (int i = 0; i <= 7; i++)
	{
		for (vector<GameObject*>::const_iterator obj = tmp_unorganized.begin(); obj != tmp_unorganized.end(); ++obj)
		{
			ComponentRectTransform* lol = (ComponentRectTransform*)(*obj)->GetComponent(C_RECT_TRANSFORM);

			if (lol != nullptr)
			{
				if (lol->order == i && (*obj)->IsActive())
				{
					tmp_childs.push_back(*obj);
				}
			}
		}
	}

	return tmp_childs;
}

vector<GameObject *> ComponentCanvas::GetGoFocus() const
{
	return go_focus;
}

void ComponentCanvas::RemoveGoFocus(GameObject * new_focus)
{
	go_focus.erase(std::remove(go_focus.begin(), go_focus.end(), new_focus), go_focus.end());
}

void ComponentCanvas::ClearFocus()
{
	go_focus.clear();
}

void ComponentCanvas::AddGoFocus(GameObject* new_focus)
{
	if ((new_focus) != nullptr)
		go_focus.push_back(new_focus);
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
