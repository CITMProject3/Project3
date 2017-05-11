#include "Application.h"

#include "Module.h"
#include "ModuleScripting.h"
#include "ModuleAudio.h"
#include "ModuleCamera3D.h"
#include "ModuleFileSystem.h"
#include "ModuleGOManager.h"
#include "ModuleInput.h"
#include "ModuleSceneIntro.h"
#include "ModuleLighting.h"
#include "ModulePhysics3D.h"
#include "ModuleRenderer3D.h"
#include "ModuleResourceManager.h"
#include "ModuleEditor.h"
#include "ModuleWindow.h"

#include "Time.h"
#include "Random.h"
#include "EventQueue.h"
#include "Data.h"

#include "Brofiler/include/Brofiler.h"

#include "ComponentCar.h"

using namespace std;

Application::Application()
{
	// Time controller
	time = new Time();
	
	// Random
	rnd = new Random();

	// EventQueue
	event_queue = new EventQueue();

	// Modules
	window = new ModuleWindow("window");
	resource_manager = new ModuleResourceManager("resource_manager");
	input = new ModuleInput("input");
	audio = new ModuleAudio("audio");
	scene_intro = new ModuleSceneIntro("scene_intro");
	renderer3D = new ModuleRenderer3D("renderer");
	camera = new ModuleCamera3D("camera");
	physics = new ModulePhysics3D("physics");
	scripting = new ModuleScripting("scripting");
	editor = new ModuleEditor("editor");
	file_system = new ModuleFileSystem("file_system");
	go_manager = new ModuleGOManager("go_manager");
	lighting = new ModuleLighting("lighting");

	//Globals
	g_Debug = new DebugDraw("debug_draw");

	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	// Main Modules
	AddModule(file_system);
	AddModule(resource_manager);
	AddModule(window);
	AddModule(input);
	AddModule(g_Debug);
	AddModule(audio);
	AddModule(scripting);
	AddModule(physics);
	AddModule(go_manager);
	AddModule(camera);	
	AddModule(lighting);
	
	// Scenes
	AddModule(scene_intro);

	//Editor
	AddModule(editor);

	// Renderer last!
	AddModule(renderer3D);
}

Application::~Application()
{
	delete rnd;
	delete event_queue;

	vector<Module*>::reverse_iterator i = list_modules.rbegin();

	while (i != list_modules.rend())
	{
		delete (*i);
		++i;
	}

	delete time;
}

bool Application::Init()
{
	bool ret = true;

	//Load Configuration
	char* buffer = nullptr;
	if (App->file_system->Load("Configuration.json", &buffer) == 0)
	{
		LOG("Error while loading Configuration file");
		//Create a new Configuration file
		if (buffer)
			delete[] buffer;

		Data root_node;
		root_node.AppendBool("start_in_game", false);
		vector<Module*>::reverse_iterator i = list_modules.rbegin();

		while (i != list_modules.rend())
		{
			root_node.AppendJObject((*i)->GetName());
			++i;
		}
	if (App->StartInGame() == false)
		{
			size_t size = root_node.Serialize(&buffer);	
			App->file_system->Save("Configuration.json", buffer, size);
		}
	}
	Data config(buffer);
	delete[] buffer;

	// Game is initialized in PlayMode?
	if (config.GetBool("start_in_game"))
	{
		start_in_game = true;
		game_state = GAME_RUNNING;
	}		

	// Call Init() in all modules
	vector<Module*>::iterator i = list_modules.begin();

	while (i != list_modules.end() && ret == true)
	{
		ret = (*i)->Init(config.GetJObject((*i)->GetName()));
		++i;
	}

	// After all Init calls we call Start() in all modules
	LOG("Application Start --------------");
	i = list_modules.begin();

	while(i != list_modules.end() && ret == true)
	{
		ret = (*i)->Start();
		++i;
	}

	capped_ms = 1000 / max_fps;

	//// Play all Components of every GameObject on the scene
	if (start_in_game)
	{		
		time->Play();

		for (std::vector<Module*>::iterator it = list_modules.begin(); it != list_modules.end(); it++)
			(*it)->OnPlay();
	}	
	
	return ret;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	time->UpdateFrame();
}

// ---------------------------------------------
void Application::FinishUpdate()
{
	event_queue->ProcessEvents();
	if (want_to_load == true)
	{
		want_to_load = false;
		resource_manager->LoadSceneFromAssets(scene_to_load);
	}
}

void Application::LoadScene(const char* path)
{
	if (want_to_load == false)
	{
		scene_to_load = (char*)path;
		want_to_load = true;
	}
}

void Application::OnStop()
{
	for (std::vector<Module*>::iterator it = list_modules.begin(); it != list_modules.end(); it++)
	{
		(*it)->OnStop();
	}
}

void Application::OnPlay()
{
	for (std::vector<Module*>::iterator it = list_modules.begin(); it != list_modules.end(); it++)
		(*it)->OnPlay();
}

void Application::RunGame() 
{
	//Save current scene only if the game was stopped
	if (game_state == GAME_STOP)
		go_manager->SaveSceneBeforeRunning();

	game_state = GAME_RUNNING;
	time->Play();

	OnPlay();
}

void Application::PauseGame()
{
	game_state = GAME_PAUSED;
	time->Pause();

	for (std::vector<Module*>::iterator it = list_modules.begin(); it != list_modules.end(); it++)
	{
		(*it)->OnPause();
	}
}

void Application::StopGame()
{
	game_state = GAME_STOP;
	OnStop();
	go_manager->LoadSceneBeforeRunning();
	time->Stop();	
}

// Call PreUpdate, Update and PostUpdate on all modules
update_status Application::Update()
{
	BROFILER_FRAME("GameLoop")

	update_status ret = UPDATE_CONTINUE;
	PrepareUpdate();
	
	vector<Module*>::iterator i = list_modules.begin();

	while (i != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*i)->PreUpdate();
		++i;
	}

	i = list_modules.begin();

	while(i != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*i)->Update();
		++i;
	}

	i = list_modules.begin();

	while (i != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*i)->PostUpdate();
		i++;
	}

	FinishUpdate();
	return ret;
}

bool Application::CleanUp()
{
	bool ret = true;

	vector<Module*>::reverse_iterator i = list_modules.rbegin();

	while (i != list_modules.rend() && ret == true)
	{
		ret = (*i)->CleanUp();
		++i;
	}

	return ret;
}

void Application::SaveBeforeClosing()
{
	Data root_node;
	char* buf;

	root_node.AppendBool("start_in_game", start_in_game);

	vector<Module*>::reverse_iterator i = list_modules.rbegin();

	while (i != list_modules.rend())
	{
		(*i)->SaveBeforeClosing(root_node.AppendJObject((*i)->GetName()));
		++i;
	}

	size_t size = root_node.Serialize(&buf);
	if (App->StartInGame() == false)
	{
		uint success = App->file_system->Save("Configuration.json", buf, size);
	
		if (success == 0)
			LOG("Configuration could not be saved before closing");
	}

	delete[] buf;
}

void Application::AddModule(Module* mod)
{
	list_modules.push_back(mod);
}

void Application::OpenURL(const char* url)
{
	ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}

void Application::SetMaxFPS(int max_fps)
{
	this->max_fps = max_fps;
	if (max_fps == 0) this->max_fps = -1;
	capped_ms = 1000 / max_fps;
}

int Application::GetFPS()
{
	return 60; //TODO: Update time with fps limit.
}

bool Application::ChangeGameState(GameStates new_state)
{
	bool success = false;
	switch (new_state)
	{
	case GAME_STOP:
		if (game_state == GAME_RUNNING || game_state == GAME_PAUSED)
		{
			StopGame();
			success = true;
		}
		break;
	case GAME_RUNNING:
		if (game_state == GAME_STOP || game_state == GAME_PAUSED)
		{
			RunGame();
			success = true;
		}
		break;
	case GAME_PAUSED:
		if (game_state == GAME_RUNNING || game_state == GAME_NEXT_FRAME)
		{
			PauseGame();
			success = true;
		}
		break;
	case GAME_NEXT_FRAME:
		if(game_state == GAME_RUNNING || game_state == GAME_PAUSED) //TODO: Now this features is not available yet. Nothing happens in the game now. 
			//NextFrameGame();
		break;
	}

	return success;
}

///Returns true if the game simulation has started. If the game is paused also returns true.
bool Application::IsGameRunning() const
{
	return (game_state == GAME_RUNNING || game_state == GAME_NEXT_FRAME || game_state == GAME_PAUSED) ? true : false;
}

///Returns true if the game is paused or in next frame mode. If the game is stop returns false.
bool Application::IsGamePaused() const
{
	return (game_state == GAME_PAUSED || game_state == GAME_NEXT_FRAME) ? true : false;
}

bool Application::StartInGame() const
{
	return start_in_game;
}
