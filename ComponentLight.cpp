#include "ComponentLight.h"
#include "imgui\imgui.h"
#include "Data.h"
#include <string>
#include "GameObject.h"

using namespace std;

ComponentLight::ComponentLight(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
}

ComponentLight::~ComponentLight()
{
}

void ComponentLight::OnInspector()
{
	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::Text("Light type: ");
		ImGui::SameLine();
		string light_type_name = "";
		switch (light_type)
		{
		case DIRECTIONAL_LIGHT:
			light_type_name = "DirectionalLight";
			break;
		}

		if (ImGui::BeginMenu(light_type_name.data()))
		{
			if (ImGui::MenuItem("Directional light"))
			{
				light_type = DIRECTIONAL_LIGHT;
			}
			ImGui::EndMenu();
		}
		ImGui::Separator();
		switch (light_type)
		{
		case DIRECTIONAL_LIGHT:
			DirectionalLightInspector();
			break;
		}
	}
}

void ComponentLight::Save(Data & file) const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);
	data.AppendInt("light_type", light_type);
	file.AppendArrayValue(data);
}

void ComponentLight::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
	light_type = (LightType)conf.GetInt("light_type");
}

LightType ComponentLight::GetLightType() const
{
	return light_type;
}

float ComponentLight::GetIntensity() const
{
	return intensity;
}

float3 ComponentLight::GetColor() const
{
	return color;
}

float3 ComponentLight::GetDirection() const
{
	return game_object->GetGlobalMatrix().WorldZ().Normalized();
}

void ComponentLight::DirectionalLightInspector()
{
	ImGui::InputFloat("Intensity: ###directional_in", &intensity);
	ImGui::ColorEdit3("Color: ###directional_col", color.ptr());
}
