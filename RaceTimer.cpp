#include "RaceTimer.h"
#include <math.h>

RaceTimer::RaceTimer()
{
}

RaceTimer::~RaceTimer()
{
}

void RaceTimer::Start()
{
	Reset();

	current_lap_time.Start();
}

void RaceTimer::AddLap()
{
	lap_times.push_back(current_lap_time.ReadMs());
	current_lap_time.Start();
	current_lap++;
}

void RaceTimer::Reset()
{
	current_lap = 1;
	lap_times.clear();
}

int RaceTimer::GetCurrentLap() const
{
	return current_lap;
}

void RaceTimer::GetCurrentLapTime(int& minutes, int& seconds, int& miliseconds) const
{
	double time = current_lap_time.ReadMs();

	seconds = floor(time / 1000);
	miliseconds = floor(time - seconds * 1000);
	minutes = floor(seconds / 60);
	seconds = seconds - (minutes * 60);
}

double RaceTimer::GetCurrentLapTime() const
{
	return current_lap_time.ReadMs();
}

bool RaceTimer::GetLapTime(int lap, int & minutes, int & seconds, int & miliseconds) const
{
	if(lap > lap_times.size() || lap <= 0)
		return false;

	double time = lap_times[lap - 1];
	seconds = floor(time / 1000);
	miliseconds = floor(time - seconds * 1000);
	minutes = floor(seconds / 60);
	seconds = seconds - (minutes * 60);

	return true;
}

bool RaceTimer::GetAllLapsTime(int & minutes, int & seconds, int & miliseconds) const
{
	double time = 0;
	for (int i = 0; i < lap_times.size(); i++)
	{
		time += lap_times[i];
	}
	if (time > 0)
	{
		seconds = floor(time / 1000);
		miliseconds = floor(time - seconds * 1000);
		minutes = floor(seconds / 60);
		seconds = seconds - (minutes * 60);
		return true;
	}
	return false;
}

