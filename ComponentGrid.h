#ifndef __COMPONENTGRID_H__
#define __COMPONENTGRID_H__

#include "Globals.h"
#include "Component.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include <vector>
class GameObject;

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
	void UpdateFocusObjectsPosition();
	void SetPlayerOrder(int one, int two, int three, int four);
private:
	int players_controlling = 1;
	int num_elements = 0;
	GameObject* grid_focus = nullptr;
	bool grid_enabled = false;
	int elements_x_row = 0;
	int element_width = 0;
	int element_height = 0;
	int margin = 0;
	int space_between_x = 0;
	int space_between_y = 0;
	int rows = 1;
	bool reorganize_grid = false;
	std::vector<GameObject*> focus_objects;
	float4 players_order = float4(0, 1, 2, 3);
	float4 focus_index_player = float4(1, 1, 1, 1);
	float4 child_focus_index = float4(0, 0, 0, 0);
	bool after_load = false;
};

#endif __COMPONENTGRID_H__