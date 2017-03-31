#include "Application.h"
#include "ComponentCanvas.h"
#include "ModuleResourceManager.h"
#include "ModuleGOManager.h"
#include "GameObject.h"
#include "imgui\imgui.h"
#include "ComponentRectTransform.h"
#include "ModuleWindow.h"
#include "ComponentGrid.h"

ComponentCanvas::ComponentCanvas(ComponentType type, GameObject * game_object) : Component(type, game_object)
{
	OnTransformModified();
}

ComponentCanvas::~ComponentCanvas()
{
	if (App->go_manager->current_scene_canvas == this)
		App->go_manager->current_scene_canvas = nullptr;
}

void ComponentCanvas::Update()
{
	if (go_focus != nullptr)
	{
		ComponentGrid* grid = (ComponentGrid*)go_focus->GetComponent(C_GRID);
		if (grid != nullptr)
		{
			grid->OnFocus();
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
}

void ComponentCanvas::OnTransformModified()
{
	ComponentRectTransform* c = (ComponentRectTransform*)game_object->GetComponent(C_RECT_TRANSFORM);

	c->SetSize(float2(App->window->GetScreenWidth(), App->window->GetScreenHeight()));
	c->SetLocalPos(float2(App->window->GetScreenWidth(), 0.0f));
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

GameObject * ComponentCanvas::GetGoFocus() const
{
	return go_focus;
}

void ComponentCanvas::SetGoFocus(GameObject * new_focus)
{
	if (new_focus != nullptr)
		go_focus = new_focus;
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
