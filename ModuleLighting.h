#ifndef __MODULELIGHTING_H__
#define __MODULELIGHTING_H__

#include "Module.h"
#include "MathGeoLib\include\MathGeoLib.h"

#include "Light.h"

class ComponentLight;

class ModuleLighting : public Module
{
public:
	ModuleLighting(const char* name, bool start_enabled = true);
	~ModuleLighting();

	bool Init(Data& config);
	void SaveBeforeClosing(Data& data)const;
	update_status PostUpdate();

	const LightInfo& GetLightInfo()const;

private:
	void CollectAllLights();

public: //TODO: Set this variables private. BUT! Can be editable in the LightWindow
	//One ambient
	float3 ambient_color = float3::one;
	float ambient_intensity = 0.1f;

private:
	LightInfo light_info;
};

#endif // !__MODULELIGHTING_H__
