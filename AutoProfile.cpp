#include "AutoProfile.h"
#include "SDL\include\SDL_timer.h"
#include "Globals.h"
#include "Profiler.h"

uint64_t AutoProfile::frequency = 0;

AutoProfile::AutoProfile(const char* name) : name(name)
{
	if(frequency == 0)
		frequency = SDL_GetPerformanceFrequency();
	start_time = SDL_GetPerformanceCounter();
}

AutoProfile::~AutoProfile()
{
	uint64_t end_time = SDL_GetPerformanceCounter();
	double elapsed_time = 1000.0 * (double(end_time - start_time) / double(frequency));

	g_Profiler.StoreSample(name, elapsed_time);
	
}