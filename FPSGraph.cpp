#include "FPSGraph.h"
#include "Application.h"

FPSGraph::FPSGraph()
{
	App->SetMaxFPS(max_fps);
}

FPSGraph::~FPSGraph()
{}

void FPSGraph::Draw()
{
	if (!active)
		return;

	int fps = App->GetFPS();

	//Get frames
	if (frames.size() > 100) //Max seconds to show
	{
		for (int i = 1; i < frames.size(); i++)
		{
			frames[i-1] = frames[i];
		}
		frames[frames.size() - 1] = fps;
	}
	else
	{
		frames.push_back(fps);
	}
	
	ImGui::Begin("FPS Graph", &active);
	
	char text[20];
	sprintf_s(text, 20, "Frames: %d", fps);
	ImGui::Text(text);

	ImGui::PlotHistogram("Framerate", &frames[0], frames.size(), 0, NULL, 0.0f, 100.0f, ImVec2(300, 100));
	if (ImGui::SliderInt("Max FPS", &max_fps, -1, 200, NULL))
	{
		App->SetMaxFPS(max_fps);
	}
	ImGui::End();
}


