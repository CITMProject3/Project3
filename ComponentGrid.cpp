#include "ComponentGrid.h"
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
	std::string str = (std::string("Rect Transform") + std::string("##") + std::to_string(uuid));
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

		int tmp = num_elements;
		if (ImGui::DragInt("Number of elements",&tmp,1.0f,0,10))
		{
			num_elements = tmp;
			reorganize_grid = true;
		}

		tmp = rows;
		if (ImGui::DragInt("Rows", &tmp, 1.0f))
		{
			rows = tmp;
			reorganize_grid = true;
		}

		tmp = element_width;
		if (ImGui::DragInt("Element width", &tmp, 1.0f))
		{
			element_width = tmp;
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
	int i_position_y = margin;
	
	std::vector<GameObject*> childs = *game_object->GetChilds();
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < elements_x_row; j++)
		{
			GameObject* obj = childs.at(j*i);

			if (obj != nullptr)
			{
				ComponentRectTransform* r_trans = (ComponentRectTransform*)obj->GetComponent(C_RECT_TRANSFORM);

				r_trans->SetLocalPos(float2(i_position_x, i_position_y));
				i_position_x += space_between_x + element_width;
				i_position_y += space_between_y + element_height;
			}
		}
	}
}

void ComponentGrid::Save(Data & file) const
{
}

void ComponentGrid::Load(Data & conf)
{
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
