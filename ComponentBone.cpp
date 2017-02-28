#include "ComponentBone.h"
#include <string>
#include "imgui\imgui.h"

ComponentBone::ComponentBone(GameObject* game_object) : Component(C_BONE, game_object)
{

}

ComponentBone::~ComponentBone()
{

}

void ComponentBone::OnInspector(bool debug)
{
	std::string str = (std::string("Bone") + std::string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{

	}
}

void ComponentBone::Save(Data& file)const
{

}

void ComponentBone::Load(Data& conf)
{

}

void ComponentBone::SetResource(ResourceFileBone* rBone)
{
	this->rBone = rBone;
}