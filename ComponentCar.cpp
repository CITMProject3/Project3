#include "ComponentCar.h"
#include "GameObject.h"
#include "imgui/imgui.h"


#include <string>

using namespace std;

ComponentCar::ComponentCar(GameObject* GO) : Component(C_CAR, GO)
{
	
}

ComponentCar::~ComponentCar()
{}

void ComponentCar::Update()
{}

void ComponentCar::OnInspector(bool debug)
{
	string str = (string("Car") + string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##car");
		}

		if (ImGui::BeginPopup("delete##car"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

		ImGui::Text("Hello World");

	}
	
}

void ComponentCar::HandlePlayerInput()
{

}

void ComponentCar::OnTransformModified()
{}

void ComponentCar::Save(Data& file) const
{}

void ComponentCar::Remove()
{}

void ComponentCar::Load(Data& config)
{}


