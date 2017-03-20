#include "Application.h"

#include "ModuleFileSystem.h"
#include "ModuleEditor.h"

#include "Assets.h"
#include "ShaderEditorWindow.h"

#include <vector>

using namespace std;

ShaderEditorWindow::ShaderEditorWindow()
{
	vertex_path.resize(512);
	fragment_path.resize(512);
	
	program = new char[64];
}

ShaderEditorWindow::~ShaderEditorWindow()
{
	delete[] program;
}

void ShaderEditorWindow::Draw()
{
	if (!active)
		return;
	
	ImGui::Begin("Shader editor", &active, flags);

	ImGui::Separator();
	ImGui::TextColored(ImVec4(1, 0, 0, 1), "This feature is still in construction. Do not use it.");
	ImGui::Separator();

	ImGui::Text("Editing: "); ImGui::SameLine(); 
	if (edit_vertex_enabled)
		ImGui::TextColored(ImVec4(0.8f, 0, 0, 1), "vertex program");
	else
		ImGui::TextColored(ImVec4(0.8f, 0, 0, 1), "fragment program");

	if (ImGui::Button("Edit vertex program"))
	{
		edit_vertex_enabled = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Edit fragment program"))
	{
		edit_vertex_enabled = false;
	}

	ImGui::Text("File: ");
	ImGui::SameLine();

	if (edit_vertex_enabled)
	{
		string vertex_name = vertex_path + "###vertex_title_edit";
		if (ImGui::BeginMenu(vertex_name.data()))
		{
			vector<string> vertex_list;
			App->editor->assets->GetAllFilesByType(FileType::VERTEX, vertex_list);

			for (vector<string>::iterator ver = vertex_list.begin(); ver != vertex_list.end(); ver++)
			{
				if (ImGui::MenuItem((*ver).data()))
				{
					vertex_path = (*ver).data();
					App->file_system->Load(vertex_path.data(), &program);		
				}
			}

			ImGui::EndMenu();
		}
	}
	else
	{
		string fragment_name = fragment_path + "###fragment_title_edit";
		if (ImGui::BeginMenu(fragment_name.data()))
		{
			vector<string> fragment_list;
			App->editor->assets->GetAllFilesByType(FileType::FRAGMENT, fragment_list);

			for (vector<string>::iterator fra = fragment_list.begin(); fra != fragment_list.end(); fra++)
			{
				if (ImGui::MenuItem((*fra).data()))
				{
					fragment_path = (*fra).data();
				}
			}

			ImGui::EndMenu();
		}
	}
	
	ImGui::InputTextMultiline("###code_shader", program, sizeof(program), ImVec2(-1.0f, ImGui::GetTextLineHeight() * 10), ImGuiInputTextFlags_AllowTabInput | 0);

	ImGui::InputInt("fdhalofha", &id);
	ImGui::Image((ImTextureID)id, ImVec2(600, 600));

	ImGui::End();
}

