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
	void OnFocus();
	void OnPress();
	void OnPressId(int i);
	void ChangeState();

	// Save
	void Save(Data& file)const;
	void Load(Data& conf);
	void Reset();
	void ResetId(int i);

	bool GetState() const;
	
public:
	ComponentMaterial* UImaterial = nullptr;
private:

	bool pressed = false;
};

#endif __COMPONENTUIBUTTON_H__
