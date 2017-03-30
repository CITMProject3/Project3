#include "ModuleLighting.h"

#include "Application.h"
#include "ModuleGOManager.h"
#include "GameObject.h"

#include "ComponentLight.h"
#include "ComponentTransform.h"

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

const LightInfo &ModuleLighting::GetLightInfo() const
{
	return light_info;
}

void ModuleLighting::CollectAllLights()
{
	//TODO: Optimize this method to collect only dynamic lights. 
	vector<ComponentLight*> lights;
	App->go_manager->GetLightInfo(lights);

	light_info.ambient_color = ambient_color;
	light_info.ambient_intensity = ambient_intensity;

	bool has_directional = false;
	light_info.point_color.clear();
	light_info.point_positions.clear();
	for (vector<ComponentLight*>::iterator it = lights.begin(); it != lights.end(); ++it)
	{
		LightType l_type = (*it)->GetLightType();
		switch (l_type)
		{
		case DIRECTIONAL_LIGHT:
			light_info.has_directional = true;
			light_info.directional_color = (*it)->GetColor();
			light_info.directional_intensity = (*it)->GetIntensity();
			light_info.directional_direction = (*it)->GetDirection();
			break;
		case POINT_LIGHT:
			light_info.point_positions.push_back((*it)->GetGameObject()->transform->GetPosition());
			light_info.point_color.push_back((*it)->GetColor());
			break;
		}
	}
}
