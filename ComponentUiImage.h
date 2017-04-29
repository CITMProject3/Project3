#ifndef __COMPONENTUIIMAGE_H__
#define __COMPONENTUIIMAGE_H__

#include "Globals.h"
#include "Component.h"

class ComponentMaterial;

class ComponentUiImage : public Component
{

public:

	ComponentUiImage(ComponentType type, GameObject* game_object);
	~ComponentUiImage();

	void Update();
	void CleanUp();

	void OnInspector(bool debug);
	
	// Save
	void Save(Data& file)const;
	void Load(Data& conf);

public:
	ComponentMaterial* UImaterial = nullptr;
	int id_to_render = 0;

};

#endif __COMPONENTUIIMAGE_H__