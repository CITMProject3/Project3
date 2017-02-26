#include "Globals.h"
#include "Application.h"
#include "ModuleAudio.h"

#include "Wwise_Library.h"

ModuleAudio::ModuleAudio(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

// Destructor
ModuleAudio::~ModuleAudio()
{}

// Called before render is available
bool ModuleAudio::Init(Data& config)
{
	LOG("Loading Audio Wwise Library");
	
	bool ret = true;

	InitMemoryManager();
	InitStreamingManager();
	InitSoundEngine();
	InitMusicEngine();
	InitCommunicationModule();

	return ret;
}

// Called before quitting
bool ModuleAudio::CleanUp()
{
	LOG("Terminating Audio Wwise Library");

	//Termination will be done in the reverse order compared to initialization.
	StopCommunicationModule();
	StopMusicEngine();
	StopSoundEngine();
	StopStreamingManager();
	StopMemoryManager();

	return true;
}

bool ModuleAudio::InitMemoryManager()
{
	// Create and initialize an instance of the default memory manager. Note
	// that you can override the default memory manager with your own. Refer
	// to the SDK documentation for more information.

	AkMemSettings memSettings;
	memSettings.uMaxNumPools = 20;

	if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
	{
		assert(!"Could not create the memory manager.");
		return false;
	}

	return true;
}

// We're using the default Low-Level I/O implementation that's part
// of the SDK's sample code, with the file package extension
//CAkFilePackageLowLevelIOBlocking g_lowLevelIO;

bool ModuleAudio::InitStreamingManager()
{
	// Create and initialize an instance of the default streaming manager. Note
	// that you can override the default streaming manager with your own. Refer
	// to the SDK documentation for more information.

	AkStreamMgrSettings stmSettings;
	AK::StreamMgr::GetDefaultSettings(stmSettings);

	// Customize the Stream Manager settings here.

	if (!AK::StreamMgr::Create(stmSettings))
	{
		assert(!"Could not create the Streaming Manager");
		return false;
	}

	// Create a streaming device with blocking low-level I/O handshaking.
	// Note that you can override the default low-level I/O module with your own. Refer
	// to the SDK documentation for more information.        
	//
	AkDeviceSettings deviceSettings;
	AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);

	// Customize the streaming device settings here.

	// CAkFilePackageLowLevelIOBlocking::Init() creates a streaming device
	// in the Stream Manager, and registers itself as the File Location Resolver.
	/*if (g_lowLevelIO.Init(deviceSettings) != AK_Success)
	{
		assert(!"Could not create the streaming device and Low-Level I/O system");
		return false;
	}*/

	return true;
}

bool ModuleAudio::InitSoundEngine()
{
	// Create the Sound Engine
	// Using default initialization parameters

	AkInitSettings initSettings;
	AkPlatformInitSettings platformInitSettings;
	AK::SoundEngine::GetDefaultInitSettings(initSettings);
	AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

	if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success)
	{
		assert(!"Could not initialize the Sound Engine.");
		return false;
	}

	return true;
}

bool ModuleAudio::InitMusicEngine()
{
	// Initialize the music engine
	// Using default initialization parameters

	AkMusicSettings musicInit;
	AK::MusicEngine::GetDefaultInitSettings(musicInit);

	if (AK::MusicEngine::Init(&musicInit) != AK_Success)
	{
		assert(!"Could not initialize the Music Engine.");
		return false;
	}

	return true;
}

bool ModuleAudio::InitCommunicationModule()
{
	#ifndef AK_OPTIMIZED
	
	// Initialize communications (not in release build!)
	AkCommSettings commSettings;
	AK::Comm::GetDefaultInitSettings(commSettings);
	if (AK::Comm::Init(commSettings) != AK_Success)
	{
		assert(!"Could not initialize communication.");
		return false;
	}
	#endif // AK_OPTIMIZED

	return true;
}

bool ModuleAudio::StopMemoryManager()
{
	// Terminate the Memory Manager
	AK::MemoryMgr::Term();

	return true;
}

bool ModuleAudio::StopStreamingManager()
{
	// Terminate the streaming device and streaming manager

	// CAkFilePackageLowLevelIOBlocking::Term() destroys its associated streaming device 
	// that lives in the Stream Manager, and unregisters itself as the File Location Resolver.
	//g_lowLevelIO.Term();

	if (AK::IAkStreamMgr::Get())
		AK::IAkStreamMgr::Get()->Destroy();

	return true;
}

bool ModuleAudio::StopSoundEngine()
{
	// Terminate the sound engine
	AK::SoundEngine::Term();

	return true;
}

bool ModuleAudio::StopMusicEngine()
{
	// Terminate the music engine
	AK::MusicEngine::Term();

	return true;
}

bool ModuleAudio::StopCommunicationModule()
{
	#ifndef AK_OPTIMIZED
	// Terminate Communication Services
	AK::Comm::Term();
	#endif

	return true;
}