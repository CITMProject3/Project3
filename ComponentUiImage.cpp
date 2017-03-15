#include "ComponentUiImage.h"
#include "GameObject.h"
#include "ComponentMaterial.h"

ComponentUiImage::ComponentUiImage(ComponentType type, GameObject * game_object) : Component(type,game_object)
{
	UImaterial = new ComponentMaterial(C_MATERIAL,nullptr);
}

void ComponentUiImage::Update(float dt)
{
}

void ComponentUiImage::CleanUp()
{
}

void ComponentUiImage::OnInspector(bool debug)
{
	UImaterial->OnInspector(false);
}

void ComponentUiImage::Save(Data & file) const
{
}

void ComponentUiImage::Load(Data & conf)
{
}
