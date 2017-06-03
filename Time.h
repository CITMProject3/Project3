#ifndef __TIME_H__
#define __TIME_H__

#include <cstdint>

class Time
{
public:
	Time();
	~Time();

	void UpdateFrame(); //Called at the start of every frame

	void Play();
	void Stop();
	void Pause();
	double RealTimeSinceStartup()const; //In seconds
	double TimeSinceGameStartup()const; //In seconds
	unsigned int GetFrameCount()const;
	int GetFPS()const;
	int GetMaxFPS()const;
	float DeltaTime()const;
	float RealDeltaTime()const;
	float GetTimeScale()const;
	void SetTimeScale(float time_scale);
	void SetMaxFPS(int max_fps);
	//Returns time since start from 0 to 1
	float GetUnitaryTime();

private:
	//Real time clock
	uint64_t	started_at = 0;
	static uint64_t frequency;

	//Game time clock
	uint64_t game_started_at = 0;
	bool game_paused = false;
	uint64_t pause_started_at = 0;
	double time_paused = 0;

	//Frame calculation
	uint64_t frame_started_at = 0;

	double second_counter = 0; // 1 second counter to check "real_frame_count" value
	unsigned int real_frame_count = 0; //Counting frames in 1 second, resets each sec
	unsigned int frame_count = 0; //Since the game started
	float delta_time = 0; //Of the game
	float real_delta_time = 0; //Of the Application
	float time_scale = 1.0f; 

	float time_unitary = 0.0f;
	
	double capped_ms = 0;
	int max_fps = 30;
	int last_fps = 60;
};

extern Time* time;
#endif
