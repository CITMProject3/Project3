#ifndef __COMPONENT_BONE_H__
#define __COMPONENT_BONE_H__

#include "Component.h"

class ResourceFileBone;

class ComponentBone : public Component
{
public:
	ComponentBone(GameObject* game_object);
	~ComponentBone();

	void OnInspector(bool debug);

	void Save(Data& file)const;
	void Load(Data& conf);

	void SetResource(ResourceFileBone* rBone);
private:

public:

private:
	ResourceFileBone* rBone = nullptr;
};
#endif // !__COMPONENT_LIGHT_H__

