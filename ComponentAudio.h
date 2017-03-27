#ifndef __COMPONENTAUDIO_H__
#define __COMPONENTAUDIO_H__

#include "Component.h"
#include <string>

class ResourceFileAudio;
struct AudioEvent;

class ComponentAudio : public Component
{
public:

	ComponentAudio(ComponentType type, GameObject* game_object);
	~ComponentAudio();

	void Update();

	void OnInspector(bool debug);

	void Save(Data& file) const;
	void Load(Data& conf);
	
	void Remove();

	void OnPlay();
	void OnStop();

private:

	ResourceFileAudio *rc_audio = nullptr;
	AudioEvent *current_event = nullptr;

	std::string event_selected;
	unsigned int event_id = 0;				// only used when loading components from a saved scene.

	unsigned wwise_id_go;

};


#endif // !__COMPONENTAUDIO_H__
