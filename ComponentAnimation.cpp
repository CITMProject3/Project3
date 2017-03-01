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

float Animation::GetDuration()
{
	return ((float)end_frame - (float)start_frame) / ticksPerSecond;
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
	data.AppendArray("animations");

	for (uint i = 0; i < animations.size(); i++)
	{
		Data anim_data;
		anim_data.AppendString("name", animations[i].name.c_str());

		anim_data.AppendUInt("start_frame", animations[i].start_frame);
		anim_data.AppendUInt("end_frame", animations[i].end_frame);

		anim_data.AppendFloat("ticks_per_second", animations[i].ticksPerSecond);
		anim_data.AppendFloat("duration", animations[i].duration);

		anim_data.AppendBool("loopable", animations[i].loopable);
		anim_data.AppendBool("current", animations[i].current);
		
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
		animations[animations.size() - 1].duration = anim_data.GetFloat("duration");
		animations[animations.size() - 1].loopable = anim_data.GetBool("loopable");
		if (animations[animations.size() - 1].current = anim_data.GetBool("current"))
			SetAnimation(animations.size() - 1, 0.0f);
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

	AddAnimation(new_name.c_str(), 0, rAnimation->full_duration, 24);
}

void ComponentAnimation::AddAnimation(const char* name, uint init, uint end, float ticksPerSec)
{
	Animation animation;
	animation.name = name;
	animation.start_frame = init;
	animation.end_frame = end;
	animation.ticksPerSecond = ticksPerSec;

	animations.push_back(animation);
}

void ComponentAnimation::SetAnimation(uint index, float blendTime)
{
	if (index < animations.size())
	{
		if (current_animation < animations.size() && index != current_animation)
		{
			animations[current_animation].current = false;

			if (blendTime > 0 && playing == true)
			{
				previous_animation = current_animation;
				prevAnimTime = time;
				blendTimeDuration = blendTime;
				this->blendTime = 0.0f;
			}
		}
		current_animation = index;
		animations[current_animation].current = true;
		time = 0.0f;
		playing = true;
	}
}

void ComponentAnimation::SetAnimation(const char* name, float blendTime)
{
	if (animations[current_animation].name != name)
	{
		for (uint i = 0; i < animations.size(); i++)
		{
			if (animations[i].name == name)
			{
				SetAnimation(i, blendTime);
				return;
			}
		}
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
	channelsLinked = true;
	bonesLinked = false;
}

void ComponentAnimation::LinkBones()
{
	std::map<std::string, ComponentMesh*> meshes;
	std::vector<ComponentBone*> bones;
	CollectMeshesBones(game_object, meshes, bones);

	for (uint i = 0; i < bones.size(); i++)
	{
		std::string string = bones[i]->GetResource()->mesh_path;
		std::map<std::string, ComponentMesh*>::iterator it = meshes.find(string);
		if (it != meshes.end())
		{
			it->second->AddBone(bones[i]);
		}
	}
	bonesLinked = true;
}

const char* ComponentAnimation::GetResourcePath()
{
	return (rAnimation != nullptr ? rAnimation->GetFile() : nullptr);
}

void ComponentAnimation::SetResource(ResourceFileAnimation* resource)
{
	rAnimation = resource;
}

void ComponentAnimation::Start()
{
	if (channelsLinked == false)
	{
		LinkChannels();
	}
	if (bonesLinked == false)
	{
		LinkBones();
	}

	if (animations.size() > 0)
	{
		animations[current_animation].current = true;
		started = true;
	}
}

void ComponentAnimation::Update(float dt)
{
//dt = Time::deltaTime;
	//"if" not necessary but we avoid all calculations
	if (dt > 0.0f)
	{
		if (playing == true)
		{
			if (started == false)
				Start();
			if (started == false)
				return;

			//Updating animation blend
			float blendRatio = 0.0f;
			if (blendTimeDuration > 0.0f)
			{
				prevAnimTime += dt;
				blendTime += dt;

				if (blendTime >= blendTimeDuration)
				{
					blendTimeDuration = 0.0f;
				}

				else if (prevAnimTime >= animations[previous_animation].GetDuration())
				{
					if (animations[previous_animation].loopable == true)
					{
						prevAnimTime = 0.0f;
						// + (currentFrame - endFrame);
					}
				}

				if (blendTimeDuration > 0.0f)
					blendRatio = blendTime / blendTimeDuration;
			}
			//Endof Updating animation blend

			time += dt;

			if (time > animations[current_animation].GetDuration())
			{
				if (animations[current_animation].loopable == true)
				{
					time = 0.0f;
				}
				else
				{
					playing = false;
					//TODO: is it really necessary? Not returning could end in last anim frame
					return;
				}
			}

			UpdateBonesTransform(&animations[current_animation], blendRatio > 0.0f ? &animations[previous_animation] : nullptr, blendRatio);
			UpdateMeshAnimation(game_object);
		}
	}
}

//-------------------------------------------
void ComponentAnimation::UpdateBonesTransform(const Animation* settings, const Animation* blend, float blendRatio)
{
	uint currentFrame = settings->start_frame + settings->ticksPerSecond * time;

	uint prevBlendFrame = 0;
	if (blend != nullptr)
	{
		prevBlendFrame = blend->start_frame + blend->ticksPerSecond * prevAnimTime;
	}

	for (uint i = 0; i < links.size(); i++)
	{
		ComponentTransform* transform = (ComponentTransform*)links[i].gameObject->GetComponent(C_TRANSFORM);

		float3 position = GetChannelPosition(links[i], currentFrame, transform->GetPosition(), *settings);
		Quat rotation = GetChannelRotation(links[i], currentFrame, transform->GetRotation(), *settings);
		float3 scale = GetChannelScale(links[i], currentFrame, transform->GetScale(), *settings);

		if (blend != nullptr)
		{
			position = float3::Lerp(GetChannelPosition(links[i], prevBlendFrame, transform->GetPosition(), *blend), position, blendRatio);
			rotation = Quat::Slerp(GetChannelRotation(links[i], prevBlendFrame, transform->GetRotation(), *blend), rotation, blendRatio);
			scale = float3::Lerp(GetChannelScale(links[i], prevBlendFrame, transform->GetScale(), *blend), scale, blendRatio);
		}

		transform->SetPosition(position);
		transform->SetRotation(rotation);
		transform->SetScale(scale);
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
		meshes[mesh->GetResource()->GetFile()] = mesh;
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
	ComponentMesh* mesh = (ComponentMesh*)gameObject->GetComponent(C_MESH);
	if (mesh != nullptr)
	{
		mesh->DeformAnimMesh();
	//	App->renderer3D->LoadBuffers(mesh->animMesh);
	}

	for (std::vector<GameObject*>::const_iterator it = gameObject->GetChilds()->begin(); it != gameObject->GetChilds()->end(); it++)
	{
		UpdateMeshAnimation(*it);
	}
}


