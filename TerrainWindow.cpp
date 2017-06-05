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

	if (App->input->GetKey(SDL_SCANCODE_KP_0) == KEY_DOWN)
	{
		App->physics->hardBrush = ! App->physics->hardBrush;
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_MULTIPLY) == KEY_DOWN)
	{
		App->physics->brushStrength++;
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
			App->physics->brushStrength++;
		}
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_DIVIDE) == KEY_DOWN)
	{
		App->physics->brushStrength--;
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
			App->physics->brushStrength--;
		}
	}

	bool toolBool = App->physics->currentTerrainTool == App->physics->paint_tool;
	if (ImGui::Checkbox("Texture paint mode", &toolBool))
	{
		if (toolBool)
		{
			App->physics->currentTerrainTool = App->physics->paint_tool;
			App->physics->brushStrength = 100.0f;
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
		ImGui::Checkbox("Hard Brush", &App->physics->hardBrush);
		ImGui::DragFloat("Opacity", &App->physics->brushStrength, 0.1f, 0.1f, 100.0f);
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
			App->physics->brushStrength = 40.0f;
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
		ImGui::DragFloat("Sculpt strength", &App->physics->brushStrength, 0.1f, 0.1f, 30.0f);
		ImGui::RadioButton("Smooth", (int*)&App->physics->sculptTool, SculptModeTools::sculpt_smooth);	ImGui::SameLine();
		ImGui::RadioButton("Raise/Lower", (int*)&App->physics->sculptTool, SculptModeTools::sculpt_raise); ImGui::SameLine();
		ImGui::RadioButton("Flatten", (int*)&App->physics->sculptTool, SculptModeTools::sculpt_flatten);
		
		toolBool = App->physics->sculptTool == SculptModeTools::sculpt_smooth;
		if (toolBool)
		{
			ImGui::NewLine();
			ImGui::Text("Size of the sample the smooth tool will use when averaging vertices. Default is 6.");
			ImGui::InputInt("##SampleSize", &App->physics->smoothSampleSize);
			ImGui::NewLine();
			ImGui::Text("Filter by texture:");
			ImGui::RadioButton("None", (int*)&App->physics->smoothMask, -1);
			for (int n = 0; n < 4, n < App->physics->textures.size(); n++)
			{
				ImGui::RadioButton(App->physics->textures[n].second.data(), (int*)&App->physics->smoothMask, n);
			}
		}
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
	if (ImGui::CollapsingHeader("Texture Map"))
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
	}

	ImGui::NewLine();
	ImGui::Separator();

	ImGui::Text("Shortcuts:\nUse keypad numbers to change textures/sculpting tools.\nUse keypad +/- to change the brush size.\nUse keypad multiply/divide to change the brush strength.\nWhen sculpting, using the raise/lower tool, hold shift to lower the terrain.\nUse keypad 0 to toggle hard/soft edges when painting.");

	App->physics->brushSize = CAP(App->physics->brushSize, 0, 1000);
	ImGui::End();
}

