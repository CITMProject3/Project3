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
	void AddLap(); //Saves the time of the current lap and starts another
	void Reset(); //Resets all variables to initial value. Does NOT start counting again (call Start again)

	int GetCurrentLap()const;
	void GetCurrentLapTime(int& minutes, int& seconds, int& miliseconds)const;
	double GetCurrentLapTime()const;

	bool GetLapTime(int lap, int& minutes, int&seconds, int& miliseconds)const;
	bool GetAllLapsTime(int& minutes, int& seconds, int& miliseconds)const;
private:
	int current_lap = 1;

	std::vector<double> lap_times;
	PerfTimer current_lap_time;
};
#endif // !__RACETIMER_H__
