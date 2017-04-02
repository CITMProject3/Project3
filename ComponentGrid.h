#ifndef __COMPONENTGRID_H__
#define __COMPONENTGRID_H__

#include "Globals.h"
#include "Component.h"

class ComponentGrid : public Component
{

public:

	ComponentGrid(ComponentType type, GameObject* game_object);
	~ComponentGrid();

	void Update();
	void CleanUp();

	void OnInspector(bool debug);
	void OnFocus();

	void OrganizeGrid();
	// Save
	void Save(Data& file)const;
	void Load(Data& conf);

	void SetNumElements(uint num);
	uint GetNumElements() const;

private:
	int player_controlling = 1;
	int num_elements = 0;
	GameObject* grid_focus = nullptr;
	bool grid_enabled = false;
	int element_width = 0;
	int element_height = 0;
	int margin = 0;
	int space_between_x = 0;
	int space_between_y = 0;
	int rows = 1;
	bool reorganize_grid = false;
};

#endif __COMPONENTGRID_H__