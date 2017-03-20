#include "Hierarchy.h"
#include "Application.h"
#include "Editor.h"

#include "GameObject.h"
#include "Component.h"
#include "ComponentTransform.h"
#include "ComponentLight.h"

#include "ModuleGOManager.h"
#include "LayerSystem.h"

#include "ModuleInput.h"
#include "ModuleResourceManager.h"
#include "ModuleCamera3D.h"

Hierarchy::Hierarchy()
{
	active = true;
	flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
}
Hierarchy::~Hierarchy()
{}

void Hierarchy::Draw()
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
		if (ImGui::IsMouseClicked(0))// && been_selected == false)
		{
			if (setting_parent == true)
			{
				if (App->editor->selected.size() > 0)
				{
					parent_to_set = App->go_manager->root;
					child_to_set = App->editor->selected.back();
					set_parent_now = true;
					App->editor->UnselectAll();
				}
			}
		}
	}

	if (ImGui::BeginPopup("HierarchyOptions"))
	{
		if (ImGui::MenuItem("Copy"))
		{
			if (App->editor->selected.size() > 0)
				App->editor->Copy(App->editor->selected.back());
		}
		if (ImGui::MenuItem("Paste"))
		{
			if (App->editor->selected.size() > 0)
				App->editor->Paste(App->editor->selected.back());
			else
				App->editor->Paste(nullptr);
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Duplicate"))
		{
			if (App->editor->selected.size() > 0)
				App->editor->Duplicate(App->editor->selected.back());
		}

		if (ImGui::MenuItem("Delete"))
		{
			App->editor->RemoveSelected();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Create Prefab"))
		{
			if (App->editor->selected.size() > 0)
			{
				App->resource_manager->SavePrefab(App->editor->selected.back());
			}
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Create Empty"))
		{
			GameObject* game_object = (App->editor->selected.size() > 0) ? App->editor->selected.back() : nullptr;
			App->editor->SelectSingle(App->go_manager->CreateGameObject(game_object));
		}
		if (ImGui::BeginMenu("3D Object"))
		{
			if (ImGui::MenuItem("Cube"))
				App->go_manager->CreatePrimitive(PrimitiveType::P_CUBE);
			
			if (ImGui::MenuItem("Sphere"))
				App->go_manager->CreatePrimitive(PrimitiveType::P_SPHERE);
			
			if (ImGui::MenuItem("Plane"))
				App->go_manager->CreatePrimitive(PrimitiveType::P_PLANE);

			if (ImGui::MenuItem("Cylinder"))
				App->go_manager->CreatePrimitive(PrimitiveType::P_CYLINDER);
			
			if (ImGui::BeginMenu("Light"))
			{
				if (ImGui::MenuItem("Directional Light"))
				{
					App->go_manager->CreateLight(nullptr, DIRECTIONAL_LIGHT);
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("Set Parent"))
		{
			setting_parent = true;
		}

		ImGui::EndPopup();
	}
	
	// Quick test for unselecting game objects when not selecting none
	int size_x = current_size.x;
	int size_y = current_size.y - ImGui::GetCursorPosY() - 10;
	ImGui::SetCursorPosX(0);
	if (ImGui::InvisibleButton("test: unselect GO", ImVec2(size_x, size_y)))
	{
		App->editor->UnselectAll();
	}
	if (set_parent_now == true)
	{
		set_parent_now = false;
		setting_parent = false;
		child_to_set->SetParent(parent_to_set);
		parent_to_set = child_to_set = nullptr;
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
		ImGui::PushID((*object)->GetUUID());
		bool open = ImGui::TreeNodeEx((*object)->name.data(), flags);
		bool hasChilds = (*object)->ChildCount() > 0;
		ImGui::PopID();
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
		{
			App->camera->Center((*object)->transform->GetGlobalMatrix().TranslatePart());
		}

		if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
		{
			if (setting_parent == true)
			{
				if (App->editor->selected.size() > 0)
				{
					parent_to_set = *object;
					child_to_set = App->editor->selected.back();
					set_parent_now = true;
					App->editor->UnselectAll(); 
					break;
				}
				setting_parent = false;
			}
			else if (App->editor->assign_wheel != -1)
			{
				App->editor->wheel_assign = *object;
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