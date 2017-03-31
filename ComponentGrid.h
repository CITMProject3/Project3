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

	// Save
	void Save(Data& file)const;
	void Load(Data& conf);

	void SetNumElements(uint num);
	uint GetNumElements() const;

private:
	uint num_elements = 0;
	GameObject* grid_focus = nullptr;
};

#endif __COMPONENTGRID_H__