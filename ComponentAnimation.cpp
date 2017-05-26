#include "ComponentAnimation.h"
#include "imgui\imgui.h"
#include "ResourceFileAnimation.h"
#include "Data.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentBone.h"
#include "ResourceFileMesh.h"
#include "ResourceFileBone.h"
#include "ModuleFileSystem.h"

#include "ModuleEditor.h"

#include "Time.h"
#include "Brofiler\include\Brofiler.h"

bool Animation::Advance(float dt)
{
	time += dt;

	if (time >= GetDuration())
	{
		if (loopable == false)
		{
			//So we keep last frame
			time = GetDuration();
			return false;
		}
		else
		{
			time = time - GetDuration();
		}
	}
	return true;
}

float Animation::GetDuration()
{
	return ((float)end_frame - (float)start_frame) / ticks_per_second;
}

void Animation::SetFrameRatio(float ratio)
{
	if (ratio >= 0 && ratio <= 1)
		time = GetDuration() * ratio;
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
			PlayAnimation(current_animation->index);
		}

		ImGui::Text("Lock animation frame");
		static float ratio;
		if (ImGui::SliderFloat("##frameSlider", &ratio, 0.0f, 1.0f))
			LockAnimationRatio(ratio);

		ImGui::Text("Animations size: %i", animations.size());
		ImGui::Separator();
		ImGui::Separator();
		for (uint i = 0; i < animations.size(); i++)
		{
			if (renaming_animation == i)
			{
				static char new_name[128];
				if (ImGui::InputText("##", new_name, 128, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					animations[renaming_animation].name = new_name;
					renaming_animation = -1;
					new_name[0] = '\0';
				}
			}
			else
				ImGui::Text(animations[i].name.c_str());
			if (ImGui::IsItemClicked(1))
			{
				popup_animation = i;
				ImGui::OpenPopup("AnimPopup");
			}

			ImGui::Separator();

			std::string loop_label = std::string("Loop##") + std::string(std::to_string(i));
			ImGui::Checkbox(loop_label.c_str(), &animations[i].loopable);

			bool isCurrent = (&animations[i] == current_animation);
			std::string current_label = std::string("CurrentAnimation##") + std::string(std::to_string(i));

			if (ImGui::Checkbox(current_label.c_str(), &isCurrent))
			{
				if (isCurrent == true)
				{
					PlayAnimation(i, 2.0f);
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

			float ticksPerSecond = animations[i].ticks_per_second;
			std::string speed_label = std::string("Speed##") + std::string(std::to_string(i));
			if (ImGui::InputFloat(speed_label.c_str(), &ticksPerSecond))
			{
				if (ticksPerSecond >= 0)
					animations[i].ticks_per_second = ticksPerSecond;
			}
			ImGui::Separator();
			ImGui::Separator();
		}

		if (ImGui::BeginPopup("AnimPopup"))
		{
			if (ImGui::MenuItem("Rename"))
			{
				renaming_animation = popup_animation;
			}

			if (ImGui::MenuItem("Delete"))
			{
				RemoveAnimation(popup_animation);
			}
			ImGui::EndPopup();
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
	if (animations.size() > 0)
	{
		for (uint i = 0; i < animations.size(); i++)
		{
			if (current_animation == &animations[i])
			{
				data.AppendInt("current_animation", i);
				break;
			}
		}
	}
	else
		data.AppendInt("current_animation", -1);

	data.AppendArray("animations");

	for (uint i = 0; i < animations.size(); i++)
	{
		Data anim_data;
		anim_data.AppendString("name", animations[i].name.c_str());

		anim_data.AppendUInt("start_frame", animations[i].start_frame);
		anim_data.AppendUInt("end_frame", animations[i].end_frame);

		anim_data.AppendFloat("ticks_per_second", animations[i].ticks_per_second);

		anim_data.AppendBool("loopable", animations[i].loopable);
		
		data.AppendArrayValue(anim_data);
	}

	file.AppendArrayValue(data);
}

void ComponentAnimation::Load(Data& conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	const char* path = conf.GetString("path");

	rAnimation = (ResourceFileAnimation*)App->resource_manager->LoadResource(path, ResourceFileType::RES_ANIMATION);

	for (uint i = 0; i < conf.GetArraySize("animations"); i++)
	{
		Data anim_data = conf.GetArray("animations", i);
		AddAnimation(anim_data.GetString("name"), anim_data.GetUInt("start_frame"), anim_data.GetUInt("end_frame"), anim_data.GetFloat("ticks_per_second"));
		animations[animations.size() - 1].loopable = anim_data.GetBool("loopable");
	}

	int current_anim = conf.GetInt("current_animation");
	if (current_anim != -1)
	{
		current_animation = &animations[current_anim];
	}
}
//-------------------------------------------

//Single animation management----------------
void ComponentAnimation::AddAnimation()
{
	//Just some simple and stupid way to avoid name duplication
	std::string new_name = "Default";
	uint defCount = 0;
	for (uint i = 0; i < animations.size(); i++)
	{
		if (animations[i].name.find("Default") != std::string::npos)
			defCount++;
	}
	if (defCount > 0)
		new_name.append(std::to_string(defCount));

	AddAnimation(new_name.c_str(), 0, rAnimation->full_duration, rAnimation->ticks_per_second);
}

void ComponentAnimation::AddAnimation(const char* name, uint init, uint end, float ticksPerSec)
{
	int index = -1;
	if (current_animation != nullptr)
		index = current_animation->index;

	Animation animation;
	animation.name = name;
	animation.start_frame = init;
	animation.end_frame = end;
	animation.ticks_per_second = ticksPerSec;
	animation.index = animations.size();
	animations.push_back(animation);

	if (index != -1)
		current_animation = &animations[index];
}

void ComponentAnimation::RemoveAnimation(uint index)
{
	if (current_animation == (animations.begin() + index)._Ptr)
		current_animation = nullptr;
	animations.erase(animations.begin() + index);
}

void ComponentAnimation::PlayAnimation(uint index, float blend_time, bool keepBlend)
{
	if (index < animations.size())
	{
		if (current_animation != nullptr && (&animations[index] != current_animation))
		{
			if (blend_time > 0 && playing == true)
			{
				if (keepBlend == false)
				{
					blend_animation = current_animation;
					this->blend_time = 0.0f;
				}

				blend_time_duration = blend_time;
			}
		}
		current_animation = &animations[index];
		current_animation->time = 0;
		playing = true;
	}
}

void ComponentAnimation::PlayAnimation(const char* name, float blendTime, bool keepBlend)
{
	if (current_animation->name != name)
	{
		for (uint i = 0; i < animations.size(); i++)
		{
			if (animations[i].name == name)
			{
				PlayAnimation(i, blendTime);
				return;
			}
		}
	}
}

void ComponentAnimation::LockAnimationRatio(float ratio)
{
	if (current_animation != nullptr)
	{
		current_animation->SetFrameRatio(ratio);
		blend_animation = nullptr;
		UpdateBonesTransform(current_animation, blend_animation, 0.0f);
		UpdateMeshAnimation(game_object);
		playing = false;
	}
}

void ComponentAnimation::LinkChannels()
{
	std::vector<GameObject*> gameObjects;
	game_object->CollectAllChilds(gameObjects);

	for (uint i = 0; i < rAnimation->num_channels; i++)
	{
		for (uint g = 0; g < gameObjects.size(); g++)
		{
			if (gameObjects[g]->name == rAnimation->channels[i].name.c_str() && gameObjects[g]->GetComponent(C_BONE))
			{
				links.push_back(Link(gameObjects[g], &rAnimation->channels[i]));
				break;
			}
		}
	}
}

void ComponentAnimation::LinkBones()
{
	std::map<std::string, ComponentMesh*> meshes;
	std::vector<ComponentBone*> bones;
	CollectMeshesBones(game_object, meshes, bones);

	for (uint i = 0; i < bones.size(); i++)
	{
		std::string string = bones[i]->GetResource()->mesh_path;
		string = App->file_system->GetNameFromPath(string); //Just for old loaded bones
		std::map<std::string, ComponentMesh*>::iterator it = meshes.find(string);
		if (it != meshes.end())
		{
			it->second->AddBone(bones[i]);
		}
	}

	//Iterate all meshes and create bones-weight buffers
	for (map<string, ComponentMesh*>::iterator mesh_it = meshes.begin(); mesh_it != meshes.end(); ++mesh_it)
		mesh_it->second->InitAnimBuffers();
}

void ComponentAnimation::LinkAnimation()
{
		LinkChannels();
		LinkBones();
		linked = true;
}

const char* ComponentAnimation::GetResourcePath()
{
	return (rAnimation != nullptr ? rAnimation->GetFile() : nullptr);
}

void ComponentAnimation::SetResource(ResourceFileAnimation* resource)
{
	rAnimation = resource;
}

bool ComponentAnimation::StartAnimation()
{
	if (linked == false)
	{
		LOG("[ERROR] The animation of %s is not linked and is trying to be played.", game_object->name);
		App->editor->DisplayWarning(WarningType::W_ERROR, "The animation of %s is not linked and is trying to be played", game_object->name);
	}

	if (current_animation != nullptr)
		started = true;
	return started;
}

void ComponentAnimation::Update()
{
	BROFILER_CATEGORY("ComponentAnimation::Update", Profiler::Color::Red)

	if (App->IsGameRunning())
	{
		if (game_started == false)
		{
			if (current_animation != nullptr)
			{
				PlayAnimation(current_animation->index);
			}
			game_started = true;
		}

		if (playing == true)
		{
			if (started == false)
				if (StartAnimation() == false)
					return;
			
			float blend_ratio = 0.0f;

			if (blend_animation != nullptr)
			{
				blend_time += time->DeltaTime();
				if (blend_animation->Advance(time->DeltaTime()) == false)
				{
					blend_animation = nullptr;
				}
				else
				{
					blend_ratio = blend_time / blend_time_duration;
					if (blend_ratio >= 1.0f)
						blend_animation = nullptr;
				}
			}
			
			if (current_animation->Advance(time->DeltaTime()) == false)
			{
				playing = false;
			}
			BROFILER_CATEGORY("ComponentAnimation::UpdateBonesTransform", Profiler::Color::BlueViolet)
			UpdateBonesTransform(current_animation, blend_animation, blend_ratio);
		}
	}

	BROFILER_CATEGORY("ComponentAnimation::UpdateMeshAnimation", Profiler::Color::Cyan)
	UpdateMeshAnimation(game_object); //Do it always
}

//-------------------------------------------
void ComponentAnimation::UpdateBonesTransform(const Animation* settings, const Animation* blend, float blendRatio)
{
	BROFILER_CATEGORY("ComponentAnimation::UpdateBonesTransform", Profiler::Color::Orange)

	uint current_frame = settings->start_frame + settings->ticks_per_second * settings->time;

	for (uint i = 0; i < links.size(); i++)
	{
		GameObject *go = links[i].gameObject;

		float3 position = GetChannelPosition(links[i], current_frame, go->transform->GetPosition(), *settings);
		Quat rotation = GetChannelRotation(links[i], current_frame, go->transform->GetRotation(), *settings);
		float3 scale = GetChannelScale(links[i], current_frame, go->transform->GetScale(), *settings);

		if (blend != nullptr)
		{
			uint blend_frame = blend->start_frame + blend->ticks_per_second * blend->time;
			position = float3::Lerp(GetChannelPosition(links[i], blend_frame, go->transform->GetPosition(), *blend), position, blendRatio);
			rotation = Quat::Slerp(GetChannelRotation(links[i], blend_frame, go->transform->GetRotation(), *blend), rotation, blendRatio);
			scale = float3::Lerp(GetChannelScale(links[i], blend_frame, go->transform->GetScale(), *blend), scale, blendRatio);
		}

		go->transform->SetPosition(position);
		go->transform->SetRotation(rotation);
		go->transform->SetScale(scale);
	}
}

float3 ComponentAnimation::GetChannelPosition(Link& link, float current_frame, float3 default, const Animation& settings)
{
	float3 position = default;

	if (link.channel->HasPosKey())
	{
		std::map<double, float3>::iterator previous = link.channel->GetPrevPosKey(current_frame);
		std::map<double, float3>::iterator next = link.channel->GetNextPosKey(current_frame);

		if (next == link.channel->positionKeys.end())
			next = previous;

		//If both keys are the same, no need to blend
		if (previous == next)
			position = previous->second;
		else //blend between both keys
		{
			//0 to 1
			float ratio = (current_frame - previous->first) / (next->first - previous->first);
			position = previous->second.Lerp(next->second, ratio);
		}
	}

	return position;
}

Quat ComponentAnimation::GetChannelRotation(Link& link, float current_frame, Quat default, const Animation& settings)
{
	Quat rotation = default;

	if (link.channel->HasRotKey())
	{
		std::map<double, Quat>::iterator previous = link.channel->GetPrevRotKey(current_frame);
		std::map<double, Quat>::iterator next = link.channel->GetNextRotKey(current_frame);

		if (next == link.channel->rotationKeys.end())
			next = previous;

		//If both keys are the same, no need to blend
		if (previous == next)
			rotation = previous->second;
		else //blend between both keys
		{
			//0 to 1
			float ratio = (current_frame - previous->first) / (next->first - previous->first);
			rotation = previous->second.Slerp(next->second, ratio);
		}
	}
	return rotation;
}

float3 ComponentAnimation::GetChannelScale(Link& link, float current_frame, float3 default, const Animation& settings)
{
	float3 scale = default;

	if (link.channel->HasScaleKey())
	{
		std::map<double, float3>::iterator previous = link.channel->GetPrevScaleKey(current_frame);
		std::map<double, float3>::iterator next = link.channel->GetPrevScaleKey(current_frame);

		if (next == link.channel->scaleKeys.end())
			next = previous;

		//If both keys are the same, no need to blend
		if (previous == next)
			scale = previous->second;
		else //blend between both keys
		{
			//0 to 1
			float ratio = (current_frame - previous->first) / (next->first - previous->first);
			scale = previous->second.Lerp(next->second, ratio);
		}
	}
	return scale;
}

void ComponentAnimation::CollectMeshesBones(GameObject* gameObject, std::map<std::string, ComponentMesh*>& meshes, std::vector<ComponentBone*>& bones)
{
	ComponentMesh* mesh = (ComponentMesh*)gameObject->GetComponent(C_MESH);
	if (mesh != nullptr)
	{
		if (mesh->GetResource() != nullptr)
			meshes[App->file_system->GetNameFromPath(mesh->GetResource()->GetFile())] = mesh;
	}
	ComponentBone* bone = (ComponentBone*)gameObject->GetComponent(C_BONE);
	if (bone != nullptr)
	{
		bones.push_back(bone);
	}

	for (std::vector<GameObject*>::const_iterator it = gameObject->GetChilds()->begin(); it != gameObject->GetChilds()->end(); it++)
	{
		CollectMeshesBones(*it, meshes, bones);
	}
}

void ComponentAnimation::UpdateMeshAnimation(GameObject* gameObject)
{
	BROFILER_CATEGORY("ComponentAnimation::UpdateMeshAnimation", Profiler::Color::Orange)
	ComponentMesh* mesh = (ComponentMesh*)gameObject->GetComponent(C_MESH);
	if (mesh != nullptr && mesh->HasBones() == true)
	{
		mesh->DeformAnimMesh();
	}

	for (std::vector<GameObject*>::const_iterator it = gameObject->GetChilds()->begin(); it != gameObject->GetChilds()->end(); it++)
	{
		UpdateMeshAnimation(*it);
	}
}


