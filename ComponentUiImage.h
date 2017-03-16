#ifndef __COMPONENTUIIMAGE_H__
#define __COMPONENTUIIMAGE_H__

#include "Globals.h"
#include "Component.h"

class ComponentMaterial;

class ComponentUiImage : public Component
{

public:

	ComponentUiImage(ComponentType type, GameObject* game_object);

	void Update(float dt);
	void CleanUp();

	void OnInspector(bool debug);

	// Save
	void Save(Data& file)const;
	void Load(Data& conf);

public:
	ComponentMaterial* UImaterial = nullptr;
private:

};

#endif __COMPONENTUIIMAGE_H__