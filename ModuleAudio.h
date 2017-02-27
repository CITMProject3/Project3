#ifndef __ModuleAudio_H__
#define __ModuleAudio_H__

#include "Module.h"
#include "AK/include/Win32/AkFilePackageLowLevelIOBlocking.h" // Sample low-level I/O implementation

// Wwise docuemntation:
// https://www.audiokinetic.com/library/edge/?source=Help&id=welcome_to_wwise

class ModuleAudio : public Module
{
public:

	ModuleAudio(const char* name, bool start_enabled = true);
	~ModuleAudio();

	update_status Update();
	update_status PostUpdate();

	bool Init(Data& config);
	bool Start();
	bool CleanUp();

private:

	// We're using the default Low-Level I/O implementation that's part
	// of the SDK's sample code, with the file package extension
	CAkFilePackageLowLevelIOBlocking g_lowLevelIO;

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