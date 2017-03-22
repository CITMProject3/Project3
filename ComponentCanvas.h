#ifndef __COMPONENTCANVAS_H__
#define __COMPONENTCANVAS_H__

#include "Component.h"
class GameObject;
class RaceTimer;
class ComponentCar;
class ComponentUiText;

class ComponentCanvas : public Component
{

public:

	ComponentCanvas(ComponentType type, GameObject* game_object);
	~ComponentCanvas();
	void Update();
	void OnPlay();
	void OnInspector(bool debug);
	
	// Save
	void Save(Data& file)const;
	void Load(Data& conf);
	void Remove();
	void ChangeScene(int scene);
	void SetPlayerReady(int num_player, bool ready);
	void SetWin(bool win);
	vector<GameObject*> GetUI();
private:
	void OnChangeScene();
	vector<GameObject*> GetGameObjectChilds(GameObject* go);
	GameObject* go_focus = nullptr;
	int current_scene = 1;
	int scene_to_change = 0;
	bool player_1_ready = false;
	bool player_2_ready = false;
	bool restart = false;
	bool win = false;
	int kmh = 0;
	RaceTimer* r_timer = nullptr;
	ComponentCar* current_car = nullptr;
	ComponentUiText* play_timer = nullptr;
	ComponentUiText* win_timer = nullptr;
	ComponentUiText* kmh_text = nullptr;
};

#endif __COMPONENTCANVAS_H__