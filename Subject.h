#ifndef __SUBJECT_H__
#define __SUBJECT_H__

#include <vector>
#include "Events.h"

class Observer;

class Subject
{
public:
	~Subject();
	void AddObserver(Observer* observer);
	void RemoveObserver(Observer* observer);
protected:
	void SendEvent(void* entity, Event event);
private:
	std::vector<Observer*> observers;
};

#endif // !__SUBJECT_H__
