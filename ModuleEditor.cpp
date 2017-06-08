#include "ModuleEditor.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleFileSystem.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModulePhysics3D.h"
#include "ModuleCamera3D.h"
#include "ModuleGOManager.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ComponentLight.h"

#include "Time.h"

#include "FPSGraph.h"
#include "WindowOptions.h"
#include "HardwareInfo.h"
#include "Console.h"
#include "Assets.h"
#include "Hierarchy.h"
#include "Inspector.h"

#include "CameraWindow.h"
#include "ResourcesWindow.h"
#include "MaterialCreatorWindow.h"
#include "ShaderEditorWindow.h"
#include "LightingWindow.h"
#include "LayersWindow.h"
#include "CurveWindow.h"
#include "TerrainWindow.h"
#include "RenderTexEditorWindow.h"
#include "TestWindow.h"
#include "RaycastHit.h"

#include "SDL/include/SDL_scancode.h"
#include "SDL/include/SDL_messagebox.h"
#include "SDL/include/SDL_mouse.h"

ModuleEditor::ModuleEditor(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	windows.push_back(console = new Console()); //Create console in the constructor to get ALL init logs from other modules.
}

ModuleEditor::~ModuleEditor()
{}

bool ModuleEditor::Init(Data & config)
{
	//TODO: move into parameter configuration setup (like window color)

	//Window rounding
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

	//Child window background
	ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(0.3, 0.3, 0.3, 0.3));

	return true;
}

bool ModuleEditor::Start()
{
	bool ret = true;

	LOG("Start Editor");

	if (App->StartInGame() == false)
	{
		//Create Windows
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
		windows.push_back(curve_win = new CurveWindow());
		windows.push_back(warning_window = new WarningWindow());
		windows.push_back(terrain_window = new TerrainWindow());
		InitSizes();
	}	
	else
	{
		//Start in game
		disable_grid = true;
	}

	//Testing
	skybox.Init("Resources/Skybox/s_left.dds", "Resources/Skybox/s_right.dds", "Resources/Skybox/s_up.dds", "Resources/Skybox/s_down.dds", "Resources/Skybox/s_front.dds", "Resources/Skybox/s_back.dds");

	OnResize(App->window->GetScreenWidth(), App->window->GetScreenHeight());

	heightmapMaxHeight = App->physics->GetTerrainHeightScale();

	return ret;
}

bool ModuleEditor::CleanUp()
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
	delete test_win;
	delete curve_win;
	delete warning_window;

	windows.clear();

	return true;
}

string ModuleEditor::GetAssetsCurrentDir() const
{
	return assets->CurrentDirectory();
}

void ModuleEditor::RefreshAssets() const
{
	if (assets)
		assets->Refresh();
}

void ModuleEditor::InitSizes()
{
	hierarchy->SetRelativeDimensions(ImVec2(0, 0.0), ImVec2(0.15, 0.8));
	inspector->SetRelativeDimensions(ImVec2(0.80, 0.0), ImVec2(0.20, 0.8));
	assets->SetRelativeDimensions(ImVec2(0, 0.8), ImVec2(1.0, 0.2));
	warning_window->SetRelativeDimensions(ImVec2(0.4, 0.4), ImVec2(0.2, 0.2));
}

void ModuleEditor::OnResize(int screen_width, int screen_height)
{
	vector<Window*>::iterator win = windows.begin();
	while (win != windows.end())
	{
		(*win)->OnResize(screen_width, screen_height);
		++win;
	}
}

bool ModuleEditor::UsingKeyboard() const
{
	return using_keyboard;
}

bool ModuleEditor::UsingMouse() const
{
	return using_mouse;
}

void ModuleEditor::SelectSingle(GameObject* game_object)
{
	if (lockSelection == false)
	{
		UnselectAll();
		if (game_object != nullptr)
			selected.push_back(game_object);
	}
}

void ModuleEditor::AddSelect(GameObject* game_object)
{
	if (lockSelection == false)
	{
		//Just for safety
		if (game_object != nullptr && IsSelected(game_object) == false)
			selected.push_back(game_object);
	}
}

void ModuleEditor::Unselect(GameObject* game_object)
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

void ModuleEditor::UnselectAll()
{
	assign_wheel = -1;
	wheel_assign = nullptr;
	selected.clear();
}

bool ModuleEditor::IsSelected(GameObject* game_object) const
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

void ModuleEditor::RemoveSelected()
{
	std::list<GameObject*>::const_iterator it = selected.begin();
	while (it != selected.end())
	{
		App->go_manager->RemoveGameObject(*it);
		it++;
	}
	selected.clear();
}

void ModuleEditor::DisplayWarning(WarningType type, const char *format, ...)
{
	if (App->StartInGame() == false)
	{
		static char tmp_string[4096];
		static char tmp_string2[4096];
		static va_list ap;

		// Construct the string from variable arguments
		va_start(ap, format);
		vsprintf_s(tmp_string, 4096, format, ap);
		va_end(ap);

		if (warning_window) warning_window->AddMessage(tmp_string, type);
	}
}

update_status ModuleEditor::PreUpdate()
{
	//ImGui::ShowTestWindow();

	using_keyboard = ImGui::GetIO().WantCaptureKeyboard;
	using_mouse = ImGui::GetIO().WantCaptureMouse;

	if (lockSelection && selected.empty() == false)
	{
		UnselectAll();
	}

	return UPDATE_CONTINUE;
}

update_status ModuleEditor::Update()
{
	update_status ret = UPDATE_CONTINUE;

	if (App->StartInGame() == false)
	{
		DisplayGizmo();
		GameOptions(); //Play/Stop/Next Frame buttons
		ret = EditorWindows(); //Update the windows of the editor
	}
	
	//Draw Grid
	if (!disable_grid)
	{
		Plane_P grid(0, 1, 0, 0);
		grid.axis = true;
		grid.Render();
	}
	
	for (std::list<GameObject*>::iterator it = selected.begin(); it != selected.end(); it++)
	{
		if ((*it)->bounding_box != nullptr)
			g_Debug->AddAABB(*(*it)->bounding_box, g_Debug->green);
	}

	HandleInput();

	//Handle Quit event
	if (App->input->Quit())
	{
		if (App->StartInGame() == false)
		{
			save_quit = true;
			OpenSaveSceneWindow();
		}
		else
		{
			quit = true;
		}
	}

	if (quit)
		ret = UPDATE_STOP;

	return ret;	
}

void ModuleEditor::HandleInput()
{
	//Shortcut to save. TODO: Do a better implementation of the shortcuts
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
	{
		OnSaveCall();
	}

	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_D) == KEY_DOWN)
	{
		if (selected.empty() == false)
		{
			App->go_manager->DuplicateGameObject(selected.back());
		}
	}
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
			GameObject* game_object = App->go_manager->Raycast(ray, std::vector<int>(), true).object;

			if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RSHIFT) == KEY_REPEAT ||
				App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT || App->input->GetKey(SDL_SCANCODE_RCTRL) == KEY_REPEAT)
			{
				IsSelected(game_object) ? Unselect(game_object) : AddSelect(game_object);
			}
			else
			{
				SelectSingle(App->go_manager->Raycast(ray, std::vector<int>(), true).object);
			}
		}
	}
}

void ModuleEditor::GameOptions() const
{
	ImGui::SetNextWindowPos(ImVec2(App->window->GetScreenWidth()/2, 30));
	bool open = true;
	ImGui::Begin("##GameOptions", &open, ImVec2(0, 0), 0.6f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3, 0.3, 0.3, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4, 0.4, 0.4, 1));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1, 0.1, 0.1, 1));

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
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::End();
}

update_status ModuleEditor::EditorWindows()
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
		if (ImGui::BeginMenu("GameObject"))
		{
			GameObjectMenu();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Physics"))
		{
			PhysicsMenu();
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
		ImGui::EndMainMenuBar();
	}

	//Windows ----------------------------------------------------------------------------------------------------
	vector<Window*>::iterator win = windows.begin();
	while (win != windows.end())
	{
		(*win)->Draw();
		++win;
	}

	SaveSceneWindow();
	
	return ret;
}


void ModuleEditor::FileMenu()
{
	if (ImGui::MenuItem("New Scene"))
	{
		App->go_manager->LoadEmptyScene();
	}
	if (ImGui::BeginMenu("Open Scene"))
	{
		std::vector<std::string> scenes;
		assets->GetAllFilesByType(FileType::SCENE, scenes);
		for (uint i = 0; i < scenes.size(); i++)
		{
			std::string name = App->file_system->GetNameFromPath(scenes[i].c_str());
			if (ImGui::MenuItem(name.c_str()))
			{
				App->resource_manager->LoadScene(scenes[i].c_str());
			}
		}
		ImGui::EndMenu();
	}
	if (ImGui::MenuItem("Save Scene"))
	{
		OnSaveCall();
	}
	if (ImGui::MenuItem("Save Scene as..."))
	{
		OpenSaveSceneWindow();
	}
}

//Menus -----------------------------------------------------------------------------------------------
void ModuleEditor::HelpMenu()
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

void ModuleEditor::WindowsMenu()
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

	if (ImGui::MenuItem("Warnings"))
	{
		warning_window->SetActive(true);
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

	if (ImGui::MenuItem("Curve Window"))
	{
		curve_win->SetActive(true);
	}
}

void ModuleEditor::EditMenu()
{
	ImGui::Checkbox("Debug render", &App->physics->renderColliders);

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

void ModuleEditor::GameObjectMenu()
{
	if (ImGui::MenuItem("Create Empty"))
	{

	}
	if (ImGui::MenuItem("Create Empty Child"))
	{

	}
	if (ImGui::BeginMenu("3D Object"))
	{
		if (ImGui::MenuItem("Cube"))
			App->go_manager->CreatePrimitive(PrimitiveType::P_CUBE);

		if (ImGui::MenuItem("Sphere"))
			App->go_manager->CreatePrimitive(PrimitiveType::P_SPHERE);

		if (ImGui::MenuItem("Plane"))
			App->go_manager->CreatePrimitive(PrimitiveType::P_PLANE);

		if (ImGui::MenuItem("Cylinder"))
			App->go_manager->CreatePrimitive(PrimitiveType::P_CYLINDER);

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Light"))
	{
		if (ImGui::MenuItem("Directional Light"))
		{
			App->go_manager->CreateLight(nullptr, LightType::DIRECTIONAL_LIGHT);
		}
		ImGui::EndMenu();
	}
}

void ModuleEditor::PhysicsMenu()
{
	if (ImGui::MenuItem("Open terrain tools"))
	{
		terrain_window->SetActive(true);
	}

	ImGui::NewLine();
	ImGui::Separator();

	if (ImGui::BeginMenu("Heightmap"))
	{
		if (ImGui::BeginMenu("Select a heightmap:"))
		{
			vector<string> textures_list;
			App->editor->assets->GetAllFilesByType(FileType::IMAGE, textures_list);
			App->editor->assets->GetAllFilesByType(FileType::RENDER_TEXTURE, textures_list);

			for (size_t i = 0; i < textures_list.size(); ++i)
			{
				if (ImGui::MenuItem(textures_list[i].data()))
				{
					string lib_file = App->resource_manager->FindFile(textures_list[i]);
					App->physics->GenerateHeightmap(lib_file);
					App->physics->SetTerrainMaxHeight(100.0f);
					App->editor->heightmapMaxHeight = 100.0f;
				}
			}
			ImGui::EndMenu();
		}
		int tex = App->physics->GetHeightmap();
		if (tex != 0)
		{
			float2 size = App->physics->GetHeightmapSize();
			float maxSize = max(size.x, size.y);
			if (maxSize > 500)
			{
				float scale = 500.0f / maxSize;
				size.x *= scale;
				size.y *= scale;
			}
			ImGui::Image((void*)tex, ImVec2(size.x, size.y));
		}
		if (tex != 0)
		{
			if (ImGui::Button("Regenerate Normals"))
			{
				App->physics->GenerateNormals();
			}
			ImGui::NewLine();
			if (ImGui::Button("Delete heightmap"))
			{
				App->physics->DeleteHeightmap();
			}
		}
		ImGui::EndMenu();
	}

	ImGui::NewLine();
	ImGui::Separator();
	if (ImGui::BeginMenu("Diffuse Textures"))
	{
		if (App->physics->GetHeightmap() != 0)
		{
			if (ImGui::BeginMenu("Load a new texture:"))
			{
				vector<string> textures_list;
				App->editor->assets->GetAllFilesByType(FileType::IMAGE, textures_list);
				App->editor->assets->GetAllFilesByType(FileType::RENDER_TEXTURE, textures_list);

				for (size_t i = 0; i < textures_list.size(); ++i)
				{
					if (ImGui::MenuItem(textures_list[i].data()))
					{
						string lib_file = App->resource_manager->FindFile(textures_list[i]);

						char* tmp_it = textures_list[i]._Myptr();
						tmp_it += textures_list[i].length();
						for (; *tmp_it != '\\' && *tmp_it != '/' && tmp_it != textures_list[i]._Myptr(); tmp_it--) {}
						tmp_it++;

						App->physics->LoadTexture(lib_file, -1, tmp_it);
					}
				}
				ImGui::EndMenu();
			}

			if (App->physics->GetNTextures() > 0)
			{
				for (uint n = 0; n < App->physics->GetNTextures(); n++)
				{
					ImGui::NewLine();
					ImGui::Text("%s", App->physics->GetTextureName(n).data());
					float2 size = App->physics->GetHeightmapSize();
					float maxSize = max(size.x, size.y);
					if (maxSize > 200)
					{
						float scale = 200.0f / maxSize;
						size.x *= scale;
						size.y *= scale;
					}
					ImGui::Image((void*)App->physics->GetTexture(n), ImVec2(size.x, size.y));
					char menuName[64] = " ";
					sprintf(menuName, "Replace texture:##texn%u", n);
					if (ImGui::BeginMenu(menuName))
					{
						vector<string> textures_list;
						App->editor->assets->GetAllFilesByType(FileType::IMAGE, textures_list);
						App->editor->assets->GetAllFilesByType(FileType::RENDER_TEXTURE, textures_list);

						for (size_t i = 0; i < textures_list.size(); ++i)
						{
							if (ImGui::MenuItem(textures_list[i].data()))
							{
								string lib_file = App->resource_manager->FindFile(textures_list[i]);

								char* tmp_it = textures_list[i]._Myptr();
								tmp_it += textures_list[i].length();
								for (; *tmp_it != '\\' && *tmp_it != '/' && tmp_it != textures_list[i]._Myptr(); tmp_it--) {}
								tmp_it++;

								App->physics->LoadTexture(lib_file, n, tmp_it);
							}
						}
						ImGui::EndMenu();
					}
					ImGui::NewLine();
					char buttonName[64] = "";
					sprintf(buttonName, "Delete texture##delText%u", n);
					if (ImGui::Button(buttonName))
					{
						App->physics->DeleteTexture(n);
					}
					ImGui::Separator();
					ImGui::NewLine();
				}
			}
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Debug Images: "))
	{
		ImGui::Text("Texture map:");
		if (App->physics->textureMap != nullptr && App->physics->textureMapBufferID != 0)
		{
			float2 size = App->physics->GetHeightmapSize();
			float maxSize = max(size.x, size.y);
			if (maxSize > 400)
			{
				float scale = 400.0f / maxSize;
				size.x *= scale;
				size.y *= scale;
			}
			ImGui::Image((void*)App->physics->textureMapBufferID, ImVec2(size.x, size.y));
		}
		else
		{
			ImGui::Text("\nNot Loaded\n");
		}
		ImGui::EndMenu();
	}
	ImGui::NewLine();
	ImGui::Separator();
	bool terrainExists = App->physics->TerrainIsGenerated();
	ImGui::NewLine();
	ImGui::Checkbox("Terrain is generated", &terrainExists);
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::Text("Terrain Max Height:");
	ImGui::DragFloat("##TerrainHeightScaling", &heightmapMaxHeight, 1.0f, 0.1f, 10000.0f);
	ImGui::SameLine();
	if (ImGui::Button("Set height"))
	{
		App->physics->SetTerrainMaxHeight(heightmapMaxHeight);
	}
	float tmp = App->physics->GetTextureScaling();
	ImGui::Text("Terrain Texture Scaling:");
	if (ImGui::DragFloat("##TerrainTextureScaling", &tmp, 0.001f, 0.001f, 1.0f))
	{
		App->physics->SetTextureScaling(tmp);
	}
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::Checkbox("Render chunks", &App->physics->renderChunks);
	ImGui::Checkbox("Render terrain", &App->physics->renderFilledTerrain);
	ImGui::Checkbox("Wireframed terrain", &App->physics->renderWiredTerrain);

}

void ModuleEditor::DebugMenu()
{
	if (ImGui::MenuItem("Show/Hide Octree"))
	{
		App->go_manager->draw_octree = !App->go_manager->draw_octree;
	}
	if (ImGui::MenuItem("Render AABBs"))
	{
		App->renderer3D->renderAABBs = !App->renderer3D->renderAABBs;
	}
	if (App->renderer3D->renderAABBs) { ImGui::SameLine(); ImGui::Text("X"); }
}

bool ModuleEditor::QuitWindow()
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

void ModuleEditor::OnSaveCall()
{
	std::string scene = App->go_manager->GetCurrentAssetsScenePath();
	if (scene == "")
	{
		OpenSaveSceneWindow();
	}
	else
	{
		AssetFile* asset = assets->FindAssetFile(scene);
		if (asset != nullptr)
		{
			const char* lib_path = asset->directory->library_path.c_str();
			App->resource_manager->SaveScene(scene.c_str(), lib_path);
		}
		else
		{
			OpenSaveSceneWindow();
		}
	}
}

void ModuleEditor::OpenSaveSceneWindow()
{
	std::string scene_name_path = App->go_manager->GetCurrentAssetsScenePath();
	if (scene_name_path != "")
	{
		std::string scene_name = App->file_system->GetNameFromPath(scene_name_path.c_str());
		uint period = scene_name.find_last_of(".");
		if (period == std::string::npos)
		{
			period = scene_name.length();
		}
		scene_name_to_save = scene_name.substr(0, period);
	}
	else
	{
		scene_name_to_save = "Untitled";
	}
	save_scene_win = true;
}

void ModuleEditor::SaveSceneWindow()
{
	if (save_scene_win)
	{
		ImGui::SetNextWindowPos(ImVec2(App->window->GetScreenWidth() / 2 - 150, App->window->GetScreenHeight() / 2 - 50));
		ImGui::SetNextWindowSize(ImVec2(300, 100));
		if (ImGui::Begin("Save Scene", &save_scene_win))
		{
			if (scene_name_to_save == "")
				scene_name_to_save = "Untiled";
			ImGui::InputText("", scene_name_to_save._Myptr(), scene_name_to_save.capacity());
			if (ImGui::Button("Save ##save_scene_button"))
			{
				string scene = scene_name_to_save.data();
				scene = assets->CurrentDirectory() + scene;
				App->resource_manager->SaveScene(scene.data(), assets->CurrentLibraryDirectory());
				save_scene_win = false;
				if (save_quit == true)
					quit = true;
			}
			ImGui::SameLine();

			if (ImGui::Button("Don't Save ##dont_save_scene_button"))
			{
				save_scene_win = false;
				if (save_quit == true)
					quit = true;
			}
			ImGui::SameLine();

			if (ImGui::Button("Cancel ##cancel_scene_button"))
			{
				if (save_quit == true)
					save_quit = false;
				save_scene_win = false;
				App->input->ResetQuit();
			}
			
		}
		ImGui::End();
		if(!save_scene_win)
			App->input->ResetQuit();
	}
}


void ModuleEditor::DisplayGizmo()
{
	//Selection keys
	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
		gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN)
		gizmo_operation = ImGuizmo::OPERATION::ROTATE;
	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
		gizmo_operation = ImGuizmo::OPERATION::SCALE;

	ImGuizmo::BeginFrame();

	if (selected.size() > 0)
	{
		GameObject* go = selected.back();

		ImGuizmo::Enable(gizmo_enabled);

		ComponentCamera* camera = App->camera->GetEditorCamera();

		float4x4 matrix = go->transform->GetLocalTransformMatrix().Transposed();

		ComponentTransform* transform = go->transform;

		ComponentTransform* parent_transform = nullptr;

		if (go->GetParent())
		{
			parent_transform = (ComponentTransform*)go->GetParent()->GetComponent(C_TRANSFORM);
			assert(parent_transform);

			matrix = parent_transform->GetGlobalMatrix() * transform->GetLocalTransformMatrix();
			matrix = matrix.Transposed();
		}



		if (gizmo_operation == ImGuizmo::OPERATION::SCALE)
			ImGuizmo::Manipulate(camera->GetViewMatrix().ptr(), camera->GetProjectionMatrix().ptr(), (ImGuizmo::OPERATION)gizmo_operation, ImGuizmo::LOCAL, matrix.ptr());

		else
			ImGuizmo::Manipulate(camera->GetViewMatrix().ptr(), camera->GetProjectionMatrix().ptr(), (ImGuizmo::OPERATION)gizmo_operation, (ImGuizmo::MODE)gizmo_mode, matrix.ptr());


		if (ImGuizmo::IsUsing())
		{
			matrix.Transpose();

			if (go->GetParent())
			{
				matrix = parent_transform->GetGlobalMatrix().Inverted() * matrix;
			}

			float3 position, scale;
			Quat rotation;
			matrix.Decompose(position, rotation, scale);

			go->transform->SetPosition(position);
			go->transform->SetRotation(rotation);
			go->transform->SetScale(scale);
		}
	}
}