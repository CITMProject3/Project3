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

	unsigned int GetTextureId()const;

private:

	void ChangeTexture();

private:
	ResourceFileTexture* texture = nullptr;
};



#endif // !__COMPONENT_MATERIAL_H_