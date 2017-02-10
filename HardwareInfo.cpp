#include "HardwareInfo.h"
#include "SDL\include\SDL.h"
#include <string>
#include "Globals.h"

using namespace std;

HardwareInfo::HardwareInfo()
{
	
}

HardwareInfo::~HardwareInfo()
{}

void HardwareInfo::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Hardware Info", &active);

	ImGui::Text("CPUs: "); 
	ImGui::SameLine(); ImGui::TextColored(TEXT_COLORED, "%d (Cache: %dkb)", SDL_GetCPUCount(), SDL_GetCPUCacheLineSize());
	
	ImGui::Text("System RAM: ");
	ImGui::SameLine(); ImGui::TextColored(TEXT_COLORED, "%.2fG", (float)SDL_GetSystemRAM()/1000);

	ImGui::Text("Caps: ");
	ImGui::SameLine();
	string buff = "";

	if (SDL_Has3DNow) buff +="3DNow, ";
	if (SDL_HasAVX) buff += "AVX, ";
	/*if (SDL_HasAVX2) buff += "AVX2, ";*/
	if (SDL_HasAltiVec) buff += "AltiVec, ";
	if (SDL_HasMMX) buff += "MMX, ";
	if (SDL_HasRDTSC) buff += "RDTSC, ";
	if (SDL_HasSSE) buff += "SSE, ";
	if (SDL_HasSSE2) buff += "SSE2, ";
	if (SDL_HasSSE3) buff += "SSE3, ";
	if (SDL_HasSSE41) buff += "SSE41, ";
	if (SDL_HasSSE41) buff += "SSE42, ";

	ImGui::TextColored(TEXT_COLORED, buff.c_str());

	ImGui::End();
}

