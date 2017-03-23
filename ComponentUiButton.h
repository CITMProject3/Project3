#ifndef __COMPONENTUIBUTTON_H__
#define __COMPONENTUIBUTTON_H__

#include "Globals.h"
#include "Component.h"

class ComponentMaterial;

class ComponentUiButton : public Component
{

public:

	ComponentUiButton(ComponentType type, GameObject* game_object);
	~ComponentUiButton();

	void Update();
	void CleanUp();

	void OnInspector(bool debug);

	// Save
	void Save(Data& file)const;
	void Load(Data& conf);

public:
	ComponentMaterial* UImaterial = nullptr;
private:
	uint player_num = 1;
	bool ready = false;
};

#endif __COMPONENTUIBUTTON_H__
