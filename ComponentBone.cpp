#include "ComponentBone.h"
#include <string>
#include "imgui\imgui.h"
#include "ResourceFileBone.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "Data.h"

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
		ImGui::Text("Num Weights: %i", rBone->numWeights);
		ImGui::Text("Offset Matrix:");
		ImGui::Text("%0.2f %0.2f %0.2f %0.2f", rBone->offset.v[0][0], rBone->offset.v[0][1], rBone->offset.v[0][2], rBone->offset.v[0][3]);
		ImGui::Text("%0.2f %0.2f %0.2f %0.2f", rBone->offset.v[1][0], rBone->offset.v[1][1], rBone->offset.v[1][2], rBone->offset.v[1][3]);
		ImGui::Text("%0.2f %0.2f %0.2f %0.2f", rBone->offset.v[2][0], rBone->offset.v[2][1], rBone->offset.v[2][2], rBone->offset.v[2][3]);
		ImGui::Text("%0.2f %0.2f %0.2f %0.2f", rBone->offset.v[3][0], rBone->offset.v[3][1], rBone->offset.v[3][2], rBone->offset.v[3][3]);
	}
}

void ComponentBone::Save(Data& file)const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendString("path", rBone->GetFile());

	file.AppendArrayValue(data);
}

void ComponentBone::Load(Data& conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	const char* path = conf.GetString("path");

	rBone = (ResourceFileBone*)App->resource_manager->LoadResource(path, ResourceFileType::RES_BONE);
}

void ComponentBone::SetResource(ResourceFileBone* rBone)
{
	this->rBone = rBone;
}

const char* ComponentBone::GetResourcePath() const
{
	return rBone == nullptr ? nullptr : rBone->GetFile();
}