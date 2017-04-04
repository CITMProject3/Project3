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
	App->audio->StopEvent(current_event, wwise_id_go);
	App->audio->UnregisterGameObject(wwise_id_go);
	if(current_event != nullptr) App->resource_manager->UnloadResource(current_event->parent_soundbank->path);
	if (attenuation_sphere != nullptr) delete attenuation_sphere;
}

void ComponentAudioSource::Update()
{ 
	math::float3 pos = game_object->transform->GetPosition();
	AkListenerPosition ak_pos;
	ak_pos.Set(pos.x, pos.z, pos.y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	AK::SoundEngine::SetPosition(wwise_id_go, ak_pos);	

	// Scripting needs THIS to properly trigger audio! What the fuck!
	if (play_event)
	{
		PlayEvent();
		play_event = false;
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

		// Event selection
		ImGui::Text("Event: ");
		ImGui::SameLine();		

		std::vector<AudioEvent*> events;
		App->audio->ObtainEvents(events);
		
		if (ImGui::BeginMenu(event_selected.c_str()))
		{
			for (std::vector<AudioEvent*>::iterator it = events.begin(); it != events.end(); ++it)
			{
				if (ImGui::MenuItem((*it)->name.c_str()))
				{
					// Unloading unused Soundbank.
					if (current_event != nullptr) App->resource_manager->UnloadResource(current_event->parent_soundbank->path);
					// Loading new bank: first Init bank if it has been not loaded and then, the other one
					if (!App->audio->IsInitSoundbankLoaded())
						if (App->resource_manager->LoadResource(App->audio->GetInitLibrarySoundbankPath(), ResourceFileType::RES_SOUNDBANK) != nullptr)  // Init SB
							App->audio->InitSoundbankLoaded();
					rc_audio = (ResourceFileAudio*)App->resource_manager->LoadResource((*it)->parent_soundbank->path, ResourceFileType::RES_SOUNDBANK);  // Other one SB

					UpdateEventSelected(*it);
				}				
			}
			ImGui::EndMenu();
		}

		if (ImGui::Button("PLAY"))
			PlayEvent();
		ImGui::SameLine();
		if (ImGui::Button("STOP"))
			StopEvent();

		ImGui::Separator();
		if (current_event)
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
		}
	}

	if (debug && attenuation_sphere)
	{
		UpdateAttenuationSpherePos();
		attenuation_sphere->Render();
	}
}

void ComponentAudioSource::PlayEvent() const
{
	App->audio->PostEvent(current_event, wwise_id_go);
}

void ComponentAudioSource::StopEvent() const
{
	App->audio->StopEvent(current_event, wwise_id_go);
}

void ComponentAudioSource::OnPlay()
{
	//PlayEvent();
}

void ComponentAudioSource::OnStop()
{
	StopEvent();
}

const AudioEvent *ComponentAudioSource::GetEvent() const
{
	return current_event;
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

	// Current event on this component Audio
	if (current_event)
	{
		data.AppendUInt("event_id", current_event->id);
		data.AppendString("event_name", current_event->name.c_str());
		data.AppendBool("3D_sound", current_event->sound_3D);
		data.AppendString("soundbank_lib_path", current_event->parent_soundbank->path.c_str());
	}	

	data.AppendFloat("scale_factor_attenuation", scale_factor_attenuation);

	file.AppendArrayValue(data);
}

void ComponentAudioSource::Load(Data & conf)
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

	// There are some events and the corresponding Soundbanks to load?
	event_id = conf.GetUInt("event_id");

	if (event_id)
	{
		current_event = App->audio->FindEventById(event_id);
		event_selected = current_event->name; // Name to show on Inspector
		if (current_event->sound_3D) CreateAttenuationShpere(current_event);
		rc_audio = (ResourceFileAudio*)App->resource_manager->LoadResource(conf.GetString("soundbank_lib_path"), ResourceFileType::RES_SOUNDBANK);
	}
}

void ComponentAudioSource::Remove()
{
	game_object->RemoveComponent(this);
}

// Attenuation related

void ComponentAudioSource::UpdateEventSelected(const AudioEvent *new_event)
{
	event_selected = new_event->name; // Name to show on Inspector
	current_event = new_event;		  // Variable that handles the new event

	if (new_event->sound_3D)
		CreateAttenuationShpere(new_event);
	else
		DeleteAttenuationShpere();
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
	CreateAttenuationShpere(current_event);
	App->audio->ModifyAttenuationFactor(scale_factor_attenuation, wwise_id_go);
}
