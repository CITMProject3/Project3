#include "WindowOptions.h"
#include "SDL\include\SDL.h"
#include "ModuleWindow.h"

WindowOptions::WindowOptions()
{
	brightness = SDL_GetWindowBrightness(App->window->window);
}

WindowOptions::~WindowOptions()
{}

void WindowOptions::Draw()
{
	if (!active)
		return;

	ImGui::Begin("WindowOptions", &active);

	//Brightness
	ImGui::SliderFloat("Brightness", &brightness, 0.0f, 1.0f);

	//Size Options
	if (ImGui::Checkbox("Fullscreen", &fullscreen))
	{
		App->window->SetFullScreen(fullscreen);
	}
	if (ImGui::Checkbox("Borderless", &borderless))
	{
		App->window->SetFullScreen(borderless);
	}
	if (ImGui::Checkbox("Resizable", &resizable))
	{
		App->window->SetFullScreen(resizable);
	}
	if (ImGui::Checkbox("FullDesktop", &full_desktop))
	{
		App->window->SetFullScreen(full_desktop);
	}

	//TODO: Sliders for windows size

	ImGui::End();
}

