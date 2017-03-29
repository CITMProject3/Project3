#include "EventLinkGos.h"

#include "Application.h"
#include "ModuleGOManager.h"

EventLinkGos::EventLinkGos(GameObject **pointer_to_go, unsigned int uuid_to_assign)
{
	type = EventType::E_LINK_GOS;
	this->pointer_to_go = pointer_to_go;
	this->uuid_to_assign = uuid_to_assign;
}

bool EventLinkGos::Process()
{
	App->go_manager->LinkGameObjectPointer(pointer_to_go, uuid_to_assign);
	return true;
}

