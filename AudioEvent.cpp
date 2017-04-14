// ----------------------
// ---- AUDIO EVENTS ----
// ----------------------

#include "AudioEvent.h"
#include "ModuleAudio.h"

AudioEvent::AudioEvent()
{
	event_call_back = &ModuleAudio::EventCallBack;
}

bool AudioEvent::IsEventPlaying() const
{
	return playing_id != 0L;
}

void AudioEvent::Unload()
{
	unload = true;
}

void AudioEvent::Unloaded()
{
	unload = false;
}

bool AudioEvent::IsReadyToUnload() const
{
	return unload && !IsEventPlaying();
}
