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
		if (ImGui::IsMouseClicked(1))
			ImGui::OpenPopup("HierarchyOptions");

	if (ImGui::BeginPopup("HierarchyOptions"))
	{
		if (ImGui::Selectable("Create Empty GameObject"))
		{
			App->editor->selected_GO = App->go_manager->CreateGameObject(NULL);
		}

		if (ImGui::Selectable("Create Empty Child"))
		{
			App->editor->selected_GO = App->go_manager->CreateGameObject(App->editor->selected_GO);
		}

		if (ImGui::Selectable("Remove selected GameObject"))
		{
			if (App->editor->selected_GO != nullptr)
			{
				App->go_manager->RemoveGameObject(App->editor->selected_GO);
				App->editor->selected_GO = nullptr;
			}
		}
		if (ImGui::Selectable("Create Prefab"))
		{
			if (App->editor->selected_GO != nullptr)
			{
				App->resource_manager->SavePrefab(App->editor->selected_GO);
			}
		}

		ImGui::EndPopup();
	}

	ImGui::End();
}

void Hierarchy::DisplayGameObjectsChilds(const std::vector<GameObject*>* childs)
{
	for (std::vector<GameObject*>::const_iterator object = (*childs).begin(); object != (*childs).end(); ++object)
	{
		uint flags = 0;
		if ((*object) == App->editor->selected_GO)
			flags = ImGuiTreeNodeFlags_Selected;

		if ((*object)->ChildCount() > 0)
		{
			if (ImGui::TreeNodeEx((*object)->name.data(), flags))
			{
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					ComponentTransform* transform = (ComponentTransform*)(*object)->GetComponent(C_TRANSFORM);
					App->camera->Center(transform->GetGlobalMatrix().TranslatePart());
				}

				if (ImGui::IsItemClicked(0))
				{
					App->editor->selected_GO = (*object);
				}

				DisplayGameObjectsChilds((*object)->GetChilds());
				ImGui::TreePop();
			}
		}
		else
		{
			if (ImGui::TreeNodeEx((*object)->name.data(), flags | ImGuiTreeNodeFlags_Leaf))
			{
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					ComponentTransform* transform = (ComponentTransform*)(*object)->GetComponent(C_TRANSFORM);
					App->camera->Center(transform->GetGlobalMatrix().TranslatePart());
				}

				if (ImGui::IsItemClicked(0))
				{
					App->editor->selected_GO = (*object);
				}
				ImGui::TreePop();
			}
		}
	}
}