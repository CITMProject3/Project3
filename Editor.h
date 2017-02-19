#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "Module.h"
#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"
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

class Editor : public Module
{
public:
	Editor(const char* name, bool start_enabled = true);
	~Editor();
	 
	bool Init(Data& config);

	bool Start();
	update_status Update();
	bool CleanUp();

	string GetAssetsCurrentDir()const;
	void RefreshAssets()const;
	//TODO: Ready for window resizing
	void OnResize(int screen_width, int screen_height);

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

	bool QuitWindow();
	void SaveSceneWindow();

public:
	Assets* assets = nullptr;
	MaterialCreatorWindow* material_creator_win = nullptr;
	RenderTexEditorWindow* rendertex_win = nullptr;
	Skybox skybox;

	GameObject* selected_GO = nullptr;

private:

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
	
	bool save_scene_win = false;
	string scene_name_to_save;

	bool disable_grid = false;

};

#endif