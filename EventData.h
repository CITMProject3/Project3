#ifndef __EVENTINFO_H__
#define __EVENTINFO_H__

enum EventType
{
	E_LINK_GOS,
	E_PLAY_SOUND
};

class EventData
{
public:

	EventType type;
	virtual bool Process() = 0;
};

#endif // !__EVENTINFO_H__




