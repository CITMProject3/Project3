#include "Hierarchy.h"
#include "Application.h"
#include "Editor.h"

#include "GameObject.h"
#include "Component.h"
#include "ComponentTransform.h"

#include "ModuleGOManager.h"
#include "LayerSystem.h"

Hierarchy::Hierarchy()
{
	active = true;
}
Hierarchy::~Hierarchy()
{}

void Hierarchy::Draw(ImGuiWindowFlags flags)
{
	if (!active) return;

	ImGui::SetNextWindowSize(current_size);
	ImGui::SetNextWindowPos(current_position);

	ImGui::Begin("Hierarchy", &active, flags);
	
	DisplayGameObjectsChilds(App->go_manager->root->GetChilds());

	if (ImGui::IsMouseHoveringWindow())
	{
		if (ImGui::IsMouseClicked(1))
		{
			ImGui::OpenPopup("HierarchyOptions");
		}
		if (ImGui::IsMouseClicked(0) && settingParent == true)
		{
			if (App->editor->selected.size() > 0)
			{
				App->editor->selected.back()->SetParent(App->go_manager->root);
			}
		}
	}



	if (ImGui::BeginPopup("HierarchyOptions"))
	{
		if (ImGui::Selectable("Create Empty GameObject"))
		{
			App->editor->SelectSingle(App->go_manager->CreateGameObject(NULL));
		}

		if (ImGui::Selectable("Create Empty Child"))
		{
			App->editor->SelectSingle(App->go_manager->CreateGameObject(App->editor->selected.back()));
		}

		if (ImGui::Selectable("Remove selected GameObject"))
		{
			App->editor->RemoveSelected();
		}
		if (ImGui::Selectable("Create Prefab"))
		{
			if (App->editor->selected.size() > 0)
			{
				App->resource_manager->SavePrefab(App->editor->selected.back());
			}
		}

		if (ImGui::Selectable("Set Parent"))
		{
			settingParent = true;
		}

		ImGui::EndPopup();
	}

	ImGui::End();
}

void Hierarchy::DisplayGameObjectsChilds(const std::vector<GameObject*>* childs)
{
	for (std::vector<GameObject*>::const_iterator object = (*childs).begin(); object != (*childs).end(); ++object)
	{
		uint flags = ImGuiTreeNodeFlags_OpenOnArrow;
		if (App->editor->IsSelected(*object))
			flags |= ImGuiTreeNodeFlags_Selected;

		if ((*object)->ChildCount() == 0)
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		bool open = ImGui::TreeNodeEx((*object)->name.data(), flags);
		bool hasChilds = (*object)->ChildCount() > 0;

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			ComponentTransform* transform = (ComponentTransform*)(*object)->GetComponent(C_TRANSFORM);
			App->camera->Center(transform->GetGlobalMatrix().TranslatePart());
		}

		if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
		{
			if (settingParent == true)
			{
				if (App->editor->selected.size() > 0)
				{
					App->editor->selected.back()->SetParent(*object);
				}
				settingParent = false;
			}
			else
			{
				OnClickSelect(*object);
			}
		}

		if (hasChilds > 0 && open)
		{
			DisplayGameObjectsChilds((*object)->GetChilds());
			ImGui::TreePop();
		}
	}
}

void Hierarchy::OnClickSelect(GameObject* game_object)
{
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT ||
		App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT)
	{
		if (App->editor->IsSelected(game_object))
		{
			App->editor->Unselect(game_object);
		}
		else
		{
			App->editor->AddSelect(game_object);
		}
	}
	else
	{
		App->editor->SelectSingle(game_object);
	}
}