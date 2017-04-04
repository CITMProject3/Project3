#include "ComponentGrid.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleInput.h"
#include "GameObject.h"
#include "ComponentUiButton.h"
#include "ComponentRectTransform.h"
#include "SDL\include\SDL_scancode.h"
#include "imgui\imgui.h"

ComponentGrid::ComponentGrid(ComponentType type, GameObject * game_object) : Component(type, game_object)
{
	focus_objects.push_back(game_object);
	focus_objects.push_back(game_object);
	focus_objects.push_back(game_object);
	focus_objects.push_back(game_object);
}

ComponentGrid::~ComponentGrid()
{
}



void ComponentGrid::Update()
{
	if (after_load)
	{
		for (int i = 0; i < players_controlling; i++)
		{
			std::vector<GameObject*> childs = *game_object->GetChilds();
			int j = 0;
			for (std::vector<GameObject*>::const_iterator it = childs.begin(); it != childs.end(); ++it)
			{
				if (j == child_focus_index[i])
				{
					focus_objects.at(i) = (*it);
					break;
				}
				j++;
			}
		}
		after_load = false;
	}
	if (reorganize_grid)
	{
		OrganizeGrid();
		reorganize_grid = false;
	}
}

void ComponentGrid::CleanUp()
{
}

void ComponentGrid::OnInspector(bool debug)
{
	std::string str = (std::string("Grid") + std::string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##rectTransform");
		}

		if (ImGui::BeginPopup("delete##rectTransform"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

		ImGui::Checkbox("Enable grid control", &grid_enabled);

		if (grid_enabled)
		{
			int tmp = players_controlling;
			if (ImGui::DragInt("Players controlling", &tmp, 1.0f, 1, 4))
			{
				players_controlling = tmp;
				reorganize_grid = true;
			}
			
			for (int i = 0; i < players_controlling; i++)
			{
				str = std::string("Object :"+ focus_objects.at(i)->name + "##" + std::to_string(i));
				ImGui::Text(str.c_str());
				ImGui::SameLine();
				if (ImGui::BeginMenu(std::string("##"+ std::to_string(i)).data()))
				{
						std::vector<GameObject*> childs = *game_object->GetChilds();
						int j = 0;
						for (std::vector<GameObject*>::const_iterator it = childs.begin(); it != childs.end(); ++it)
						{
							if (ImGui::MenuItem((*it)->name.data()))
							{
								focus_objects.at(i) = (*it);
								child_focus_index[i] = j;
							}
							j++;
						}
					ImGui::EndMenu();
				}

			}
		}
		
		int tmp = num_elements;
		if (ImGui::DragInt("Number of elements",&tmp,1.0f,0,10))
		{
			if (tmp < 0)
				tmp = 0;
			num_elements = tmp;
			reorganize_grid = true;
		}

		tmp = rows;
		if (ImGui::DragInt("Rows", &tmp, 1.0f))
		{
			if (tmp <= 0)
				tmp = 1;
			rows = tmp;
			reorganize_grid = true;
		}

		tmp = element_width;
		if (ImGui::DragInt("Element width", &tmp, 1.0f))
		{
			element_width = tmp;
			reorganize_grid = true;
		}

		tmp = element_height;
		if (ImGui::DragInt("Element height", &tmp, 1.0f))
		{
			element_height = tmp;
			reorganize_grid = true;
		}
		
		tmp = margin;
		if (ImGui::DragInt("Margin", &tmp, 1.0f))
		{
			margin = tmp;
			reorganize_grid = true;
		}

		tmp = space_between_x;
		if (ImGui::DragInt("Margin X between elements", &tmp, 1.0f))
		{
			space_between_x = tmp;
			reorganize_grid = true;
		}

		tmp = space_between_y;
		if (ImGui::DragInt("Margin Y between elements", &tmp, 1.0f))
		{
			space_between_y = tmp;
			reorganize_grid = true;
		}
	}
}

void ComponentGrid::OnFocus()
{
	if (grid_focus != nullptr)
	{
		std::vector<Component*> focus_components = *grid_focus->GetComponents();

		for (std::vector<Component*>::const_iterator comp = focus_components.begin(); comp != focus_components.end(); ++comp)
		{
			if ((*comp) != nullptr)
			{
				(*comp)->OnFocus();
			}
		}
	}

	for (int i = 0; i < players_controlling; i++)
	{
		int id = players_order[i];
		if (App->input->GetJoystickButton(id, JOY_BUTTON::DPAD_UP) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_UP)  == KEY_DOWN)
		{
			if (rows > 1 && num_elements > 1)
			{
				focus_index_player[id] -= elements_x_row;
				if (focus_index_player[id]< 0)
					focus_index_player[id]= 0;
				UpdateFocusObjectsPosition();
			}
		}

		if (App->input->GetJoystickButton(id, JOY_BUTTON::DPAD_DOWN) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_DOWN)
		{
			if (rows > 1 && num_elements > 1 )
			{
				focus_index_player[id]+= elements_x_row;
				if (focus_index_player[id]> num_elements)
					focus_index_player[id]= num_elements;
				UpdateFocusObjectsPosition();
			}
		}

		if (App->input->GetJoystickButton(id, JOY_BUTTON::DPAD_RIGHT)  == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN)
		{
			if (num_elements > 1 && focus_index_player[id]< num_elements)
			{
				focus_index_player[id]++;

				if (focus_index_player[id]> num_elements)
					focus_index_player[id]= num_elements;

				UpdateFocusObjectsPosition();
			}
		}

		if (App->input->GetJoystickButton(id, JOY_BUTTON::DPAD_LEFT) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
		{
			if (num_elements > 1)
			{
				focus_index_player[id]--;
				if (focus_index_player[id]< 0)
					focus_index_player[id]= 0;
				UpdateFocusObjectsPosition();

			}
		}

		if (App->input->GetJoystickButton(id, JOY_BUTTON::A) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
		{
			if (num_elements > 1)
			{
				ComponentUiButton* but = (ComponentUiButton*)focus_objects.at(id)->GetComponent(C_UI_BUTTON);
				if (but != nullptr)
				{
					but->OnPress();
				}
			}
		}

	}
	


}

void ComponentGrid::OrganizeGrid()
{
	elements_x_row = num_elements / rows;
	ComponentRectTransform* r_trans = (ComponentRectTransform*)game_object->GetComponent(C_RECT_TRANSFORM);
	r_trans->SetSize(float2(elements_x_row*(element_width + space_between_x + margin) , rows*(element_height + space_between_y + margin)));
	int i_position_x = -margin;
	int i_pos_x_reset = i_position_x;
	int i_position_y = margin;
	
	std::vector<GameObject*> childs = *game_object->GetChilds();
	int elem = 0;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < elements_x_row; j++)
		{
			if (childs.size() > j)
			{
				GameObject* obj = childs.at(elem);

				if (obj != nullptr)
				{
					ComponentRectTransform* r_trans_e = (ComponentRectTransform*)obj->GetComponent(C_RECT_TRANSFORM);

					r_trans_e->SetLocalPos(float2(i_position_x, i_position_y));
					i_position_x += space_between_x + element_width;
				}
			}	
			elem++;
		}
		i_position_x = i_pos_x_reset;
		i_position_y += space_between_y + element_height;
	}
}

void ComponentGrid::Save(Data & file) const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);

	data.AppendInt("players_controlling", players_controlling);
	data.AppendInt("num_elements", num_elements);
	data.AppendInt("grid_enabled", grid_enabled);
	data.AppendInt("element_width", element_width);
	data.AppendInt("element_height", element_height);
	data.AppendInt("margin", margin);
	data.AppendInt("space_between_x", space_between_x);
	data.AppendInt("space_between_y", space_between_y);
	data.AppendInt("rows", rows);
	data.AppendBool("grid_enabled", grid_enabled);
	data.AppendFloat4("child_focus_index", child_focus_index.ptr());
	file.AppendArrayValue(data);
}

void ComponentGrid::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
	players_controlling = conf.GetInt("players_controlling");
	num_elements = conf.GetInt("num_elements");
	grid_enabled = conf.GetBool("grid_enabled");
	element_width = conf.GetInt("element_width");
	element_height = conf.GetInt("element_height");
	margin = conf.GetInt("margin");
	space_between_x = conf.GetInt("space_between_x");
	space_between_y = conf.GetInt("space_between_y");
	rows = conf.GetInt("rows");
	grid_enabled = conf.GetBool("grid_enabled");
	child_focus_index = conf.GetFloat4("child_focus_index");
	after_load = true;
	reorganize_grid = true;
}

void ComponentGrid::SetNumElements(uint num)
{
	if (num != 0)
		num_elements = num;
}

uint ComponentGrid::GetNumElements() const
{
	return num_elements;
}

void ComponentGrid::UpdateFocusObjectsPosition()
{
	std::vector<GameObject*> childs = *game_object->GetChilds();
	for (int i = 0; i < players_controlling; i++)
	{
		GameObject* obj_focus = focus_objects.at(i);
		GameObject* obj_in_grid = childs.at(int(focus_index_player[i]));
		ComponentRectTransform* r_tran_focus = (ComponentRectTransform*)obj_focus->GetComponent(C_RECT_TRANSFORM);
		ComponentRectTransform* r_tran_grid = (ComponentRectTransform*)obj_in_grid->GetComponent(C_RECT_TRANSFORM);
		if (r_tran_focus && r_tran_grid)
		{
			r_tran_focus->SetLocalPos(r_tran_grid->GetLocalPos());
		}
			
	}
}

void ComponentGrid::SetPlayerOrder(int one, int two, int three, int four)
{
	players_order = float4(one, two, three, four);
}
