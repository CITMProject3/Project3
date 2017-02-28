#include "ComponentAnimation.h"
#include "imgui\imgui.h"
#include "ResourceFileAnimation.h"
#include "Data.h"
#include "Application.h"
#include "ModuleResourceManager.h"

float Animation::GetDuration()
{
	return 0;
}

ComponentAnimation::ComponentAnimation(GameObject* game_object) : Component(C_ANIMATION, game_object)
{

}

ComponentAnimation::~ComponentAnimation()
{

}

//Base component behaviour ------------------
void ComponentAnimation::OnInspector(bool debug)
{
	if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::Checkbox("Playing", &playing))
		{
			SetAnimation(current_animation);
		}

		ImGui::Text("Animations size: %i", animations.size());
		ImGui::Separator();
		ImGui::Separator();
		for (uint i = 0; i < animations.size(); i++)
		{
			ImGui::Text(animations[i].name.c_str());
			ImGui::Separator();

			std::string loop_label = std::string("Loop##") + std::string(std::to_string(i));
			ImGui::Checkbox(loop_label.c_str(), &animations[i].loopable);

			bool isCurrent = animations[i].current;
			std::string current_label = std::string("CurrentAnimation##") + std::string(std::to_string(i));

			if (ImGui::Checkbox(current_label.c_str(), &isCurrent))
			{
				if (isCurrent == true)
				{
					SetAnimation(i, 2.0f);
				}
			}

			int start_frame = animations[i].start_frame;
			std::string startF_label = std::string("Start Frame##") + std::string(std::to_string(i));
			if (ImGui::InputInt(startF_label.c_str(), &start_frame))
			{
				if (start_frame >= 0)
					animations[i].start_frame = start_frame;
			}

			int end_frame = animations[i].end_frame;
			std::string endF_label = std::string("End Frame##") + std::string(std::to_string(i));
			if (ImGui::InputInt(endF_label.c_str(), &end_frame))
			{
				if (end_frame >= 0 && end_frame != animations[i].end_frame)
				{
					//TODO: restart animation?
					animations[i].end_frame = end_frame;
				}
			}

			float ticksPerSecond = animations[i].ticksPerSecond;
			std::string speed_label = std::string("Speed##") + std::string(std::to_string(i));
			if (ImGui::InputFloat(speed_label.c_str(), &ticksPerSecond))
			{
				if (ticksPerSecond >= 0)
					animations[i].ticksPerSecond = ticksPerSecond;
			}
			ImGui::Separator();
			ImGui::Separator();
		}

		if (ImGui::Button("Add Animation"))
		{
			AddAnimation();
		}
	}
}

void ComponentAnimation::Save(Data& file)const
{
	Data data;
	data.AppendInt("type", ComponentType::C_ANIMATION);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendString("path", rAnimation->GetFile());

	file.AppendArrayValue(data);
}

void ComponentAnimation::Load(Data& conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	const char* path = conf.GetString("path");

	rAnimation = (ResourceFileAnimation*)App->resource_manager->LoadResource(path, ResourceFileType::RES_ANIMATION);
}
//-------------------------------------------

//Single animation management----------------
void ComponentAnimation::AddAnimation()
{

}

void ComponentAnimation::AddAnimation(const char* name, uint init, uint end, float ticksPerSec)
{

}

void ComponentAnimation::SetAnimation(uint index, float blendTime)
{

}

void ComponentAnimation::SetAnimation(const char* name, float blendTime)
{

}

const char* ComponentAnimation::GetResourcePath()
{
	return (rAnimation != nullptr ? rAnimation->GetFile() : nullptr);
}

void ComponentAnimation::SetResource(ResourceFileAnimation* resource)
{
	rAnimation = resource;
}

//-------------------------------------------
void ComponentAnimation::UpdateChannelsTransform(const Animation& settings, const Animation& blend, float blendRatio)
{

}

float3 ComponentAnimation::GetChannelPosition(Link& link, float currentKey, float3 default, const Animation& settings)
{
	return float3::zero;
}

Quat ComponentAnimation::GetChannelRotation(Link& link, float currentKey, Quat default, const Animation& settings)
{
	return Quat::identity;
}

float3 ComponentAnimation::GetChannelScale(Link& link, float currentKey, float3 default, const Animation& settings)
{
	return float3::zero;
}
