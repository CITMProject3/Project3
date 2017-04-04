#ifndef __ModuleAudio_H__
#define __ModuleAudio_H__

#include "Module.h"
#include "AK/include/Win32/AkFilePackageLowLevelIOBlocking.h" // Sample low-level I/O implementation

#include <string>

struct SoundBank;

struct AudioEvent
{
	std::string name;
	unsigned int id = 0;
	SoundBank *parent_soundbank = nullptr;
};

struct SoundBank
{
	std::string name;
	std::string path;
	unsigned int id = 0;
	std::vector<AudioEvent*> events;
};

class ComponentCamera;

// Wwise docuemntation:
// https://www.audiokinetic.com/library/edge/?source=Help&id=welcome_to_wwise

#define MAX_LISTENERS 8

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

	void SetLibrarySoundbankPath(const char *lib_path);
	const char *GetInitLibrarySoundbankPath() const;
	unsigned int ExtractSoundBankInfo(std::string soundbank_path);
	void ObtainEvents(std::vector<AudioEvent*> &events);

	void InitSoundbankLoaded();
	bool IsInitSoundbankLoaded() const;

	void LoadSoundBank(const char *soundbank_path);
	void UnloadSoundBank(const char *soundbank_path);
	void PostEvent(const AudioEvent *ev, unsigned int id);
	void StopEvent(const AudioEvent *ev, unsigned int id);

	void RegisterGameObject(unsigned int id);
	void UnregisterGameObject(unsigned int id);

	AudioEvent *FindEventById(unsigned event_id);

	// Listeners
	void UpdateListenerPos(ComponentCamera *cam, unsigned int listener_id); //Update pos and orientation
	void SetListeners(unsigned int wwise_go_id) const;
	void AddListener(unsigned char listener_id);
	void RemoveListener(unsigned char listener_id);

private:

	// We're using the default Low-Level I/O implementation that's part
	// of the SDK's sample code, with the file package extension
	CAkFilePackageLowLevelIOBlocking g_lowLevelIO;	

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

	bool IsSoundBank(const std::string &file_to_check) const;

	// Listeners
	unsigned char active_listeners = 0;

};

#endif // __ModuleAudio_H__