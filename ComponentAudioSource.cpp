#include "ComponentAudioSource.h"

#include "ModuleAudio.h"
#include "ModuleResourceManager.h"
#include "Application.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "Random.h"

#include "Primitive.h"

#include "imgui\imgui.h"

ComponentAudioSource::ComponentAudioSource(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
	wwise_id_go = App->rnd->RandomInt();
	App->audio->RegisterGameObject(wwise_id_go);
	App->audio->SetListeners(wwise_id_go);
}

ComponentAudioSource::~ComponentAudioSource()
{ 
	if (attenuation_sphere != nullptr) delete attenuation_sphere;
	StopAllEvents();
	RemoveAllEvents();
	App->audio->UnregisterGameObject(wwise_id_go);
}

void ComponentAudioSource::Update()
{ 
	math::float3 pos = game_object->transform->GetPosition();
	AkListenerPosition ak_pos;
	ak_pos.Set(pos.x, pos.z, pos.y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	AK::SoundEngine::SetPosition(wwise_id_go, ak_pos);	

	// Scripting needs THIS to properly trigger audio! What the fuck!
	if (play_event_pending)
	{
		PlayEvent(play_event_pending_index);
		play_event_pending = false;
	}
}

void ComponentAudioSource::OnInspector(bool debug)
{
	std::string str = (std::string("Audio Source") + std::string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##audio_source");
		}

		if (ImGui::BeginPopup("delete##audio_source"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

		//Active
		bool is_active = IsActive();
		if (ImGui::Checkbox("###activeAudio", &is_active))
		{
			SetActive(is_active);
		}
		
		ShowAddRemoveButtons();
		ShowListOfEvents();		

		ImGui::Separator();
	/*	if (current_event)
		{
			if (current_event->sound_3D)
			{
				ImGui::Text("3D Sound enabled");
				ImGui::Text("Max attenuation distance: %.2f units", current_event->max_attenuation * scale_factor_attenuation);
				if (ImGui::DragFloat("Attenuation factor", &scale_factor_attenuation, 0.1f, 0.10f, 10.0f))
					ModifyAttenuationFactor();					
			} 
			else
				ImGui::Text("3D Sound disabled");			
		}*/
	}

	if (debug && attenuation_sphere)
	{
		UpdateAttenuationSpherePos();
		attenuation_sphere->Render();
	}
}

void ComponentAudioSource::PlayEvent(unsigned index) const
{
	App->audio->PostEvent(list_of_events[index], wwise_id_go);
}

void ComponentAudioSource::StopEvent(unsigned index) const
{
	App->audio->StopEvent(list_of_events[index], wwise_id_go);
}

void ComponentAudioSource::StopAllEvents() const
{
	for (size_t i = 0; i < list_of_events.size(); ++i)
		StopEvent(i);
}

void ComponentAudioSource::PlayAudio(unsigned id_audio)
{
	if (id_audio < list_of_events.size())
	{
		play_event_pending = true;
		play_event_pending_index = id_audio;
	}
}

void ComponentAudioSource::OnPlay()
{
	//PlayEvent();
}

void ComponentAudioSource::OnStop()
{
	StopAllEvents();
}

long unsigned ComponentAudioSource::GetWiseID() const
{
	return wwise_id_go;
}

void ComponentAudioSource::Save(Data & file)const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);

	// Saving Init Soundbank information. Must be loaded first when loading new scenes.
	data.AppendString("init_soundbank_lib_path", App->audio->GetInitLibrarySoundbankPath());

	// Scale factor for attenuation
	data.AppendFloat("scale_factor_attenuation", scale_factor_attenuation);

	// List of events
	data.AppendArray("audio_events");
	for (std::vector<const AudioEvent*>::const_iterator curr_event = list_of_events.begin(); curr_event != list_of_events.end(); ++curr_event)
	{
		Data sound_event;
		if ((*curr_event)) // Maybe the user hasn't selected any audio event...
		{
			sound_event.AppendUInt("event_id", (*curr_event)->id);
			sound_event.AppendString("event_name", (*curr_event)->name.c_str());
			sound_event.AppendBool("sound_3D", (*curr_event)->sound_3D);
			sound_event.AppendString("soundbank_lib_path", (*curr_event)->parent_soundbank->path.c_str());
		}
		else
			sound_event.AppendUInt("event_id", 0);

		data.AppendArrayValue(sound_event);
	}

	file.AppendArrayValue(data);
}

void ComponentAudioSource::Load(Data &conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	// It's mandatory to load Init Soundbank first
	if (!App->audio->IsInitSoundbankLoaded())
	{
		if (App->resource_manager->LoadResource(conf.GetString("init_soundbank_lib_path"), ResourceFileType::RES_SOUNDBANK) != nullptr)
			App->audio->InitSoundbankLoaded();
	}		

	scale_factor_attenuation = conf.GetFloat("scale_factor_attenuation");

	for (size_t i = 0; i < conf.GetArraySize("audio_events"); ++i)
	{
		Data audio_event = conf.GetArray("audio_events", i);

		const AudioEvent *a_event = App->audio->FindEventById(audio_event.GetUInt("event_id"));
		if (a_event)
		{
			App->resource_manager->LoadResource(audio_event.GetString("soundbank_lib_path"), ResourceFileType::RES_SOUNDBANK);
			list_of_events.push_back(a_event);
		}			
		else
			list_of_events.push_back(empty_event);
	}

	// There are some events and the corresponding Soundbanks to load?
	//event_id = conf.GetUInt("event_id");

	//if (event_id)
	//{
	//	current_event = App->audio->FindEventById(event_id);
	//	event_selected = current_event->name; // Name to show on Inspector
	//	if (current_event->sound_3D) CreateAttenuationShpere(current_event);
	//	rc_audio = (ResourceFileAudio*)App->resource_manager->LoadResource(conf.GetString("soundbank_lib_path"), ResourceFileType::RES_SOUNDBANK);
	//}
}

void ComponentAudioSource::Remove()
{
	game_object->RemoveComponent(this);
}

// Attenuation related

void ComponentAudioSource::UpdateEventSelected(unsigned int index, const AudioEvent *new_event)
{
	//event_selected = new_event->name; // Name to show on Inspector

	list_of_events[index] = new_event;

	//current_event = new_event;		  // Variable that handles the new event

	/*if (new_event->sound_3D)
		CreateAttenuationShpere(new_event);
	else
		DeleteAttenuationShpere();*/
}

void ComponentAudioSource::CreateAttenuationShpere(const AudioEvent *event)
{
	DeleteAttenuationShpere();

	attenuation_sphere = new Sphere_P(event->max_attenuation * scale_factor_attenuation);
	UpdateAttenuationSpherePos();
}

void ComponentAudioSource::DeleteAttenuationShpere()
{
	if (attenuation_sphere)
	{
		delete attenuation_sphere;
		attenuation_sphere = nullptr;
	}
}

void ComponentAudioSource::UpdateAttenuationSpherePos()
{
	math::float3 pos = game_object->transform->GetPosition();
	attenuation_sphere->SetPos(pos.x, pos.y, pos.z);
}

void ComponentAudioSource::ModifyAttenuationFactor()
{
	//CreateAttenuationShpere(current_event);
	App->audio->ModifyAttenuationFactor(scale_factor_attenuation, wwise_id_go);
}

void ComponentAudioSource::RemoveAllEvents()
{
	for (std::vector<const AudioEvent*>::iterator it = list_of_events.begin(); it != list_of_events.end(); ++it)
	{
		if((*it) != nullptr)
			App->resource_manager->UnloadResource((*it)->parent_soundbank->path);  // Resource
	}

	list_of_events.clear();
}

// Inspector options
void ComponentAudioSource::ShowAddRemoveButtons()
{
	// Add new audio source
	if (ImGui::Button("+"))
	{
		list_of_events.push_back(empty_event);
	}

	ImGui::SameLine();

	// Remove last audio source added
	if (ImGui::Button("-") && !list_of_events.empty())
	{
		if(list_of_events.back() != nullptr) // Unload Resource
			App->resource_manager->UnloadResource(list_of_events.back()->parent_soundbank->path);
		list_of_events.pop_back(); // Event
	}
}

void ComponentAudioSource::ShowListOfEvents()
{
	std::vector<AudioEvent*> events;
	App->audio->ObtainEvents(events);

	// List of different events
	unsigned index = 0;
	for (std::vector<const AudioEvent*>::iterator curr_event = list_of_events.begin(); curr_event != list_of_events.end(); ++curr_event)
	{
		ImGui::Text("ID %u", index);
		ImGui::Text("Event: "); ImGui::SameLine();

		// Name of the event with ## ID to separate the diferent fields
		std::string event_name = (*curr_event != nullptr ? (*curr_event)->name.c_str() : "");
		event_name += "##" + to_string(index);

		// Showing available event options
		if (ImGui::BeginMenu(event_name.c_str()))
		{
			for (std::vector<AudioEvent*>::iterator it = events.begin(); it != events.end(); ++it)
			{
				if (ImGui::MenuItem((*it)->name.c_str()))
				{
					// Unloading unused Soundbank.
					if ((*curr_event) != nullptr) App->resource_manager->UnloadResource((*curr_event)->parent_soundbank->path);
					// Loading new bank: first Init bank if it has been not loaded and then, the other one
					if (!App->audio->IsInitSoundbankLoaded())
						if (App->resource_manager->LoadResource(App->audio->GetInitLibrarySoundbankPath(), ResourceFileType::RES_SOUNDBANK) != nullptr)  // Init SB
							App->audio->InitSoundbankLoaded();
					App->resource_manager->LoadResource((*it)->parent_soundbank->path, ResourceFileType::RES_SOUNDBANK);  // Other one SB

					UpdateEventSelected(index, *it);
				}
			}
			ImGui::EndMenu();
		}

		ShowPlayStopButtons(index);
		++index;
	}
}

void ComponentAudioSource::ShowPlayStopButtons(unsigned index)
{
	// Play and stop buttons for each event entry on Component
	std::string play_button = "PLAY##" + std::to_string(index);
	std::string stop_button = "STOP##" + std::to_string(index);

	if (ImGui::Button(play_button.c_str()))
		PlayEvent(index);
	ImGui::SameLine();
	if (ImGui::Button(stop_button.c_str()))
		StopEvent(index);
}
