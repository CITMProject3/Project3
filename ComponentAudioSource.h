#ifndef __COMPONENTAUDIOSOURCE_H__
#define __COMPONENTAUDIOSOURCE_H__

#include "Component.h"
#include <string>
#include <vector>

class Primitive;
class ResourceFileAudio;
class AudioEvent;

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

	long unsigned GetWiseID() const;

	void PlayAudio(unsigned id_audio);

private:	

	bool play_event_pending = false;
	unsigned play_event_pending_index = 0;

	void PlayEvent(unsigned index) const;
	void StopEvent(unsigned index) const;
	void StopAllEvents() const;

	std::vector<AudioEvent*> list_of_events;	
	AudioEvent *empty_event = nullptr;

	unsigned int wwise_id_go;

	float scale_factor_attenuation = 1.0f;
	Primitive *attenuation_sphere = nullptr;

	void UpdateEventSelected(unsigned int pos, AudioEvent *new_event);
	void CreateAttenuationShpere(const AudioEvent *event);
	void DeleteAttenuationShpere();
	void UpdateAttenuationSpherePos();
	void ModifyAttenuationFactor();

	void RemoveAllEvents();

	// Inspector options
	void ShowAddRemoveButtons();
	void ShowListOfEvents();
	void ShowPlayStopButtons(unsigned index);
};


#endif // !__COMPONENTAUDIOSOURCE_H__
