#include "Application.h"
#include "LightingWindow.h"
#include "ModuleLighting.h"

LightingWindow::LightingWindow()
{}

LightingWindow::~LightingWindow()
{}

void LightingWindow::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Lighting", &active, flags);

	ImGui::Text("Ambient light");
	ImGui::Separator();
	
	ImGui::InputFloat("Intensity: ###ambient_in", &App->lighting->ambient_intensity);
	ImGui::ColorEdit3("Color: ###ambient_col", App->lighting->ambient_color.ptr());

	ImGui::Separator();

	ImGui::End();
}

