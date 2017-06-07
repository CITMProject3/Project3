#ifndef __EVENTQUEUE_H__
#define __EVENTQUEUE_H__

#define MAX_PENDING 32

#include <assert.h>

class EventData;
enum EventType;

class EventQueue
{

public:

	EventQueue();
	~EventQueue();
	void PostEvent(EventData *ev);
	void ProcessEvents();

private:

	EventData *queue[MAX_PENDING];
	unsigned head, tail;

};

#endif // !__EVENTQUEUE_H__
