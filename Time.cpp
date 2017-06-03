#include "Time.h"
#include "SDL\include\SDL_timer.h"
#include "Globals.h"

uint64_t Time::frequency = 0;

Time::Time()
{
	if (frequency == 0)
		frequency = SDL_GetPerformanceFrequency();
	started_at = SDL_GetPerformanceCounter();
	frame_started_at = started_at;
	SetMaxFPS(max_fps);
}

Time::~Time()
{}

void Time::UpdateFrame()
{
	real_delta_time = (float)((double(SDL_GetPerformanceCounter() - frame_started_at) / double(frequency)));
	real_frame_count++;

	//Frame delay to cap FPS
	if (capped_ms > 0 && real_delta_time < capped_ms)
	{
		SDL_Delay((capped_ms - real_delta_time) * 1000.0);
		real_delta_time = capped_ms;
	}

	second_counter += real_delta_time;
	if (second_counter >= 1)
	{
		second_counter = 0;
		last_fps = real_frame_count;
		real_frame_count = 0;
	}

	//Game is running
	if (game_started_at > 0 && !game_paused)
	{
		delta_time = real_delta_time * time_scale;
		++frame_count;
	}
	
	frame_started_at = SDL_GetPerformanceCounter();

	time_unitary += real_delta_time;

	if (time_unitary > 1.0f)
	{
		time_unitary = time_unitary - 1.0f;
	}
	//LOG("DELTA TIME %f", delta_time);
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

int Time::GetFPS()const
{
	return last_fps;
}

int Time::GetMaxFPS()const
{
	return max_fps;
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

void Time::SetMaxFPS(int max_fps)
{
	this->max_fps = max_fps;
	if (max_fps == 0)
	{
		this->max_fps = -1;
		capped_ms = 0;
	}
	else
		capped_ms = 1.0f / (double)max_fps;

}

float Time::GetUnitaryTime()
{
	return time_unitary;
}




