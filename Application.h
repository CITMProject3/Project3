#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "Globals.h"

#include <vector>

class Module;

class ModuleCamera3D;
class ModuleFileSystem;
class ModuleGOManager;
class ModuleInput;
class ModuleLighting;
class ModuleSceneIntro;
class ModulePhysics3D;
class ModuleRenderer3D;
class ModuleResourceManager;
class ModuleEditor;
class ModuleWindow;
class ModuleAudio;

class Random;

using namespace std; 

enum GameStates
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

	bool ChangeGameState(GameStates new_state);
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
	ModuleEditor* editor;

	Random* rnd = nullptr;

private:

	vector<Module*> list_modules;
	int fps = 60;
	int capped_ms = -1;

	GameStates game_state = GAME_STOP;
};

extern Application* App;

#endif 