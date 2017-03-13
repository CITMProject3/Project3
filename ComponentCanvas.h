#ifndef __COMPONENTCANVAS_H__
#define __COMPONENTCANVAS_H__

#include "Component.h"

class ComponentCanvas : public Component
{

public:

	ComponentCanvas(ComponentType type, GameObject* game_object);

	void Update();

	void OnInspector(bool debug);

	// Save
	void Save(Data& file)const;
	void Load(Data& conf);

private:
	
	GameObject* go_focus = nullptr;

};

#endif __COMPONENTCANVAS_H__