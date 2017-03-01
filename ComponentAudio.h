#ifndef __COMPONENTAUDIO_H__
#define __COMPONENTAUDIO_H__

#include "Component.h"

class ResourceFileMesh;

class ComponentAudio : public Component
{
public:

	ComponentAudio(ComponentType type, GameObject* game_object);
	~ComponentAudio();

	void Update();

	void OnInspector(bool debug);

	void Save(Data& file)const;
	void Load(Data& conf);
	
	void Remove();

private:
	
	//ResourceFileMesh* rc_mesh = nullptr;
	//Mesh* mesh = nullptr;
	//math::AABB aabb; //Local one
	//math::AABB bounding_box; //In the world position
};


#endif // !__COMPONENTAUDIO_H__
