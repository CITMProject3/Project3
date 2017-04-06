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
class ModuleScripting;

class Random;
class EventQueue;

using namespace std; 
enum PLAYER;

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

	bool StartInGame()const;
	void LoadScene(const char* path);

	void OnStop();
	void OnPlay();

private:

	void AddModule(Module* mod);
	void PrepareUpdate();
	void FinishUpdate();

	//Game States
	void RunGame();
	void StopGame();	
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
	ModuleScripting* scripting;
	ModuleEditor* editor;

	Random* rnd = nullptr;
	EventQueue *event_queue = nullptr;

private:

	vector<Module*> list_modules;
	int max_fps = 60;
	int last_fps = 60;
	int capped_ms = -1;

	bool start_in_game = false;
	GameStates game_state = GAME_STOP;

	bool want_to_load = false;
	char* scene_to_load = "";

};

extern Application* App;

#endif 