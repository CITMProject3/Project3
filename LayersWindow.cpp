#include "Application.h"
#include "LayersWindow.h"
#include "LayerSystem.h"
#include "ModuleGOManager.h"

LayersWindow::LayersWindow()
{}

LayersWindow::~LayersWindow()
{}

void LayersWindow::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Layers", &active, flags);

	vector<string>* layers = App->go_manager->layer_system->GetEditableLayerList();

	for (size_t i = 0; i < layers->size(); ++i)
	{
		ImGui::Text("Layer %i :", i);
		ImGui::SameLine();
		string id = "###layerid" + std::to_string(i);
		if (i != 0)
			ImGui::InputText(id.data(), layers->at(i)._Myptr(), layers->at(i).capacity());
		else
			ImGui::Text(layers->at(i).data());
	}

	ImGui::End();
}

