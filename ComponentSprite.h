#ifndef __COMPONENT_SPRITE_H_
#define __COMPONENT_SPRITE_H_

#include "Component.h"
#include "MathGeoLib\include\MathGeoLib.h"

class ResourceFileTexture;

class ComponentSprite : public Component
{
public:
	ComponentSprite(ComponentType type, GameObject* game_object);
	~ComponentSprite();

	void Update();

	void OnInspector(bool debug);
	void OnTransformModified();

	unsigned int GetTextureId()const;

private:

	void ChangeTexture();

private:
	ResourceFileTexture* texture = nullptr;
	math::AABB aabb;
	math::AABB bounding_box;

	unsigned int width; //In pixels
	unsigned int height; //In pixels

public:
	math::float2 size; //With/100 & Height/100
};



#endif // !__COMPONENT_MATERIAL_H_