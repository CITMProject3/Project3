#include "EventQueue.h"

#include "EventData.h"
#include "EventLinkGos.h"

EventQueue::EventQueue()
{
	head = tail = 0;
	for (unsigned i = 0; i < MAX_PENDING; ++i)
		queue[i] = nullptr;

}

EventQueue::~EventQueue()
{
	for (unsigned i = 0; i < MAX_PENDING; ++i)
	{
		if (queue[i] != nullptr)
			delete queue[i];
	}
}

void EventQueue::PostEvent(EventData *ev)
{
	// TODO: Expand ring buffer queue when is completely full
	assert((tail + 1) % MAX_PENDING != head);
	queue[tail] = ev;
	tail = (tail + 1) % MAX_PENDING;
}

void EventQueue::ProcessEvents()
{
	// Processing all events on queue
	while (head != tail)
	{
		queue[head]->Process();
		delete queue[head];
		queue[head] = nullptr;
		head = (head + 1) % MAX_PENDING;
	}
}
