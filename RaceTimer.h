#ifndef __RACETIMER_H__
#define __RACETIMER_H__

#include <vector>
#include "PerfTimer.h"

class RaceTimer
{
public:
	RaceTimer();
	~RaceTimer();

	void Start(); //Starts counting the time for the first lap
	void Update(float dt);
	void AddLap(unsigned int player); //Saves the time of the current lap and starts another
	void Reset(); //Resets all variables to initial value. Does NOT start counting again (call Start again)

	double GetRaceTime(int& minutes, int& seconds, int& miliseconds)const;

	int GetCurrentLap(unsigned int player)const;
	void GetCurrentLapTime(unsigned int player, int& minutes, int& seconds, int& miliseconds)const;
	double GetCurrentLapTime(unsigned int player)const;

	bool GetLapTime(unsigned int player, int lap, int& minutes, int&seconds, int& miliseconds)const;
	bool GetAllLapsTime(unsigned int player, int& minutes, int& seconds, int& miliseconds)const;
private:
	int current_lap1 = 1;
	int current_lap2 = 1;

	double race_timer = 0;

	std::vector<double> lap1_times;
	std::vector<double> lap2_times;
};
#endif // !__RACETIMER_H__
