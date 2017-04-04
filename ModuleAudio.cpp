#include "ModuleAudio.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleGOManager.h"

#include "ComponentAudioSource.h"
#include "ComponentCamera.h"

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

bool ModuleAudio::Start()
{
	// Looking for library directory for Soundbanks
	char *buf;
	if(App->file_system->Load("Assets/Soundbanks.meta", &buf) > 0)
	{
		Data sb(buf);
		lib_base_path = sb.GetString("library_path");
		delete buf;
	}

	// Saving Soundbank related information: events, switches, states, RTCP...
	if (!lib_base_path.empty())
	{
		std::vector<std::string> folders, files;
		std::string audio_folder = lib_base_path;
		App->file_system->GetFilesAndDirectories(audio_folder.c_str(), folders, files);

		for (size_t i = 0; i < folders.size(); ++i)
		{
			files.clear();
			string sub_folder = audio_folder + folders[i];
			App->file_system->GetFilesAndDirectories(sub_folder.c_str(), folders, files);

			for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
			{
				if (IsSoundBank((*it)))
					ExtractSoundBankInfo(sub_folder + '/' + (*it));
			}
		}			
	}

	return true;
}

// Called before quitting
bool ModuleAudio::CleanUp()
{
	LOG("Terminating Audio Wwise Library");

	// Deleting soundbank information
	for (std::vector<SoundBank*>::iterator it_sb = soundbank_list.begin(); it_sb != soundbank_list.end(); ++it_sb)
	{
		// Events
		for (std::vector<AudioEvent*>::iterator it_ev = (*it_sb)->events.begin(); it_ev != (*it_sb)->events.end(); ++it_ev)
			delete (*it_ev);
		delete (*it_sb);
	}

	//Termination will be done in the reverse order compared to initialization.
	StopCommunicationModule();
	StopMusicEngine();
	StopSoundEngine();
	StopStreamingManager();
	StopMemoryManager();

	return true;
}

update_status ModuleAudio::Update()
{
	return UPDATE_CONTINUE;
}

update_status ModuleAudio::PostUpdate()
{
	// Process bank requests, events, positions, RTPC, etc.
	AK::SoundEngine::RenderAudio();

	return UPDATE_CONTINUE;
}

void ModuleAudio::UpdateListenerPos(ComponentCamera *cam, unsigned int listener_id)
{
	math::float3 front = cam->GetFront();   // Orientation of the listener
	math::float3 up = cam->GetUp();			// Top orientation of the listener
	math::float3 pos = cam->GetPos();	    // Position of the listener

	AkListenerPosition ak_pos;
	ak_pos.Set(pos.x, pos.z, pos.y, front.x, front.z, front.y, up.x, up.z, up.y);

	AK::SoundEngine::SetListenerPosition(ak_pos, listener_id);
}

void ModuleAudio::SetListeners(unsigned int wwise_go_id) const
{
	AK::SoundEngine::SetActiveListeners(wwise_go_id, active_listeners);
}

void ModuleAudio::AddListener(unsigned char listener_id)
{
	assert(listener_id < MAX_LISTENERS);

	//if (active_listeners == 0)
	//	active_listeners = 1;		// If there is no listener yet...

	unsigned char new_listener = (1 << listener_id);	 // Obtaining bit position for new listener. I.e. Listener_id --> 3 , new_listener = 00001000
	active_listeners |= new_listener;					 // One audio listener more to the current active listeners!

	// Adapting all sources to the new listener configuration
	std::vector<Component*> audio_sources;
	App->go_manager->GetAllComponents(audio_sources, ComponentType::C_AUDIO_SOURCE);

	for (size_t i = 0; i < audio_sources.size(); ++i)
	{
		ComponentAudioSource *audio = (ComponentAudioSource*)audio_sources[i];
		SetListeners(audio->GetWiseID());
	}
}

void ModuleAudio::RemoveListener(unsigned char listener_id)
{
	unsigned char listener_to_remove = ~(1 << listener_id);		// Obtaining bit position for listener to remove. I.e I.e. Listener_id --> 3 , new_listener = 11110111
	active_listeners &= listener_to_remove;						// Deleting listener from the current active listeners!

	// Adapting all sources to the new listener configuration
	std::vector<Component*> audio_sources;
	App->go_manager->GetAllComponents(audio_sources, ComponentType::C_AUDIO_SOURCE);

	for (size_t i = 0; i < audio_sources.size(); ++i)
	{
		ComponentAudioSource *audio = (ComponentAudioSource*)audio_sources[i];
		SetListeners(audio->GetWiseID());
	}
}

unsigned int ModuleAudio::ExtractSoundBankInfo(std::string soundbank_path)
{
	char *buf;
	AkUInt32 ret = 0;

	std::string json_file = soundbank_path.substr(0, soundbank_path.find_last_of('.')) + ".json"; // Changing .bnk with .json
	if (App->file_system->Load(json_file.c_str(), &buf))
	{
		Data general_info(buf);
		Data sb_info = general_info.GetJObject("SoundBanksInfo").GetArray("SoundBanks", 0);
		
		// Soundbank Information
		SoundBank *soundbank = new SoundBank();
		soundbank_list.push_back(soundbank);

		ret = soundbank->id = std::stoul(sb_info.GetString("Id"));
		soundbank->name = sb_info.GetString("ShortName");
		soundbank->path = soundbank_path;

		// Is Init Soundbank?
		if (soundbank->name.compare("Init") == 0)
			init_sb = soundbank;		

		// Events Information
		uint num_events = sb_info.GetArraySize("IncludedEvents");
		for (uint i = 0; i < num_events; ++i)
		{
			Data events_info = sb_info.GetArray("IncludedEvents", i);

			AudioEvent *a_event = new AudioEvent();
			soundbank->events.push_back(a_event);
			a_event->parent_soundbank = soundbank;

			a_event->id = std::stoul(events_info.GetString("Id"));
			a_event->name = events_info.GetString("Name");
		}
		
		delete buf; // Freeing memory
	}

	return ret;
}

void ModuleAudio::ObtainEvents(std::vector<AudioEvent*> &events)
{
	// Deleting soundbank information
	for (std::vector<SoundBank*>::iterator it_sb = soundbank_list.begin(); it_sb != soundbank_list.end(); ++it_sb)
	{
		for (std::vector<AudioEvent*>::iterator it_ev = (*it_sb)->events.begin(); it_ev != (*it_sb)->events.end(); ++it_ev)
			events.push_back(*it_ev);		
	}		
}

bool ModuleAudio::IsSoundBank(const std::string &file_to_check) const
{
	size_t extension = file_to_check.find_last_of('.');
	if (extension != std::string::npos && file_to_check.substr(extension + 1).compare("bnk") == 0)
		return true;
	return false;
}

void ModuleAudio::LoadSoundBank(const char *soundbank_path)
{
	AkBankID returned_bankID;

	// Load the corresponding soundbank
	AKRESULT eResult = AK::SoundEngine::LoadBank(soundbank_path, AK_DEFAULT_POOL_ID, returned_bankID);
	assert(eResult == AK_Success);
}

void ModuleAudio::UnloadSoundBank(const char *soundbank_path)
{
	// Unload the corresponding soundbank
	AkBankID returned_bankID = 0;
	AKRESULT eResult = AK::SoundEngine::UnloadBank(soundbank_path, NULL);
	assert(eResult == AK_Success);
}

void ModuleAudio::PostEvent(const AudioEvent *ev, unsigned int id)
{
	if(ev) AK::SoundEngine::PostEvent(ev->name.c_str(), id);
}

void ModuleAudio::StopEvent(const AudioEvent *ev, unsigned int id)
{
	if (ev) AK::SoundEngine::ExecuteActionOnEvent(ev->name.c_str(), AK::SoundEngine::AkActionOnEventType_Stop, id);
}

void ModuleAudio::RegisterGameObject(unsigned int id)
{
	AK::SoundEngine::RegisterGameObj(id);
}

void ModuleAudio::UnregisterGameObject(unsigned int id)
{
	AK::SoundEngine::UnregisterGameObj(id);
}

void ModuleAudio::SetLibrarySoundbankPath(const char *lib_path)
{
	lib_base_path = lib_path;
}

const char *ModuleAudio::GetInitLibrarySoundbankPath() const
{
	if (init_sb)
		return init_sb->path.c_str();
	return "";
}

void ModuleAudio::InitSoundbankLoaded()
{
	init_sb_loaded = true;
}

bool ModuleAudio::IsInitSoundbankLoaded() const
{
	return init_sb_loaded;
}

AudioEvent *ModuleAudio::FindEventById(unsigned event_id)
{
	// Deleting soundbank information
	for (std::vector<SoundBank*>::const_iterator it_sb = soundbank_list.begin(); it_sb != soundbank_list.end(); ++it_sb)
	{
		for (std::vector<AudioEvent*>::iterator it_ev = (*it_sb)->events.begin(); it_ev != (*it_sb)->events.end(); ++it_ev)
			if ((*it_ev)->id == event_id)
				return *it_ev;
	}

	return nullptr;
}

// --- Initialization methods ---

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
	if (g_lowLevelIO.Init(deviceSettings) != AK_Success)
	{
		assert(!"Could not create the streaming device and Low-Level I/O system");
		return false;
	}

	// Setup banks path
	g_lowLevelIO.SetBasePath(AKTEXT("."));
	AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)"));

	return true;
}

bool ModuleAudio::InitSoundEngine()
{
	// Create the Sound Engine
	// Using default initialization parameters

	// CRZ: These are default options. In order to adjust memory usage:
	// https://www.audiokinetic.com/library/edge/?source=SDK&id=goingfurther__optimizingmempools.html#soundengine_initialization_advanced_soundengine_pool_size

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