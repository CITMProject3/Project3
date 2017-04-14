#include "Application.h"
#include "TerrainWindow.h"
#include "ModulePhysics3D.h"
#include "ModuleResourceManager.h"
#include "ModuleEditor.h"
#include "Assets.h"

TerrainWindow::TerrainWindow()
{
}

TerrainWindow::~TerrainWindow()
{
}

void TerrainWindow::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Terrain_Tools", &active, flags);

	bool toolBool = App->physics->currentTerrainTool == App->physics->paint_tool;
	if (ImGui::Checkbox("Texture paint mode", &toolBool))
	{
		if (toolBool) { App->physics->currentTerrainTool = App->physics->paint_tool; }
		else { App->physics->currentTerrainTool = App->physics->none_tool; }
	}
	if (App->physics->currentTerrainTool == App->physics->paint_tool)
	{
		ImGui::Text("Brush");
		char button[64] = " ";
		for (int n = 0; n < App->physics->GetNTextures(); n++)
		{
			sprintf(button, "%i##paintTextureButton", n + 1);
			ImGui::SameLine();
			if (ImGui::Button(button))
			{
				App->physics->paintTexture = n;
			}
		}
		ImGui::InputInt("Brush Size", &App->physics->brushSize);
		if (App->physics->paintTexture < App->physics->GetNTextures())
		{
			float2 size = App->physics->GetHeightmapSize();
			float maxSize = max(size.x, size.y);
			if (maxSize > 200)
			{
				float scale = 200.0f / maxSize;
				size.x *= scale;
				size.y *= scale;
			}
			ImGui::Image((void*)App->physics->GetTexture(App->physics->paintTexture), ImVec2(size.x, size.y));
		}
	}

	ImGui::NewLine();
	ImGui::Separator();

	toolBool = App->physics->currentTerrainTool == App->physics->sculpt_tool;
	if (ImGui::Checkbox("Sculpt mode", &toolBool))
	{
		if (toolBool) { App->physics->currentTerrainTool = App->physics->sculpt_tool; }
		else { App->physics->currentTerrainTool = App->physics->none_tool; }
	}
	if (App->physics->currentTerrainTool == App->physics->sculpt_tool)
	{
		ImGui::InputInt("Brush Size", &App->physics->brushSize);
		ImGui::DragFloat("Sculpt strength", &App->physics->sculptStrength, 0.1f, 0.1f, 30.0f);
		ImGui::RadioButton("Flatten", (int*)&App->physics->sculptTool, SculptModeTools::sculpt_flatten); ImGui::SameLine();
		ImGui::RadioButton("Raise/Lower", (int*)&App->physics->sculptTool, SculptModeTools::sculpt_raise); ImGui::SameLine();
		ImGui::RadioButton("Smooth", (int*)&App->physics->sculptTool, SculptModeTools::sculpt_smooth);
	}

	ImGui::NewLine();
	ImGui::Separator();


	toolBool = App->physics->currentTerrainTool == App->physics->goPlacement_tool;
	if (ImGui::Checkbox("GameObject placement mode", &toolBool))
	{
		if (toolBool) { App->physics->currentTerrainTool = App->physics->goPlacement_tool; }
		else { App->physics->currentTerrainTool = App->physics->none_tool; }
	}
	if (App->physics->currentTerrainTool == App->physics->goPlacement_tool)
	{
		if (ImGui::BeginMenu("Select a Prefab"))
		{
			vector<string> mesh_list;
			App->editor->assets->GetAllFilesByType(FileType::PREFAB, mesh_list);

			for (size_t i = 0; i < mesh_list.size(); ++i)
			{
				if (ImGui::MenuItem(mesh_list[i].data()))
				{
					App->physics->GO_toPaint_libPath = App->resource_manager->FindFile(mesh_list[i]);
					terrainPlacingObject = mesh_list[i];
				}
			}
			ImGui::EndMenu();
		}
		ImGui::NewLine();
		if (terrainPlacingObject.length() > 4)
		{
			ImGui::Text("Currently selected prefab:");
			ImGui::Text("%s", terrainPlacingObject.data());
		}
	}
	ImGui::End();
}

