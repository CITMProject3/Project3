#ifndef __COMPONENTCANVAS_H__
#define __COMPONENTCANVAS_H__

#include "Component.h"
class GameObject;
class RaceTimer;
class ComponentCar;
class ComponentUiText;
class ComponentUiButton;

class ComponentCanvas : public Component
{

public:

	ComponentCanvas(ComponentType type, GameObject* game_object);
	~ComponentCanvas();
	void Update();
	void OnPlay();
	void OnInspector(bool debug);
	void OnTransformModified();
	void ResizeScreen();
	// Save
	void Save(Data& file)const;
	void Load(Data& conf);
	void Remove();
	vector<GameObject*> GetUI();
	vector<GameObject*> GetGoFocus()const;
	
	void AddGoFocus(GameObject* new_focus);
	void RemoveGoFocus(GameObject* new_focus);
	void ClearFocus();
private:

	vector<GameObject*> GetGameObjectChilds(GameObject* go);
	vector<GameObject*> go_focus;
	int current_width = 1600;
	int current_height = 900;
};

#endif __COMPONENTCANVAS_H__