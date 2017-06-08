#include "Application.h"
#include "ComponentUiText.h"
#include "ModuleResourceManager.h"
#include "ModuleGOManager.h"
#include "GameObject.h"
#include "ComponentRectTransform.h"
#include "ComponentMaterial.h"
#include "ResourceFileTexture.h"
#include "ResourceFileMesh.h"
#include "ComponentMesh.h"
#include "imgui\imgui.h"

ComponentUiText::ComponentUiText(ComponentType type, GameObject * game_object) : Component(type, game_object)
{
	text = "20:20:20"; 
	text.resize(20);
	array_values = "0123456789:k";
	array_values.resize(20);
	current_text_changing.resize(20);
	UImaterial = new ComponentMaterial(C_MATERIAL, nullptr);
	img_width = 100;
	img_height = 100;
	char_w = new int[2];
	char_h = new int[2];
	GeneratePlane();
}

ComponentUiText::~ComponentUiText()
{
	delete UImaterial;
}

void ComponentUiText::GeneratePlane()
{
	std::string prim_path = "Resources/Primitives/2147000003.msh";
	ResourceFileMesh* tmp_rc = (ResourceFileMesh*)App->resource_manager->LoadResource(prim_path, ResourceFileType::RES_MESH);
	ResourceFileMesh* plane = new ResourceFileMesh(*tmp_rc);
	plane->mesh = new Mesh();
	plane->mesh->num_vertices = 4;
	plane->mesh->vertices = new float[plane->mesh->num_vertices * 3];
	plane->mesh->num_indices = 6;
	plane->mesh->indices = new uint[plane->mesh->num_indices];
	plane->mesh->num_uvs = 4;
	plane->mesh->uvs = new float[plane->mesh->num_uvs * 2];
	plane->mesh->vertices[0] = 0.0f;
	plane->mesh->vertices[1] = 0.0f;
	plane->mesh->vertices[2] = 0.0f;
	plane->mesh->vertices[3] = 1.0f;
	plane->mesh->vertices[4] = 0.0f;
	plane->mesh->vertices[5] = 0.0f;
	plane->mesh->vertices[6] = 0.0f;
	plane->mesh->vertices[7] = 1.0f;
	plane->mesh->vertices[8] = 0.0f;
	plane->mesh->vertices[9] = 1.0f;
	plane->mesh->vertices[10] = 1.0f;
	plane->mesh->vertices[11] = 0.0f;


	plane->mesh->indices[0] = 0;
	plane->mesh->indices[1] = 2;
	plane->mesh->indices[2] = 1;
	plane->mesh->indices[3] = 1;
	plane->mesh->indices[4] = 2;
	plane->mesh->indices[5] = 3;



	plane->mesh->uvs[0] = 0.0f;
	plane->mesh->uvs[1] = 1.0f;
	plane->mesh->uvs[2] = 1.0f;
	plane->mesh->uvs[3] = 1.0f;
	plane->mesh->uvs[4] = 0.0f;
	plane->mesh->uvs[5] = 0.0f;
	plane->mesh->uvs[6] = 1.0f;
	plane->mesh->uvs[7] = 0.0f;

	plane->ReLoadInMemory();
	tplane = plane;
	planes.push_back(plane);
	meshes.push_back(plane->mesh);
}

void ComponentUiText::Update()
{
		
}

void ComponentUiText::CleanUp()
{
}

void ComponentUiText::OnInspector(bool debug)
{
	string str = (string("UI Text##") + to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##uitext");
		}

		if (ImGui::BeginPopup("delete##uitext"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

		if (UImaterial->DefaultMaterialInspector())
		{
			GenerateFont(); 
			OnChangeTexture();
		}
		if (ImGui::Button("Change Text"))
		{
			change_text = true;
			current_text_changing = text;
		}
		ImGui::SameLine();
		ImGui::Text(text.c_str());

		if (ImGui::Button("Change array values"))
		{
			change_array_values = true;
			current_text_changing = array_values;
		}
		ImGui::SameLine();
		ImGui::Text(array_values.c_str());
		
		if (change_text == true)
		{
			SetText(text);
		}
		else if (change_array_values == true)
		{
			SetText(array_values);
		}

		ImGui::InputInt("Letter offset", &char_offset);
	}
}

void ComponentUiText::Save(Data & file) const
{
	Data data;

	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendString("text", text.c_str());
	data.AppendString("array_values", array_values.c_str());
	data.AppendInt("char_offset", char_offset);
	data.AppendArray("Material");
	UImaterial->Save(data);
	file.AppendArrayValue(data);
}

void ComponentUiText::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
	text = conf.GetString("text");
	array_values = conf.GetString("array_values");
	char_offset = conf.GetInt("char_offset");
	Data mat_file;
	mat_file = conf.GetArray("Material", 0);
	UImaterial->Load(mat_file);
//	text.resize(text.length() + 10);
	array_values.resize(array_values.length() + 1);
	OnChangeTexture();
	//LOG("%d",text.size());
}

int ComponentUiText::GetLenght() const
{
	return len;
}

string ComponentUiText::GetText() const
{
	return text;
}

string ComponentUiText::GetArrayValues() const
{
	return array_values;
}

int ComponentUiText::GetCharRows() const
{
	return 0;
}

int ComponentUiText::GetCharOffset() const
{
	return char_offset;
}

float ComponentUiText::GetCharwidth(int i) const
{
	if (char_w != nullptr)
		return char_w[i];
	else
		return 0.0f;
}

float ComponentUiText::GetCharHeight(int i) const
{
	if (char_h != nullptr)
		return char_h[i];
	else
		return 0.0f;
}

float ComponentUiText::GetImgWidth() const
{
	return img_width;
}

float ComponentUiText::GetImgHeight() const
{
	return img_height;
}

void ComponentUiText::SetText(string &text)
{
	//ImGui::SetNextWindowPos(ImVec2(App->window->GetScreenWidth() / 2 - 150, App->window->GetScreenHeight() / 2 - 50));
	ImGui::SetNextWindowSize(ImVec2(300, 100));

	string str = string("Change Text: ") + text;
	if (ImGui::Begin(str.c_str(),&change_text))
	{
		ImGui::InputText("", current_text_changing._Myptr(), current_text_changing.capacity());
		if (ImGui::Button("Save ##change_text"))
		{
			text = current_text_changing.c_str();
			change_text = false;
			change_array_values = false;
			GenerateFont();
			OnChangeTexture();
		}
		ImGui::End();
	}

}

void ComponentUiText::SetDisplayText(string text)
{
	this->text = text;
}

void ComponentUiText::SetCharOffset(int off)
{
	char_offset = off;
}

void ComponentUiText::GenerateFont()
{
	len = array_values.length();
}

bool ComponentUiText::OnChangeTexture()
{
	if (UImaterial->list_textures_paths.size() > 0)
	{
		for (vector<ResourceFileMesh*>::const_iterator it = planes.begin(); it != planes.end(); it++)
		{
			(*it)->Unload();
		}
		planes.clear();
		meshes.clear();
		int i = 0;
		char_w = new int[UImaterial->list_textures_paths.size()];
		char_h = new int[UImaterial->list_textures_paths.size()];
		for (vector<string>::const_iterator it = UImaterial->list_textures_paths.begin(); it != UImaterial->list_textures_paths.end(); it++)
		{
			string tex_path = (*it);
			ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(tex_path, ResourceFileType::RES_TEXTURE);

			if (rc_tmp != nullptr)
			{
				img_width = rc_tmp->GetWidth();
				img_height = rc_tmp->GetHeight();

				char_w[i] = img_width;
				char_h[i] = img_height;

				ResourceFileMesh* plane = new ResourceFileMesh(*tplane);
				plane->mesh = new Mesh();

				plane->mesh->num_vertices = 4;
				plane->mesh->vertices = new float[plane->mesh->num_vertices * 3];
				plane->mesh->num_indices = 6;
				plane->mesh->indices = new uint[plane->mesh->num_indices];
				plane->mesh->num_uvs = 4;
				plane->mesh->uvs = new float[plane->mesh->num_uvs * 2];
				plane->mesh->vertices[0] = 0.0f;
				plane->mesh->vertices[1] = 0.0f;
				plane->mesh->vertices[2] = 0.0f;
				plane->mesh->vertices[3] = char_w[i];
				plane->mesh->vertices[4] = 0.0f;
				plane->mesh->vertices[5] = 0.0f;
				plane->mesh->vertices[6] = 0.0f;
				plane->mesh->vertices[7] = char_h[i];
				plane->mesh->vertices[8] = 0.0f;
				plane->mesh->vertices[9] = char_w[i];
				plane->mesh->vertices[10] = char_h[i];
				plane->mesh->vertices[11] = 0.0f;

				plane->mesh->indices[0] = 0;
				plane->mesh->indices[1] = 2;
				plane->mesh->indices[2] = 1;
				plane->mesh->indices[3] = 1;
				plane->mesh->indices[4] = 2;
				plane->mesh->indices[5] = 3;

				plane->mesh->uvs[0] = 0.0f;
				plane->mesh->uvs[1] = 1.0f;
				plane->mesh->uvs[2] = 1.0f;
				plane->mesh->uvs[3] = 1.0f;
				plane->mesh->uvs[4] = 0.0f;
				plane->mesh->uvs[5] = 0.0f;
				plane->mesh->uvs[6] = 1.0f;
				plane->mesh->uvs[7] = 0.0f;

				plane->ReLoadInMemory();

				planes.push_back(plane);
				meshes.push_back(plane->mesh);
			}

			i++;
		}
		ComponentRectTransform* c = (ComponentRectTransform*)game_object->GetComponent(C_RECT_TRANSFORM);
		c->SetSize(float2(img_width, img_height));
		c->OnTransformModified();

		return true;
	}
	else
		return false;
	
}
