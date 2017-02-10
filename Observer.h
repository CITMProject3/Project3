#ifndef __OBSERVER_H__
#define __OBSERVER_H__

#include "Events.h"

class Observer
{
public:
	virtual ~Observer();
	virtual void OnNotify(void* entity, Event event);
};
#endif // !__OBSERVER_H__
