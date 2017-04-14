#include "Application.h"
#include "TerrainWindow.h"
#include "ModulePhysics3D.h"
#include "ModuleResourceManager.h"
#include "ModuleEditor.h"
#include "Assets.h"

#include "ResourceFileTexture.h"
#include "Time.h"

#include "ModuleInput.h"
#include "SDL/include/SDL_scancode.h"

TerrainWindow::TerrainWindow()
{
}

TerrainWindow::~TerrainWindow()
{
}

void TerrainWindow::Draw()
{
	if (!active)
	{
		App->editor->lockSelection = false;
		App->physics->currentTerrainTool = App->physics->none_tool;
		return;
	}

	ImGui::Begin("Terrain_Tools", &active, flags);


	if (App->input->GetKey(SDL_SCANCODE_KP_PLUS) == KEY_DOWN)
	{
		App->physics->brushSize++;
		timer = 0.0f;
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_PLUS) == KEY_REPEAT)
	{
		if (timer < 0.5f)
		{
			timer += time->RealDeltaTime();
		}
		else
		{
			App->physics->brushSize++;
		}
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_MINUS) == KEY_DOWN)
	{
		App->physics->brushSize--;
		timer = 0.0f;
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_MINUS) == KEY_REPEAT)
	{
		if (timer < 0.5f)
		{
			timer += time->RealDeltaTime();
		}
		else
		{
			App->physics->brushSize--;
		}
	}


	if (App->input->GetKey(SDL_SCANCODE_KP_MULTIPLY) == KEY_DOWN)
	{
		App->physics->sculptStrength++;
		timer = 0.0f;
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_MULTIPLY) == KEY_REPEAT)
	{
		if (timer < 0.5f)
		{
			timer += time->RealDeltaTime();
		}
		else
		{
			App->physics->sculptStrength++;
		}
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_DIVIDE) == KEY_DOWN)
	{
		App->physics->sculptStrength--;
		timer = 0.0f;
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_DIVIDE) == KEY_REPEAT)
	{
		if (timer < 0.5f)
		{
			timer += time->RealDeltaTime();
		}
		else
		{
			App->physics->sculptStrength--;
		}
	}

	bool toolBool = App->physics->currentTerrainTool == App->physics->paint_tool;
	if (ImGui::Checkbox("Texture paint mode", &toolBool))
	{
		if (toolBool)
		{
			App->physics->currentTerrainTool = App->physics->paint_tool;
			App->editor->lockSelection = true;
		}
		else
		{
			App->physics->currentTerrainTool = App->physics->none_tool;
			App->editor->lockSelection = false;
		}
	}
	if (App->physics->currentTerrainTool == App->physics->paint_tool)
	{
		ImGui::Text("Brush");
		char button[64] = " ";
		for (int n = 0; n < App->physics->GetNTextures(); n++)
		{
			if (App->input->GetKey(SDL_SCANCODE_1 + n) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_KP_1 + n) == KEY_DOWN)
			{
				App->physics->paintTexture = n;
			}
			sprintf(button, "%i##paintTextureButton", n + 1);

			if (ImGui::Button(button))
			{
				App->physics->paintTexture = n;
			}
			ImGui::SameLine();
			if (App->physics->paintTexture != n)
			{
				ImGui::Text("%s", App->physics->GetTextureName(n).data());
			}
			else
			{
				ImGui::Text("--- %s ---", App->physics->GetTextureName(n).data());
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
		if (toolBool)
		{
			App->physics->currentTerrainTool = App->physics->sculpt_tool;
			App->editor->lockSelection = true;
		}
		else
		{
			App->physics->currentTerrainTool = App->physics->none_tool;
			App->editor->lockSelection = false;
		}
	}
	if (App->physics->currentTerrainTool == App->physics->sculpt_tool)
	{
		for (int n = 0; n < 3; n++)
		{
			if (App->input->GetKey(SDL_SCANCODE_1 + n) == KEY_DOWN || App->input->GetKey(SDL_SCANCODE_KP_1 + n) == KEY_DOWN)
			{
				App->physics->sculptTool = (SculptModeTools)n;
			}
		}
		ImGui::InputInt("Brush Size", &App->physics->brushSize);
		ImGui::DragFloat("Sculpt strength", &App->physics->sculptStrength, 0.1f, 0.1f, 30.0f);
		ImGui::RadioButton("Smooth", (int*)&App->physics->sculptTool, SculptModeTools::sculpt_smooth);	ImGui::SameLine();
		ImGui::RadioButton("Raise/Lower", (int*)&App->physics->sculptTool, SculptModeTools::sculpt_raise); ImGui::SameLine();
		ImGui::RadioButton("Flatten", (int*)&App->physics->sculptTool, SculptModeTools::sculpt_flatten);
		
	}

	ImGui::NewLine();
	ImGui::Separator();


	toolBool = App->physics->currentTerrainTool == App->physics->goPlacement_tool;
	if (ImGui::Checkbox("GameObject placement mode", &toolBool))
	{
		if (toolBool) 
		{
			App->physics->currentTerrainTool = App->physics->goPlacement_tool;
			App->editor->lockSelection = true;
		}
		else
		{
			App->physics->currentTerrainTool = App->physics->none_tool;
			App->editor->lockSelection = false;
		}
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

	ImGui::NewLine();
	ImGui::Separator();

	ImGui::Text("Shortcuts:\nUse keypad numbers to change textures/sculpting tools.\nUse keypad +/- to change the brush size.\nUse keypad multiply/divide to change the brush strength.\nWhen sculpting, using the raise/lower tool, hold shift to lower the terrain");

	App->physics->brushSize = CAP(App->physics->brushSize, 0, 1000);
	ImGui::End();
}

