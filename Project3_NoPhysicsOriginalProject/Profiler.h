#ifndef __PROFILER_H__
#define __PROFILER_H__

#include <map>
#include <vector>
#include <deque>
#include "Window.h"

using namespace std;

#define MAX_TIME_ITEMS 600

struct ProfilerSample
{
	deque<double> total_ms;
	deque<unsigned int> calls;
	unsigned int id; //Position to insert new sample
};

enum ProfilerSortType
{
	NAME,
	CALLS,
	TOTAL_MS,
	SELF_MS
};

class Profiler : public Window
{
public:
	Profiler();
	~Profiler();

	void Update();
	void Draw();

	void StoreSample(const char* name, double time);

private:

	//Fills an array of floats with the total_ms of the sample.
	void SampleToArray(const ProfilerSample &p_sample, vector<float>& arr)const;

	void DisplayTableColumns(const char* name, ProfilerSortType type);

private:
	map<const char*, ProfilerSample> samples;

	//Window
	int current_frame = 0;
	bool is_playing = false;
	ProfilerSample* sample_selected = nullptr;
	ProfilerSortType sort_type = NAME;
};

extern Profiler g_Profiler;

#endif
