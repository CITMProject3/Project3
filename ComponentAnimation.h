#ifndef __COMPONENT_ANIMATION_H__
#define __COMPONENT_ANIMATION_H__

#include "Component.h"
#include <string>
#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"

class GameObject;
class Channel;
class ComponentMesh;
class ComponentBone;

class ResourceFileAnimation;

struct Animation
{
	std::string name;

	uint start_frame;
	uint end_frame;

	float ticksPerSecond;
	float duration;

	bool loopable = false;
	bool current = false;

	float GetDuration();
};

class ComponentAnimation : public Component
{
	//Careful, this could be dangerous, duplicating pointers
	//TODO: try some other way
	struct Link
	{
		Link(GameObject* gameObject, Channel* channel) : gameObject(gameObject), channel(channel) {};
		GameObject* gameObject;
		Channel* channel;
	};

public:
	ComponentAnimation(GameObject* game_object);
	~ComponentAnimation();

	//Base component behaviour ------------------
	void OnInspector(bool debug);

	void Save(Data& file)const;
	void Load(Data& conf);
	//-------------------------------------------

	//Single animation management----------------
	void AddAnimation();
	void AddAnimation(const char* name, uint init, uint end, float ticksPerSec);

	void SetAnimation(uint index, float blendTime = 0.0f);
	void SetAnimation(const char* name, float blendTime = 0.0f);
	//-------------------------------------------

	const char* GetResourcePath();
	void SetResource(ResourceFileAnimation* resource);

private:
	void UpdateChannelsTransform(const Animation* settings, const Animation* blend, float blendRatio);
	float3 GetChannelPosition(Link& link, float currentKey, float3 default, const Animation& settings);
	Quat GetChannelRotation(Link& link, float currentKey, Quat default, const Animation& settings);
	float3 GetChannelScale(Link& link, float currentKey, float3 default, const Animation& settings);

	void ComponentAnimation::CollectMeshesBones(GameObject* gameObject, std::map<std::string, ComponentMesh*>& meshes, std::vector<ComponentBone*>& bones);
	void ComponentAnimation::UpdateMeshAnimation(GameObject* gameObject);

public:
	std::vector<Animation> animations;
	//Used for blending
	uint previous_animation = 0;
	uint current_animation = 0;
	bool playing = false;

private:
	ResourceFileAnimation* rAnimation;

	bool started = false;

	float prevAnimTime = 0.0f;
	float time = 0.0f;
	float blendTime = 0.0f;
	float blendTimeDuration = 0.0f;

	std::vector<Link> links;

	bool channelsLinked = false;
	bool bonesLinked = false;
};
#endif // !__COMPONENT_LIGHT_H__