#include "ComponentScript.h"

#include <string>
#include "imgui\imgui.h"

ComponentScript::ComponentScript(ComponentType type, GameObject* game_object) : Component(type, game_object)
{ }


ComponentScript::~ComponentScript()
{
}

void ComponentScript::Update()
{
	//Component must be active to update
	if (!IsActive())
		return;
}

void ComponentScript::OnInspector(bool debug)
{
	std::string str = (std::string("Script") + std::string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##Script");
		}

		if (ImGui::BeginPopup("delete##Script"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

		//Active
		bool is_active = IsActive();
		if (ImGui::Checkbox("###activeScript", &is_active))
		{
			SetActive(is_active);
		}
	}
}

void ComponentScript::Save(Data & file) const
{
	
}

void ComponentScript::Load(Data & conf)
{
	
}