#include "EventPlaySound.h"

#include "Application.h"
#include "ModuleAudio.h"

#include "ComponentAudioSource.h"

#include "Module.h"

EventPlaySound::EventPlaySound(ComponentAudioSource *audio)
{
	type = EventType::E_PLAY_SOUND;
	this->audio = audio;
}

bool EventPlaySound::Process()
{
	//audio->PlayEvent();
	return true;
}

