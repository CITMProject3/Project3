#include "Application.h"
#include "Editor.h"
#include "Imgui\imgui.h"
#include "Random.h"
#include "FPSGraph.h"
#include "WindowOptions.h"
#include "HardwareInfo.h"
#include "Console.h"
#include "Assets.h"
#include "Hierarchy.h"
#include "Inspector.h"
#include "Profiler.h"
#include "DebugDraw.h"
#include "CameraWindow.h"
#include "Time.h"
#include "ResourcesWindow.h"
#include "MaterialCreatorWindow.h"
#include "ShaderEditorWindow.h"
#include "LightingWindow.h"
#include "LayersWindow.h"
#include "RenderTexEditorWindow.h"
#include "ModuleCamera3D.h"
#include "ComponentCamera.h"
#include "TestWindow.h"
#include "GameObject.h"
#include "RaycastHit.h"

Editor::Editor(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	windows.push_back(console = new Console()); //Create console in the constructor to get ALL init logs from other modules.
}

Editor::~Editor()
{}

bool Editor::Init(Data & config)
{
	//TODO: move into parameter configuration setup (like window color)

	//Window rounding
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

	//Child window background
	ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(0.3, 0.3, 0.3, 0.3));

	return true;
}

bool Editor::Start()
{
	bool ret = true;

	LOG("Start Editor");

	//Create Windows
	windows.push_back(&g_Profiler);
	windows.push_back(fps_graph_win = new FPSGraph());
	windows.push_back(winoptions_win = new WindowOptions());
	windows.push_back(hardware_win = new HardwareInfo());
	windows.push_back(assets = new Assets());
	windows.push_back(hierarchy = new Hierarchy());
	windows.push_back(inspector = new Inspector());
	windows.push_back(camera_win = new CameraWindow());
	windows.push_back(resource_win = new ResourcesWindow());
	windows.push_back(material_creator_win = new MaterialCreatorWindow());
	windows.push_back(shader_editor_win = new ShaderEditorWindow());
	windows.push_back(lighting_win = new LightingWindow());
	windows.push_back(layers_win = new LayersWindow());
	windows.push_back(rendertex_win = new RenderTexEditorWindow());
	windows.push_back(test_win = new TestWindow());

	InitSizes();

	//Testing
	skybox.Init("Resources/Skybox/s_left.dds", "Resources/Skybox/s_right.dds", "Resources/Skybox/s_up.dds", "Resources/Skybox/s_down.dds", "Resources/Skybox/s_front.dds", "Resources/Skybox/s_back.dds");

	return ret;
}

bool Editor::CleanUp()
{
	LOG("Clean Up Editor");

	delete fps_graph_win;
	delete winoptions_win;
	delete hardware_win;
	delete assets;
	delete camera_win;
	delete resource_win;
	delete material_creator_win;
	delete shader_editor_win;
	delete lighting_win;
	delete layers_win;
	delete rendertex_win;

	windows.clear();

	return true;
}

string Editor::GetAssetsCurrentDir() const
{
	return assets->CurrentDirectory();
}

void Editor::RefreshAssets() const
{
	if (assets)
		assets->Refresh();
}

void Editor::InitSizes()
{
	hierarchy->SetRelativeDimensions(ImVec2(0, 0.0), ImVec2(0.15, 0.8));
	inspector->SetRelativeDimensions(ImVec2(0.80, 0.0), ImVec2(0.20, 0.8));
	assets->SetRelativeDimensions(ImVec2(0, 0.8), ImVec2(1.0, 0.2));
}

void Editor::OnResize(int screen_width, int screen_height)
{
	vector<Window*>::iterator win = windows.begin();
	while (win != windows.end())
	{
		(*win)->OnResize(screen_width, screen_height);
		++win;
	}
}

bool Editor::UsingKeyboard() const
{
	return using_keyboard;
}

bool Editor::UsingMouse() const
{
	return using_mouse;
}

void Editor::SelectSingle(GameObject* game_object)
{
	UnselectAll();
	selected.push_back(game_object);
}

void Editor::AddSelect(GameObject* game_object)
{
	//Just for safety
	if (IsSelected(game_object) == false)
		selected.push_back(game_object);
}

void Editor::Unselect(GameObject* game_object)
{
	std::list<GameObject*>::iterator it = selected.begin();
	while (it != selected.end())
	{
		if (*it == game_object)
		{
			selected.erase(it);
			break;
		}
		it++;
	}
}

void Editor::UnselectAll()
{
	selected.clear();
}

bool Editor::IsSelected(GameObject* game_object) const
{
	std::list<GameObject*>::const_iterator it = selected.begin();
	while (it != selected.end())
	{
		if (*it == game_object)
		{
			return true;
		}
		it++;
	}
	return false;
}

void Editor::RemoveSelected()
{
	std::list<GameObject*>::const_iterator it = selected.begin();
	while (it != selected.end())
	{
		App->go_manager->RemoveGameObject(*it);
		it++;
	}
	selected.clear();
}

void Editor::Copy(GameObject* game_object)
{
	if (copy_go != nullptr)
		delete copy_go;

	copy_go = new GameObject(*game_object);
}

void Editor::Paste(GameObject* game_object)
{
	GameObject* new_go = App->go_manager->CreateGameObject(game_object->GetParent(), copy_go);
}

void Editor::Duplicate(GameObject* game_object)
{
	Copy(game_object);
	Paste(game_object->GetParent());
}

update_status Editor::PreUpdate()
{
	using_keyboard = ImGui::GetIO().WantCaptureKeyboard;
	using_mouse = ImGui::GetIO().WantCaptureMouse;

	return UPDATE_CONTINUE;
}

update_status Editor::Update()
{
	PROFILE("Editor::Update()");

	update_status ret = UPDATE_CONTINUE;

	GameOptions(); //Play/Stop/Next Frame buttons

	ret = EditorWindows(); //Update the windows of the editor
	
	//Draw Grid
	if (!disable_grid)
	{
		Plane_P grid(0, 1, 0, 0);
		grid.axis = true;
		grid.Render();
	}
	
	HandleInput();

	//Handle Quit event
	bool quit = false;
	if (App->input->Quit())
		quit = QuitWindow();

	if (quit)
		ret = UPDATE_STOP;

	return ret;	
}

void Editor::HandleInput()
{
	//Shortcut to save. TODO: Do a better implementation of the shortcuts
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
		save_scene_win = true;

	if (App->input->GetKey(SDL_SCANCODE_DELETE) == KEY_DOWN)
	{
		RemoveSelected();
	}

	//GameObject selection (click and drag)
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		select_dragging = true;
		start_drag.x = App->input->GetMouseX();
		start_drag.y = App->input->GetMouseY();
	}

	if (select_dragging = true && App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_UP)
	{
		ImVec2 end_drag(App->input->GetMouseX(), App->input->GetMouseY());
		if (start_drag.x == end_drag.x && start_drag.y == end_drag.y)
		{
			Ray ray = App->camera->GetEditorCamera()->CastCameraRay(float2(App->input->GetMouseX(), App->input->GetMouseY()));
			//TODO:(Ausiàs) change game_object for a list
			GameObject* game_object = App->go_manager->Raycast(ray, true).object;

			if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT ||
				App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT)
			{
				IsSelected(game_object) ? Unselect(game_object) : AddSelect(game_object);
			}
			else
			{
				SelectSingle(App->go_manager->Raycast(ray, true).object);
			}
		}
	}
}

void Editor::GameOptions() const
{
	ImGui::SetNextWindowPos(ImVec2(App->window->GetScreenWidth()/2, 30));
	bool open = true;
	ImGui::Begin("##GameOptions", &open, ImVec2(0, 0), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

	if (App->IsGameRunning() == false || App->IsGamePaused())
	{
		if (ImGui::Button("Play ##gameoptions_play"))
			App->ChangeGameState(GAME_RUNNING);
	}
	else
	{
		if (ImGui::Button("Stop ##gameoptions_stop"))
			App->ChangeGameState(GAME_STOP);
	}
	
	ImGui::SameLine();
	if (ImGui::Button("Pause ##gameoptions_pause"))
	{
		App->ChangeGameState(GAME_PAUSED);
	}
	ImGui::SameLine();
	ImGui::Button("Next ##gameoptions_next");
	ImGui::SameLine();
	int time_game_running = time->TimeSinceGameStartup();
	ImGui::Text("Game time: %i", time_game_running);
	ImGui::End();
}

update_status Editor::EditorWindows()
{
	update_status ret = UPDATE_CONTINUE;

	//Main MenuBar --------------------------------------------------------------------------------------------
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			FileMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			EditMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Windows"))
		{
			WindowsMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			HelpMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Debug"))
		{
			DebugMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Quit"))
		{
			ret = UPDATE_STOP;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	//Windows ----------------------------------------------------------------------------------------------------
	vector<Window*>::iterator win = windows.begin();
	while (win != windows.end())
	{
		PROFILE("Editor::Update-PaintWindows");
		(*win)->Draw();
		++win;
	}

	SaveSceneWindow();
	
	return ret;
}


void Editor::FileMenu()
{
	if (ImGui::MenuItem("Create New Scene"))
	{
		App->go_manager->LoadEmptyScene();
	}
}

//Menus -----------------------------------------------------------------------------------------------
void Editor::HelpMenu()
{
	if (ImGui::MenuItem("Documentation"))
		App->OpenURL("https://github.com/traguill/Ezwix-Engine/wiki");

	if (ImGui::MenuItem("Download latest build"))
		App->OpenURL("https://github.com/traguill/Ezwix-Engine/releases");

	if (ImGui::MenuItem("Report a bug"))
		App->OpenURL("https://github.com/traguill/Ezwix-Engine/issues");

	if (ImGui::MenuItem("About"))
	{
		ImGui::OpenPopup("##1 About");	
		if (ImGui::BeginPopup("##1 About"))
		{
			ImGui::Text("Ezwix Engine");
			ImGui::Text("A 3D engine for learning purposes.");
			ImGui::Text("Autor: Guillem Travila");
			ImGui::Text("Libraries: SDL 2.0, ImGui, MathGeoLib, Bullet, OpenGl 3.2");
			ImGui::Text("License"); //TODO

			ImGui::EndPopup();
		}
	}
}

void Editor::WindowsMenu()
{
	if (ImGui::BeginMenu("Configuration"))
	{
		if (ImGui::MenuItem("FPS Graph"))
		{
			fps_graph_win->SetActive(true);
		}
		if (ImGui::MenuItem("Window Options"))
		{
			winoptions_win->SetActive(true);
		}
		if (ImGui::MenuItem("Hardware Info"))
		{
			hardware_win->SetActive(true);
		}

		ImGui::EndMenu();
	}

	if (ImGui::MenuItem("Console"))
	{
		console->SetActive(true);
	}
	
	if (ImGui::MenuItem("Profiler"))
	{
		g_Profiler.SetActive(true);
	}

	if (ImGui::MenuItem("Assets"))
	{
		assets->SetActive(true);
	}

	if (ImGui::MenuItem("Resources"))
	{
		resource_win->SetActive(true);
	}

	if (ImGui::MenuItem("Material Creator"))
	{
		material_creator_win->SetActive(true);
	}

	if (ImGui::MenuItem("Hierarchy"))
	{
		hierarchy->SetActive(true);
	}

	if (ImGui::MenuItem("Inspector"))
	{
		inspector->SetActive(true);
	}

	if (ImGui::MenuItem("Test Window"))
	{
		test_win->SetActive(true);
	}
}

void Editor::EditMenu()
{
	if (ImGui::MenuItem("Camera"))
	{
		camera_win->SetActive(true);
	}

	if (ImGui::MenuItem("Shaders"))
	{
		shader_editor_win->SetActive(true);
	}

	if (ImGui::MenuItem("Lighting"))
	{
		lighting_win->SetActive(true);
	}

	if (ImGui::MenuItem("Layers"))
	{
		layers_win->SetActive(true);
	}

	if (ImGui::MenuItem("RenderTexture"))
	{
		rendertex_win->SetActive(true);
	}

	ImGui::MenuItem("Disable grid", NULL, &disable_grid);
	
}

void Editor::DebugMenu()
{
	if (ImGui::MenuItem("Show/Hide Octree"))
	{
		App->go_manager->draw_octree = !App->go_manager->draw_octree;
	}
}

bool Editor::QuitWindow()
{
	bool ret = false;

	//Show confirmation window
	const SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Save" },
		{ /*input*/								0,1, "Don't Save" },
		{										0, 2, "Cancel" },
	};
	const SDL_MessageBoxColorScheme colorScheme = 
	{
		{ 
		  /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
			{ 255,   0,   0 },
			/* [SDL_MESSAGEBOX_COLOR_TEXT] */
			{ 0, 255,   0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
			{ 255, 255,   0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
			{ 0,   0, 255 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
			{ 255,   0, 255 }
		}
	};
	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_INFORMATION, /* .flags */
		App->window->window, /* .window */
		"The Scene has been modified", /* .title */
		"Do you want to save the changes you made in the scene? \n Your changes will be lost if you don't save them.", /* .message */
		SDL_arraysize(buttons), /* .numbuttons */
		buttons, /* .buttons */
		&colorScheme /* .colorScheme */
	};
	int buttonid;

	if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0) 
	{
		LOG("Error displaying Quit Message Box");
	}
	
	switch (buttonid)
	{
	case 0: //Save
		save_scene_win = true;
		break;
	case 1: //Quit
		ret = true;
		break;
	case 2: //Cancel
		App->input->ResetQuit();
		break;
	}

	return ret;
}

void Editor::SaveSceneWindow()
{
	if (save_scene_win)
	{
		ImGui::SetNextWindowPos(ImVec2(App->window->GetScreenWidth() / 2 - 150, App->window->GetScreenHeight() / 2 - 50));
		ImGui::SetNextWindowSize(ImVec2(300, 100));
		if (ImGui::Begin("Save Scene", &save_scene_win))
		{
			ImGui::InputText("", scene_name_to_save._Myptr(), scene_name_to_save.capacity());
			if (ImGui::Button("Save ##save_scene_button"))
			{
				string scene = scene_name_to_save.data();
				scene = assets->CurrentDirectory() + scene;
				App->resource_manager->SaveScene(scene.data(), assets->CurrentLibraryDirectory());
				save_scene_win = false;
			}
			ImGui::End();
		}
	}
}
