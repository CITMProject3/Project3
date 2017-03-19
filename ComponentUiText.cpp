#include "Application.h"
#include "ComponentUiText.h"
#include "ModuleResourceManager.h"
#include "ModuleGOManager.h"
#include "GameObject.h"
#include "ComponentRectTransform.h"
#include "ComponentMaterial.h"
#include "ResourceFileTexture.h"

ComponentUiText::ComponentUiText(ComponentType type, GameObject * game_object) : Component(type, game_object)
{
	text = "124124"; 
	array_values = "! @,_./0123456789$;<&?abcdefghijklmnopqrstuvwxyz";
	rows = 1;
	UImaterial = new ComponentMaterial(C_MATERIAL, nullptr);
	img_width = 100;
	img_height = 100;
}

ComponentUiText::~ComponentUiText()
{
	delete UImaterial;
}
void ComponentUiText::Update(float dt)
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

		int tmp = rows;
		ImGui::Text("Rows");
		if (ImGui::InputInt("", &tmp, 1.0f))
		{
			rows = tmp;
			GenerateFont();
		}
	}
}

void ComponentUiText::Save(Data & file) const
{
	Data data;

	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendString("text", text.c_str());

	file.AppendArrayValue(data);
}

void ComponentUiText::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
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
	return char_row;
}

float ComponentUiText::GetCharwidth() const
{
	return float(char_w)/ float(img_width);
}

float ComponentUiText::GetCharHeight() const
{
	return float(char_h) / float(img_height);
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

	string str = string("Change Text:") + text;
	if (ImGui::Begin(str.c_str(),&change_text))
	{
		ImGui::InputText("", current_text_changing._Myptr(), current_text_changing.capacity());
		if (ImGui::Button("Save ##change_text"))
		{
			text = current_text_changing;
			change_text = false;
			change_array_values = false;
			GenerateFont();
		}
		ImGui::End();
	}

}

void ComponentUiText::GenerateFont()
{
	len = array_values.length();
	char_row = len / rows;
	OnChangeTexture();
}

bool ComponentUiText::OnChangeTexture()
{
	if (UImaterial->list_textures_paths.size() > 0)
	{
		string tex_path = (*UImaterial->list_textures_paths.begin());
		ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(tex_path, ResourceFileType::RES_TEXTURE);

		if (rc_tmp == nullptr)
			return false;

		img_width = rc_tmp->GetWidth();
		img_height = rc_tmp->GetHeight();

		char_w = img_width / char_row;
		char_h = img_height / rows;

		ComponentRectTransform* c = (ComponentRectTransform*)game_object->GetComponent(C_RECT_TRANSFORM);
		c->SetSize(float2(char_w, char_h));
		c->OnTransformModified();

		return true;
	}
	else
		return false;
	
}
