#ifndef __COMPONENTAUDIOLISTENER_H__
#define __COMPONENTAUDIOLISTENER_H__

#include "Component.h"

class ComponentAudioListener : public Component
{
public:

	ComponentAudioListener(ComponentType type, GameObject* game_object);
	~ComponentAudioListener();

	void Update();

	void OnInspector(bool debug);

	void Save(Data& file) const;
	void Load(Data& conf);

	void Remove();

private:

	unsigned int listener_id;			 // 0 (first listener) - 7 (last listener)
};


#endif // !__COMPONENTAUDIOLISTENER_H__
