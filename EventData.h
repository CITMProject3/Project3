#ifndef __EVENTINFO_H__
#define __EVENTINFO_H__

enum EventType
{
	E_LINK_GOS
};

class EventData
{
public:

	EventType type;
	virtual bool Process() = 0;
};

#endif // !__EVENTINFO_H__




