#include "Application.h"
#include "Component.h"
#include "GameObject.h"
#include "Globals.h"
#include "Data.h"

Component::Component(ComponentType type, GameObject* game_object) : type (type), game_object(game_object)
{
	uuid = uuid = App->rnd->RandomInt();
}

Component::~Component()
{}

void Component::OnInspector()
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

void Component::Update()
{
}

void Component::OnTransformModified()
{}

void Component::Save(Data & file) const
{
}

void Component::Remove()
{
	game_object->RemoveComponent(this);
}

void Component::Load(Data & config)
{}

