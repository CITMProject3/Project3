#include "Time.h"
#include "SDL\include\SDL_timer.h"

uint64_t Time::frequency = 0;

Time::Time()
{
	if (frequency == 0)
		frequency = SDL_GetPerformanceFrequency();
	started_at = SDL_GetPerformanceCounter();
	frame_started_at = started_at;
}

Time::~Time()
{}

void Time::UpdateFrame()
{
	real_delta_time = (float)((double(SDL_GetPerformanceCounter() - frame_started_at) / double(frequency)));

	//Game is running
	if (game_started_at > 0 && !game_paused)
	{
		delta_time = real_delta_time * time_scale;
		++frame_count;
	}
	
	frame_started_at = SDL_GetPerformanceCounter();

	time_unitary += real_delta_time;
	
}

double Time::RealTimeSinceStartup()const
{
	return (double(SDL_GetPerformanceCounter() - started_at) / double(frequency));
}

void Time::Play()
{
	if (game_paused)
	{
		time_paused += SDL_GetPerformanceCounter() - pause_started_at;
		game_paused = false;	
	}
	else
	{
		game_started_at = SDL_GetPerformanceCounter();
		time_paused = 0;
	}
}

void Time::Stop()
{
	game_paused = false;
	game_started_at = 0;
	time_paused = 0;
}

void Time::Pause()
{
	game_paused = true;
	pause_started_at = SDL_GetPerformanceCounter();
}

double Time::TimeSinceGameStartup() const
{
	double ret = 0;
	if (game_started_at > 0)
	{
		ret = (double(SDL_GetPerformanceCounter() - game_started_at) / double(frequency));
		if (game_paused)
		{
			ret -= (double(SDL_GetPerformanceCounter() - pause_started_at) / double(frequency));
		}
	}
	return ret;
}

unsigned int Time::GetFrameCount() const
{
	return frame_count;
}

float Time::DeltaTime() const
{
	return (game_paused || started_at == 0) ? 0 : delta_time;
}

float Time::RealDeltaTime() const
{
	return real_delta_time;
}

float Time::GetTimeScale() const
{
	return time_scale;
}

void Time::SetTimeScale(float time_scale)
{
	if (time_scale >= 0)
		this->time_scale = time_scale;
}

float Time::GetUnitaryTime()
{
	return time_unitary;
}




