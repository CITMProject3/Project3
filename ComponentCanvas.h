#ifndef __COMPONENTCANVAS_H__
#define __COMPONENTCANVAS_H__

#include "Component.h"
class GameObject;

class ComponentCanvas : public Component
{

public:

	ComponentCanvas(ComponentType type, GameObject* game_object);
	~ComponentCanvas();
	void Update();

	void OnInspector(bool debug);
	
	// Save
	void Save(Data& file)const;
	void Load(Data& conf);
	void Remove();

	vector<GameObject*> GetUI();
private:
	vector<GameObject*> GetGameObjectChilds(GameObject* go);
	GameObject* go_focus = nullptr;

};

#endif __COMPONENTCANVAS_H__