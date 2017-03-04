#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <list>
#include "Globals.h"
#include "Timer.h"
#include "ModuleSceneIntro.h"
#include "Editor.h"
#include "DebugDraw.h"
#include "AutoProfile.h"
#include "Random.h"
#include "Time.h"
#include <string>
#include <stdlib.h>

class Module;
class ModuleCamera3D;
class ModuleCar;
class ModuleFileSystem;
class ModuleGOManager;
class ModuleInput;
class ModuleLighting;
class ModulePhysics3D;
class ModuleRenderer3D;
class ModuleResourceManager;
class ModuleWindow;
class ModuleAudio;

using namespace std; 

enum game_states
{
	GAME_STOP,
	GAME_RUNNING,
	GAME_PAUSED,
	GAME_NEXT_FRAME
};

class Application
{
public:

	Application();
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();
	void SaveBeforeClosing();

	void OpenURL(const char* url);
	void SetMaxFPS(int max_fps);
	int GetFPS();

	bool ChangeGameState(game_states new_state);
	bool IsGameRunning()const;
	bool IsGamePaused()const;
private:

	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();

	//Game States
	void StopGame();
	void RunGame();
	void PauseGame();

public:
	ModuleWindow* window;
	ModuleInput* input;
	ModuleAudio* audio;
	ModuleSceneIntro* scene_intro;
	ModuleRenderer3D* renderer3D;
	ModuleCamera3D* camera;
	ModulePhysics3D* physics;
	ModuleFileSystem* file_system;
	ModuleGOManager* go_manager;
	ModuleResourceManager* resource_manager;
	ModuleLighting* lighting;
	Editor* editor;
	Random* rnd = nullptr;
	ModuleCar* car = nullptr;
private:


	vector<Module*> list_modules;
	int fps = 60;
	int capped_ms = -1;

	game_states game_state = GAME_STOP;
};

extern Application* App;

#endif 