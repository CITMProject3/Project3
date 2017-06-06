#include "ComponentLight.h"
#include "imgui\imgui.h"
#include "Data.h"
#include <string>
#include "GameObject.h"
#include "DebugDraw.h"
#include "ComponentTransform.h"

#include "Brofiler\include\Brofiler.h"

using namespace std;

ComponentLight::ComponentLight(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
}

ComponentLight::~ComponentLight()
{
}

void ComponentLight::Update()
{
	BROFILER_CATEGORY("ComponentLight::Update", Profiler::Color::Yellow)
	float4x4 matrix = game_object->transform->GetGlobalMatrix();
	float3 forward = matrix.Col3(2);
	g_Debug->AddArrow(matrix.TranslatePart(), forward, g_Debug->yellow, 3.0f);
}

void ComponentLight::OnInspector(bool debug)
{
	string str = (string("Light") + string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##light");
		}

		if (ImGui::BeginPopup("delete##light"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

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
	data.AppendFloat("intensity", intensity);
	data.AppendFloat3("color", color.ptr());
	file.AppendArrayValue(data);
}

void ComponentLight::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");
	light_type = (LightType)conf.GetInt("light_type");
	intensity = conf.GetFloat("intensity");
	color = conf.GetFloat3("color");
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

//TODO: add a light to render module and remove previous light?
void ComponentLight::SetType(LightType type)
{
	light_type = type;
}
