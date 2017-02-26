#ifndef __ModuleAudio_H__
#define __ModuleAudio_H__

#include "Module.h"

// Wwise docuemntation:
// https://www.audiokinetic.com/library/edge/?source=Help&id=welcome_to_wwise

class ModuleAudio : public Module
{
public:

	ModuleAudio(const char* name, bool start_enabled = true);
	~ModuleAudio();

	bool Init(Data& config);
	bool CleanUp();

private:

	// Init methods
	bool InitMemoryManager();
	bool InitStreamingManager();
	bool InitSoundEngine();
	bool InitMusicEngine();
	bool InitCommunicationModule();

	// Termination methods
	bool StopMemoryManager();
	bool StopStreamingManager();
	bool StopSoundEngine();
	bool StopMusicEngine();
	bool StopCommunicationModule();

};

#endif // __ModuleAudio_H__