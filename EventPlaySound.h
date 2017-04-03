#ifndef __EVENTPLAYSOUND_H__
#define __EVENTPLAYSOUND_H__

#include "EventData.h"

class ComponentAudioSource;

class EventPlaySound : public EventData
{

private:

	const ComponentAudioSource *audio;

public:

	EventPlaySound(ComponentAudioSource *audio);
	bool Process();
};


#endif // !__EVENTPLAYSOUND_H__

