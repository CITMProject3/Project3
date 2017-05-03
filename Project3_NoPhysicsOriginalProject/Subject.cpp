#include "Subject.h"
#include "Observer.h"

Subject::~Subject()
{
	observers.clear();
}

void Subject::AddObserver(Observer * observer)
{
	if (observer)
		observers.push_back(observer);
}

void Subject::RemoveObserver(Observer * observer)
{
	if (observer)
		observers.erase(std::find(observers.begin(), observers.end(), observer));
}

void Subject::SendEvent(void * entity, Event event)
{
	for (std::vector<Observer*>::iterator it = observers.begin(); it != observers.end(); ++it)
		(*it)->OnNotify(entity, event);
}
