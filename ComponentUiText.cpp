#include "ComponentUiText.h"
#include "Component.h"
#include "Application.h"
#include "imgui\imgui.h"
#include "ModuleRenderer3D.h"
#include "font/ysglfontdata.h"


ComponentUiText::ComponentUiText(GameObject * game_object) : Component(C_UI_TEXT, game_object)
{
	text.assign("New Text...............................................ydyrdryrydtuuvbhl,hblkkbljbhlhvjvgktyfiydrdrtesesysery.");
}

void ComponentUiText::Update(float dt)
{
	int bitmapList = 0;

	// Add bitmap to glList
	//ysGlMakeFontBitmapDisplayList()

	// Write string into Bitmap
	YsGlUseFontBitmap20x32(bitmapList);
}

void ComponentUiText::PostUpdate()
{
}

void ComponentUiText::CleanUp()
{
}

void ComponentUiText::OnInspector(bool debug)
{
	std::string str = (std::string("UI Text##") + std::to_string(uuid));
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
	}

	ImGui::Text("Text");
	
	buffer[200];
	//for (int i = 0; i < text.length(); i++) { buffer[i]= text[i]; }

	if (ImGui::InputText("", buffer, 200, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		// text changed
		//text.assign(buffer);
	}
}

void ComponentUiText::SetText(std::string t)
{
	text.assign(t.c_str());
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
	const char* t = conf.GetString("text");
	//text.assign(t);
}