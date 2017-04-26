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
	race_timer = 0;
}

void RaceTimer::Update(float dt)
{
	race_timer += dt;
}

void RaceTimer::AddLap(unsigned int player)
{
	if (player == 0)
	{
		double lap_time = race_timer;
		if (lap1_times.size() > 0)
			lap_time -= lap1_times.back();

		lap1_times.push_back(lap_time);
		current_lap1++;
	}
	else if (player == 1)
	{
		double lap_time = race_timer;
		if (lap2_times.size() > 0)
			lap_time -= lap2_times.back();

		lap2_times.push_back(lap_time);
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

double RaceTimer::GetRaceTime(int& minutes, int& seconds, int& miliseconds)const
{
	double time = race_timer;

	seconds = floor(time);
	miliseconds = (time - seconds) * 1000;
	minutes = floor(seconds / 60);
	seconds = seconds - (minutes * 60);

	return time;
}

int RaceTimer::GetCurrentLap(unsigned int player) const
{
	return player == 0 ? current_lap1 : current_lap2;
}

void RaceTimer::GetCurrentLapTime(unsigned int player, int& minutes, int& seconds, int& miliseconds) const
{
	double time = GetCurrentLapTime(player);

	seconds = floor(time);
	miliseconds = (time - seconds) * 1000;
	minutes = floor(seconds / 60);
	seconds = seconds - (minutes * 60);
}

double RaceTimer::GetCurrentLapTime(unsigned int player) const
{
	double time = race_timer;
	if (player == 0)
	{
		if (lap1_times.size() > 0)
			time -= lap1_times.back();
	}
	else if (player == 1)
	{
		if (lap2_times.size() > 0)
			time -= lap2_times.back();
	}
	return time;
}

bool RaceTimer::GetLapTime(unsigned int player, int lap, int & minutes, int & seconds, int & miliseconds) const
{
	double time = 0;
	if (player == 0)
	{
		if (lap > lap1_times.size() || lap < 0)
			return false;
		time = lap1_times[lap];
		if (lap > 0)
			time -= lap1_times[lap - 1];
	}
	else if (player == 1)
	{
		if (lap > lap2_times.size() || lap < 0)
			return false;
		time = lap2_times[lap];
		if (lap > 0)
			time -= lap2_times[lap - 1];
	}

	seconds = floor(time);
	miliseconds = (time - seconds) * 1000;
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
		miliseconds = (time - seconds) * 1000;
		minutes = floor(seconds / 60);
		seconds = seconds - (minutes * 60);
		return true;
	}
	return false;
}

