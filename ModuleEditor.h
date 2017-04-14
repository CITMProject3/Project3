#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Module.h"

#include "Skybox.h"

#include "MathGeoLib\include\MathGeoLib.h"
#include "ImGuizmo\ImGuizmo.h"
#include "imgui/imgui.h"

#include "WarningWindow.h" // Needed here because of WarningType

#include <list>
#include <vector>

class GameObject;

class FPSGraph;
class WindowOptions;
class HardwareInfo;
class Console;
class Assets;
class Hierarchy;
class Inspector;
class Window;
class CameraWindow;
class ResourcesWindow;
class MaterialCreatorWindow;
class ShaderEditorWindow;
class LightingWindow;
class LayersWindow;
class RenderTexEditorWindow;
class TestWindow;
class CurveWindow;
class TerrainWindow;

class ComponentCar;

using namespace std;

class ModuleEditor : public Module
{
public:
	ModuleEditor(const char* name, bool start_enabled = true);
	~ModuleEditor();

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

	void DisplayWarning(WarningType type, const char *format, ...);

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
	ImGuizmo::MODE gizmo_mode = ImGuizmo::MODE::WORLD;
	bool gizmo_enabled = true;

	int assign_wheel = -1;
	GameObject* wheel_assign = nullptr;

	bool assign_item = false;
	ComponentCar* to_assign_item = nullptr;

	bool disable_grid = false;
private:

	bool using_keyboard;
	bool using_mouse;

	bool select_dragging = false;
	ImVec2 start_drag;

	vector<Window*> windows;

	// Permanent windows
	Hierarchy* hierarchy = nullptr;
	Inspector* inspector = nullptr;

	// Other windows
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
	WarningWindow* warning_window = nullptr;
	TerrainWindow* terrain_window = nullptr;

	bool save_scene_win = false;
	string scene_name_to_save;
	public:
	float heightmapMaxHeight = 1.0f;

	bool lockSelection = false;
	private:
	bool save_quit = false;
	bool quit = false;
};

#endif