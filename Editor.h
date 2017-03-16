#ifndef __EDITOR_H__
#define __EDITOR_H__

//PEP : this is a git bash test

#include "Module.h"
#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include "ImGuizmo\ImGuizmo.h"
#include "Window.h"
#include <list>
#include <vector>
#include "Skybox.h"

class FPSGraph;
class WindowOptions;
class HardwareInfo;
class Console;
class Assets;
class Hierarchy;
class Inspector;
class CameraWindow;
class ResourcesWindow;
class MaterialCreatorWindow;
class ShaderEditorWindow;
class LightingWindow;
class LayersWindow;
class RenderTexEditorWindow;
class TestWindow;
class CurveWindow;

using namespace std;

class Editor : public Module
{
public:
	Editor(const char* name, bool start_enabled = true);
	~Editor();
	 
	bool Init(Data& config);

	bool Start();

	update_status PreUpdate();
	update_status Update();
	bool CleanUp();

	void HandleInput();

	string GetAssetsCurrentDir()const;
	void RefreshAssets()const;

	void InitSizes();
	void OnResize(int screen_width, int screen_height);

	bool UsingKeyboard() const;
	bool UsingMouse() const;

	//GameObject selection
	void SelectSingle(GameObject* game_object);
	void AddSelect(GameObject* game_object);
	void Unselect(GameObject* game_object);
	void UnselectAll();

	bool IsSelected(GameObject* game_object) const;
	void RemoveSelected();

	void Copy(GameObject* game_object);
	void Paste(GameObject* game_object);
	void Duplicate(GameObject* game_object);

private:
	//Game Simulation Options
	void GameOptions()const;

	//Editor Windows
	update_status EditorWindows();
	
	//Menus
	void FileMenu();
	void HelpMenu();
	void WindowsMenu();
	void EditMenu();
	void DebugMenu();
	void GameObjectMenu();
	void PhysicsMenu();

	bool QuitWindow();
	void OnSaveCall();
	void OpenSaveSceneWindow();
	void SaveSceneWindow();

	void DisplayGizmo();

public:
	Assets* assets = nullptr;
	MaterialCreatorWindow* material_creator_win = nullptr;
	RenderTexEditorWindow* rendertex_win = nullptr;
	Skybox skybox;

	std::list<GameObject*> selected;
	GameObject* copy_go = nullptr;

	ImGuizmo::OPERATION gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
	bool gizmo_enabled = true;

private:

	bool using_keyboard;
	bool using_mouse;

	bool select_dragging = false;
	ImVec2 start_drag;

	vector<Window*> windows;

	//Windows
	Hierarchy* hierarchy = nullptr;
	Inspector* inspector = nullptr;
	FPSGraph* fps_graph_win = nullptr;
	WindowOptions* winoptions_win = nullptr;
	HardwareInfo* hardware_win = nullptr;
	CameraWindow* camera_win = nullptr;
	ResourcesWindow* resource_win = nullptr;
	ShaderEditorWindow* shader_editor_win = nullptr;
	LightingWindow* lighting_win = nullptr;
	LayersWindow* layers_win = nullptr;
	TestWindow* test_win = nullptr;
	CurveWindow* curve_win = nullptr;

	bool save_scene_win = false;
	string scene_name_to_save;

	bool disable_grid = false;

	float heightMapScaling = 1.0f;

	bool save_quit = false;
	bool quit = false;
};

#endif