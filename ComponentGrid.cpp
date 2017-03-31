#include "ComponentGrid.h"
#include "GameObject.h"
#include "ComponentUiButton.h"

ComponentGrid::ComponentGrid(ComponentType type, GameObject * game_object) : Component(type, game_object)
{
}

ComponentGrid::~ComponentGrid()
{
}

void ComponentGrid::Update()
{
}

void ComponentGrid::CleanUp()
{
}

void ComponentGrid::OnInspector(bool debug)
{
}

void ComponentGrid::OnFocus()
{
	if (grid_focus != nullptr)
	{
		ComponentUiButton* grid = (ComponentUiButton*)grid_focus->GetComponent(C_UI_BUTTON);
		if (grid != nullptr)
		{
			grid->OnFocus();
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
