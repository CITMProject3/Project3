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

	current_lap1_time.Start();
	current_lap2_time.Start();
}

void RaceTimer::AddLap(unsigned int player)
{
	if (player == 0)
	{
		lap1_times.push_back(current_lap1_time.ReadMs());
		current_lap1_time.Start();
		current_lap1++;
	}
	else if (player == 1)
	{
		lap2_times.push_back(current_lap2_time.ReadMs());
		current_lap2_time.Start();
		current_lap2++;
	}
}

void RaceTimer::Reset()
{
	current_lap1 = 1;
	current_lap2 = 1;
	lap1_times.clear();
	lap2_times.clear();
}

int RaceTimer::GetCurrentLap(unsigned int player) const
{
	return player == 0 ? current_lap1 : current_lap2;
}

void RaceTimer::GetCurrentLapTime(unsigned int player, int& minutes, int& seconds, int& miliseconds) const
{
	double time = 0;
	if (player == 0)
	{
		time = current_lap1_time.ReadMs();
	}
	if (player == 1)
	{
		time = current_lap2_time.ReadMs();
	}

	seconds = floor(time / 1000);
	miliseconds = floor(time - seconds * 1000);
	minutes = floor(seconds / 60);
	seconds = seconds - (minutes * 60);
}

double RaceTimer::GetCurrentLapTime(unsigned int player) const
{
	return player == 0 ? current_lap1_time.ReadMs() : current_lap2_time.ReadMs();
}

bool RaceTimer::GetLapTime(unsigned int player, int lap, int & minutes, int & seconds, int & miliseconds) const
{
	double time = 0;
	if (player == 0)
	{
		if(lap > lap1_times.size() || lap <= 0)
			return false;
		time = lap1_times[lap - 1];
	}
	else if (player == 1)
	{
		if (lap > lap2_times.size() || lap <= 0)
			return false;
		time = lap2_times[lap - 1];
	}

	seconds = floor(time / 1000);
	miliseconds = floor(time - seconds * 1000);
	minutes = floor(seconds / 60);
	seconds = seconds - (minutes * 60);

	return true;
}

bool RaceTimer::GetAllLapsTime(unsigned int player, int & minutes, int & seconds, int & miliseconds) const
{
	double time = 0;
	for (int i = 0; i < player == 0 ? lap1_times.size() : lap2_times.size(); i++)
	{
		time += player == 0 ? lap1_times[i] : lap2_times[i];
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

