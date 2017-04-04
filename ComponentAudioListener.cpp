#include "ComponentAudioListener.h"

#include "Application.h"
#include "ModuleAudio.h"

#include "imgui\imgui.h"

#include "GameObject.h"
#include "ComponentCamera.h"

#include <string>

unsigned int ComponentAudioListener::num_listeners = 0;

ComponentAudioListener::ComponentAudioListener(ComponentType type, GameObject* game_object) : Component(type, game_object)
{ 
	listener_id = num_listeners++;
	App->audio->AddListener(listener_id);
}

ComponentAudioListener::~ComponentAudioListener()
{ 
	App->audio->RemoveListener(listener_id);
	--num_listeners;
}

void ComponentAudioListener::Update()
{
	ComponentCamera *cam = (ComponentCamera*)game_object->GetComponent(ComponentType::C_CAMERA);

	if (cam)
		App->audio->UpdateListenerPos(cam, listener_id);
	else
		LOG("Audio listener requires a Component Camera attached on the same Gameobject");
}

void ComponentAudioListener::OnInspector(bool debug)
{
	std::string str = (std::string("Audio Listener") + std::string("##") + std::to_string(uuid));
	if (ImGui::CollapsingHeader(str.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::IsItemClicked(1))
		{
			ImGui::OpenPopup("delete##audiolistener");
		}

		if (ImGui::BeginPopup("delete##audiolistener"))
		{
			if (ImGui::MenuItem("Delete"))
			{
				Remove();
			}
			ImGui::EndPopup();
		}

		//Active
		bool is_active = IsActive();
		if (ImGui::Checkbox("###activeAudioListener", &is_active))
		{
			SetActive(is_active);

			if (is_active) App->audio->AddListener(listener_id);
			else App->audio->RemoveListener(listener_id);
		}

		ImGui::Text("Listener ID: %u", listener_id);
	}
}

void ComponentAudioListener::Save(Data & file)const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);

	data.AppendUInt("listener ID", listener_id);

	file.AppendArrayValue(data);
}

void ComponentAudioListener::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	// Setting listener configuration for saved listener_id
	App->audio->RemoveListener(listener_id);
	listener_id = conf.GetUInt("listener ID");
	if(active) App->audio->AddListener(listener_id);
}

void ComponentAudioListener::Remove()
{
	game_object->RemoveComponent(this);
}
