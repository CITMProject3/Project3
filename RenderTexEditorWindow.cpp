#include "Application.h"
#include "RenderTexEditorWindow.h"
#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "ModuleEditor.h"

RenderTexEditorWindow::RenderTexEditorWindow()
{}

RenderTexEditorWindow::~RenderTexEditorWindow()
{}

void RenderTexEditorWindow::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Render Texure Configuration", &active, flags);

	if (assets_path.size() == 0)
	{
		ImGui::Text("Click on a render texture and select edit option");
	}
	else
	{
		ImGui::Text("Render texture: %s", assets_path.data());
		ImGui::Separator();

		ImGui::Text("Width: ");
		ImGui::SameLine();
		ImGui::InputInt("###input_width_tex", &width);

		ImGui::Text("Height: ");
		ImGui::SameLine();
		ImGui::InputInt("###input_height_tex", &height);

		ImGui::Text("Use depth as texture: ");
		ImGui::SameLine();
		ImGui::Checkbox("###input_depth_tex", &use_depth_as_texture);

		if (ImGui::Button("Save ###ren_tex"))
		{
			App->resource_manager->SaveRenderTexture(assets_path, library_path, width, height, use_depth_as_texture);
		}
	}

	ImGui::End();
}

void RenderTexEditorWindow::LoadToEdit(const char * path)
{
	char* buffer;
	int size = App->file_system->Load(path, &buffer);

	if (size > 0)
	{
		Data data(buffer);

		width = data.GetInt("width");
		height = data.GetInt("height");
		use_depth_as_texture = data.GetBool("use_depth_as_texture");
		assets_path = data.GetString("assets_path");
		library_path = data.GetString("library_path");
	}
	else
	{
		LOG("[Error] Could not load file %s", path);
		App->editor->DisplayWarning(WarningType::W_ERROR, "Could not load file %s", path);
	}

	if (buffer)
		delete[] buffer;
}

