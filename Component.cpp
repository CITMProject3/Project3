#include "Application.h"
#include "Component.h"
#include "GameObject.h"
#include "Globals.h"
#include "Data.h"
#include "Random.h"

Component::Component(ComponentType type, GameObject* game_object) : type (type), game_object(game_object)
{
	uuid = uuid = App->rnd->RandomInt();
}

Component::~Component()
{}

ComponentType Component::GetType() const
{
	return type;
}

GameObject * Component::GetGameObject() const
{
	return game_object;
}

unsigned int Component::GetUUID() const
{
	return uuid;
}

bool Component::IsActive()
{
	return active;
}

void Component::SetActive(bool value)
{
	active = value;
}

void Component::Remove()
{
	game_object->RemoveComponent(this);
}