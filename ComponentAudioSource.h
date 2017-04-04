#ifndef __COMPONENTAUDIOSOURCE_H__
#define __COMPONENTAUDIOSOURCE_H__

#include "Component.h"
#include <string>

class ResourceFileAudio;
struct AudioEvent;

class ComponentAudioSource : public Component
{
public:

	ComponentAudioSource(ComponentType type, GameObject* game_object);
	~ComponentAudioSource();

	void Update();

	void OnInspector(bool debug);

	void Save(Data& file) const;
	void Load(Data& conf);
	
	void Remove();

	void OnPlay();
	void OnStop();

	const AudioEvent *GetEvent() const;
	long unsigned GetWiseID() const;

	void PlayEvent() const;
	void StopEvent() const;

	bool play_event = false;

private:	

	ResourceFileAudio *rc_audio = nullptr;
	AudioEvent *current_event = nullptr;

	std::string event_selected;
	unsigned int event_id = 0;			// only used when loading components from a saved scene.

	unsigned int wwise_id_go;

};


#endif // !__COMPONENTAUDIOSOURCE_H__
