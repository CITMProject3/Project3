#include "ComponentGrid.h"
#include "ModuleResourceManager.h"
#include "GameObject.h"
#include "ComponentUiButton.h"
#include "ComponentRectTransform.h"
#include "imgui\imgui.h"

ComponentGrid::ComponentGrid(ComponentType type, GameObject * game_object) : Component(type, game_object)
{
}

ComponentGrid::~ComponentGrid()
{
}

void ComponentGrid::Update()
{
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
			int tmp = player_controlling;
			if (ImGui::DragInt("Number of elements", &tmp, 1.0f, 1, 4))
			{
				player_controlling = tmp;
				reorganize_grid = true;
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
}

void ComponentGrid::OrganizeGrid()
{
	int elements_x_row = num_elements / rows;
	ComponentRectTransform* r_trans = (ComponentRectTransform*)game_object->GetComponent(C_RECT_TRANSFORM);
	int i_position_x = -(r_trans->GetRectSize().x/2) + margin;
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
					ComponentRectTransform* r_trans = (ComponentRectTransform*)obj->GetComponent(C_RECT_TRANSFORM);

					r_trans->SetLocalPos(float2(i_position_x, i_position_y));
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

	data.AppendBool("player_controlling", player_controlling);
	data.AppendInt("num_elements", num_elements);
	data.AppendInt("grid_enabled", grid_enabled);
	data.AppendInt("element_width", element_width);
	data.AppendInt("element_height", element_height);
	data.AppendInt("margin", margin);
	data.AppendInt("space_between_x", space_between_x);
	data.AppendInt("space_between_y", space_between_y);
	data.AppendInt("rows", rows);
	file.AppendArrayValue(data);
}

void ComponentGrid::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
	player_controlling = conf.GetInt("player_controlling");
	num_elements = conf.GetInt("num_elements");
	grid_enabled = conf.GetBool("grid_enabled");
	element_width = conf.GetInt("element_width");
	element_height = conf.GetInt("element_height");
	margin = conf.GetInt("margin");
	space_between_x = conf.GetInt("space_between_x");
	space_between_y = conf.GetInt("space_between_y");
	rows = conf.GetInt("rows");
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
