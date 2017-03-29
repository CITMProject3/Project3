#ifndef __EVENTLINKGOS_H__
#define __EVENTLINKGOS_H__

#include "EventData.h"

class GameObject;

class EventLinkGos : public EventData
{

private:

	unsigned int uuid_to_assign;
	GameObject **pointer_to_go;

public:

	EventLinkGos(GameObject **pointer_to_go, unsigned int uuid_to_assign);
	bool Process();
};


#endif // !__EVENTLINKGOS_H__
