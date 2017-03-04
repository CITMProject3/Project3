#ifndef __ModuleAudio_H__
#define __ModuleAudio_H__

#include "Module.h"
#include "AK/include/Win32/AkFilePackageLowLevelIOBlocking.h" // Sample low-level I/O implementation

#include <string>

class SoundBank;

struct AudioEvent
{
	std::string name;
	long unsigned int id = 0;
	SoundBank *parent_soundbank = nullptr;
};

struct SoundBank
{
	std::string name;
	std::string path;
	long unsigned int id = 0;
	std::vector<AudioEvent*> events;
};

class ComponentCamera;

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

	void SetListener(const ComponentCamera *listener);
	void SetLibrarySoundbankPath(const char *lib_path);
	long unsigned int ExtractSoundBankInfo(std::string soundbank_path);
	void ObtainEvents(std::vector<AudioEvent*> &events);

	void LoadSoundBank(const char *soundbank_path);
	void UnloadSoundBank(const char *soundbank_path);
	void PostEvent(const AudioEvent *ev, long unsigned int id);

	void RegisterGameObject(long unsigned int id);
	void UnregisterGameObject(long unsigned int id);

	AudioEvent *FindEventById(long unsigned event_id);


private:

	// We're using the default Low-Level I/O implementation that's part
	// of the SDK's sample code, with the file package extension
	CAkFilePackageLowLevelIOBlocking g_lowLevelIO;	
	
	const ComponentCamera *listener = nullptr;	// Component camera that incorporates the audio listener

	// Soundbank related variables
	std::string lib_base_path;
	SoundBank *init_sb = nullptr;
	bool init_sb_loaded = false;
	std::vector<SoundBank*> soundbank_list; // List of soundbanks

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

	// Update position and orientation of listener
	void UpdateListenerPos();

	bool IsSoundBank(const std::string &file_to_check) const;

};

#endif // __ModuleAudio_H__