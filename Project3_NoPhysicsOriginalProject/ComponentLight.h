#ifndef __COMPONENT_LIGHT_H__
#define __COMPONENT_LIGHT_H__

#include "Component.h"
#include "MathGeoLib\include\MathGeoLib.h"

enum LightType 
{
	DIRECTIONAL_LIGHT = 0
};

class ComponentLight : public Component
{
public:
	ComponentLight(ComponentType type, GameObject* game_object);
	~ComponentLight();

	void OnInspector();

	void Save(Data& file)const;
	void Load(Data& conf);

	LightType GetLightType()const;
	float GetIntensity()const;
	float3 GetColor()const;
	float3 GetDirection()const;
private:

	void DirectionalLightInspector();

private:
	LightType light_type = LightType::DIRECTIONAL_LIGHT;
	float intensity = 1.0f;
	math::float3 color = math::float3::one;
};
#endif // !__COMPONENT_LIGHT_H__
