#include "Application.h"
#include "ModuleLighting.h"
#include "ComponentLight.h"

ModuleLighting::ModuleLighting(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleLighting::~ModuleLighting()
{
}

bool ModuleLighting::Init(Data & config)
{
	ambient_intensity = config.GetFloat("ambient_intensity");
	ambient_color = config.GetFloat3("ambient_color");
	return true;
}

void ModuleLighting::SaveBeforeClosing(Data & data) const
{
	data.AppendFloat("ambient_intensity", ambient_intensity);
	data.AppendFloat3("ambient_color", ambient_color.ptr());
}

update_status ModuleLighting::PostUpdate()
{
	CollectAllLights();
	return UPDATE_CONTINUE;
}

const LightInfo & ModuleLighting::GetLightInfo() const
{
	return light_info;
}

void ModuleLighting::CollectAllLights()
{
	//TODO: Optimize this method to collect only dynamic lights. 
	ComponentLight* directional = App->go_manager->GetDirectionalLight();

	light_info.ambient_color = ambient_color;
	light_info.ambient_intensity = ambient_intensity;
	if (directional)
	{
		light_info.has_directional = true;
		light_info.directional_color = directional->GetColor();
		light_info.directional_intensity = directional->GetIntensity();
		light_info.directional_direction = directional->GetDirection();
	}
	else
		light_info.has_directional = false;
}
