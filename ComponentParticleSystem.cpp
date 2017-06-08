#include "ComponentParticleSystem.h"

#include "imgui\imgui.h"
#include "Data.h"

#include "Glew\include\glew.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#include "ResourceFileTexture.h"

#include "Application.h"
#include "Assets.h"
#include "GameObject.h"
#include "Time.h"
#include "OpenGLFunc.h"
#include "Random.h"
#include "DebugDraw.h"

#include "ModuleEditor.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"

#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"

#include "Brofiler\include\Brofiler.h"

#include <string>
#include <algorithm>
using namespace std;

ComponentParticleSystem::ComponentParticleSystem(ComponentType type, GameObject* game_object) : Component(type, game_object), 
color(1), cti_entry(1, 0, float3(1)), tex_anim_data(1), bounding_box(vec(-0.5f), vec(0.5f)), bb_size(1.0f), bb_pos_offset(0.0f), state(PSState::PS_STOP),
img_size(1.0f)
{
	BROFILER_CATEGORY("ComponentParticleSystem::Init", Profiler::Color::Navy);

	particles_container.resize(top_max_particles);
	alive_particles_position.resize(top_max_particles);
	alive_particles_color.resize(top_max_particles);
	alive_particles_life.resize(top_max_particles);

	glGenBuffers(1, &position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glBufferData(GL_ARRAY_BUFFER, top_max_particles * 3 * sizeof(float), NULL, GL_STREAM_DRAW);

	glGenBuffers(1, &color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glBufferData(GL_ARRAY_BUFFER, top_max_particles * 4 * sizeof(float), NULL, GL_STREAM_DRAW);

	glGenBuffers(1, &life_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, life_buffer);
	glBufferData(GL_ARRAY_BUFFER, top_max_particles * sizeof(float), NULL, GL_STREAM_DRAW);

	float3 system_position = game_object->GetGlobalMatrix().TranslatePart();
	
	box_shape_obb.pos = system_position;
	box_shape_obb.r = box_shape * 0.5;
	Quat rotation = game_object->GetGlobalMatrix().RotatePart().ToQuat();
	box_shape_obb.axis[0] = rotation * float3::unitX;
	box_shape_obb.axis[1] = rotation * float3::unitY;
	box_shape_obb.axis[2] = rotation * float3::unitZ;

	sphere_shape.pos = system_position;
	sphere_shape.r = 1.0f;

	rnd = LCG(time->RealTimeSinceStartup());

	ColorTimeItem* c_begin = new ColorTimeItem(1.0f, 0.0, color);
	ColorTimeItem* c_end = new ColorTimeItem(1.0, 100.0, color);
	color_time.push_back(c_begin);
	color_time.push_back(c_end);
}

ComponentParticleSystem::~ComponentParticleSystem()
{
	glDeleteBuffers(1, &position_buffer);
	glDeleteBuffers(1, &color_buffer);
	glDeleteBuffers(1, &life_buffer);

	for (vector<ColorTimeItem*>::iterator it = color_time.begin(); it != color_time.end(); ++it)
		delete *it;
	color_time.clear();
}

void ComponentParticleSystem::OnInspector(bool debug)
{
	if (ImGui::CollapsingHeader("ParticleSystem", ImGuiTreeNodeFlags_DefaultOpen))
	{
		InspectorDelete();

		//Active
		bool is_active = IsActive();
		if (ImGui::Checkbox("###active_particle", &is_active))
		{
			SetActive(is_active);
		}

		//Main Options
		ImGui::Text("Duration: "); ImGui::SameLine(); ImGui::DragFloat("###ps_duration", &duration, 0.1f);
		ImGui::Text("Looping: "); ImGui::SameLine(); ImGui::Checkbox("###ps_looping", &looping);
		ImGui::Text("Lifetime: "); ImGui::SameLine(); ImGui::DragFloat("###ps_lifetime", &life_time, 1.0f, 0.0f, 1000.0f);
		ImGui::Text("Speed: "); ImGui::SameLine(); ImGui::DragFloat("###ps_speed", &speed, 1.0f, 0.0, 1000.0f);
		ImGui::Text("Size: "); ImGui::SameLine(); ImGui::DragFloat("###ps_size", &size, 1.0f, 0.0, 1000.0f);
		if (ImGui::CollapsingHeader("Start color: ")) { ImGui::ColorPicker("###ps_start_color", color.ptr()); }
		InspectorSimulationSpace();
		ImGui::Text("Max particles: "); ImGui::SameLine(); ImGui::DragInt("###max_particles", &max_particles, 1, 0, 1000);
		ImGui::Text("Play On Awake: "); ImGui::SameLine(); ImGui::Checkbox("###ps_play_awake", &play_on_awake);

		InspectorEmission();
		InspectorShape();
		InspectorColorOverTime();
		InspectorTextureAnimation();

		//Render
		if (ImGui::CollapsingHeader("Render ###ps_render"))
		{
			if (texture)
				ImGui::Image((ImTextureID)texture->GetTexture(), ImVec2(50, 50));

			ImGui::Text("Change sprite: ");
			if (ImGui::BeginMenu("###ps_change_tex"))
			{
				InspectorChangeTexture();
				ImGui::EndMenu();
			}
		}

		InspectorBoundingBox();

		InspectorSimulation();
	}
}

void ComponentParticleSystem::Save(Data & file) const
{
	Data data;
	data.AppendInt("type", type);
	data.AppendUInt("UUID", uuid);
	data.AppendBool("active", active);

	data.AppendFloat("life_time", life_time);
	data.AppendInt("max_particles", max_particles);
	data.AppendFloat("emission_rate", emission_rate);
	data.AppendFloat("speed", speed);
	data.AppendFloat("size", size);
	data.AppendBool("play_on_awake", play_on_awake);
	data.AppendFloat3("color", color.ptr());
	data.AppendFloat("duration", duration);
	data.AppendBool("looping", looping);
	data.AppendBool("simulation_space", simulation_space_local);

	data.AppendBool("color_over_time", color_over_time_active);
	if (color_over_time_active)
	{
		data.AppendArray("colors");
		
		for (int c = 0; c < color_time.size(); ++c)
		{
			Data colors;
			colors.AppendFloat3("color", color_time[c]->color.ptr());
			colors.AppendFloat("alpha", color_time[c]->alpha);
			colors.AppendFloat("position", color_time[c]->position);
			data.AppendArrayValue(colors);
		}
	}

	data.AppendInt("shape_type", shape_type);
	switch (shape_type)
	{
	case SHAPE_BOX:
		data.AppendFloat3("box_shape", box_shape.ptr());
		break;
	case SHAPE_SPHERE:
		data.AppendFloat("sphere_radius", sphere_shape.r);
		data.AppendBool("sphere_emit_from_shell", sphere_emit_from_shell);
		break;
	}
	

	data.AppendBool("texture_anim", texture_anim);
	if (texture_anim)
		data.AppendFloat3("tex_anim_data", tex_anim_data.ptr());

	//Render
	if (texture)
		data.AppendString("texture", texture->GetFile());
	else
		data.AppendString("texture", "");

	//Bounding box
	data.AppendFloat3("bb_pos_offset", bb_pos_offset.ptr());
	data.AppendFloat3("bb_size", bb_size.ptr());

	//Bursts
	data.AppendArray("bursts");
	if (bursts.size() > 0)
	{
		for (int i = 0; i < bursts.size(); ++i)
		{
			Data burst;
			burst.AppendFloat("time", bursts[i].time);
			burst.AppendInt("min_particles", bursts[i].min_particles);
			burst.AppendInt("max_particles", bursts[i].max_particles);
			data.AppendArrayValue(burst);
		}
	}

	file.AppendArrayValue(data);
}

void ComponentParticleSystem::Load(Data & conf)
{
	uuid = conf.GetUInt("UUID");
	active = conf.GetBool("active");

	life_time = conf.GetFloat("life_time");
	max_particles = conf.GetInt("max_particles");
	emission_rate = conf.GetFloat("emission_rate");
	spawn_time = 1.0f / emission_rate;
	speed = conf.GetFloat("speed");
	size = conf.GetFloat("size");
	play_on_awake = conf.GetBool("play_on_awake");
	color = conf.GetFloat3("color");
	duration = conf.GetFloat("duration");
	looping = conf.GetBool("looping");
	simulation_space_local = conf.GetBool("simulation_space");

	color_over_time_active = conf.GetBool("color_over_time");
	if (color_over_time_active)
	{
		int c_size = conf.GetArraySize("colors");

		delete color_time[0];
		delete color_time[1];
		color_time.clear();

		Data color_item;
		for (int c = 0; c < c_size; ++c)
		{
			color_item = conf.GetArray("colors", c);
			ColorTimeItem* cti = new ColorTimeItem(color_item.GetFloat("alpha"), color_item.GetFloat("position"), color_item.GetFloat3("color"));
			color_time.push_back(cti);
		}
	}

	shape_type = (ParticleShapeType)conf.GetInt("shape_type");
	switch (shape_type)
	{
	case SHAPE_BOX:
		box_shape = conf.GetFloat3("box_shape");
		box_shape_obb.r = box_shape * 0.5f;
		break;
	case SHAPE_SPHERE:
		sphere_shape.r = conf.GetFloat("sphere_radius");
		sphere_emit_from_shell = conf.GetBool("sphere_emit_from_shell");
		break;
	}

	bb_pos_offset = conf.GetFloat3("bb_pos_offset");
	bounding_box.Translate((game_object->transform->GetGlobalMatrix().TranslatePart() + bb_pos_offset) - bounding_box.CenterPoint());
	bb_size = conf.GetFloat3("bb_size");
	float3 center = bounding_box.CenterPoint();
	float3 half = bb_size * 0.5f;
	bounding_box.minPoint = center - half;
	bounding_box.maxPoint = center + half;

	if (conf.GetArraySize("bursts") > 0)
	{
		Data burst;
		for (int i = 0; i < conf.GetArraySize("bursts"); ++i)
		{
			burst = conf.GetArray("bursts", i);
			Burst b;
			b.time = burst.GetFloat("time");
			b.min_particles = burst.GetInt("min_particles");
			b.max_particles = burst.GetInt("max_particles");

			bursts.push_back(b);
		}
	}

	texture_anim = conf.GetBool("texture_anim");
	if (texture_anim)
		tex_anim_data = conf.GetFloat3("tex_anim_data");

	string tex_path = conf.GetString("texture");
	if (tex_path.size() > 0)
	{
		ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(tex_path, ResourceFileType::RES_TEXTURE);
		if (rc_tmp)
		{
			texture = rc_tmp;
			img_size.x = rc_tmp->GetWidth() / 100.0f;
			img_size.y = rc_tmp->GetHeight() / 100.0f;
		}
		else
		{
			LOG("[ERROR] Loading failure on particle system %s %s", game_object->name.data(), tex_path.data());
			App->editor->DisplayWarning(WarningType::W_ERROR, "Loading failure on particle system %s %s", game_object->name.data(), tex_path.data());
		}
	}
}

void ComponentParticleSystem::Update()
{
	BROFILER_CATEGORY("ComponentParticleSystem::Update", Profiler::Color::Navy);
	if (active == false)
		return;

	if (state == PS_PLAYING)
	{
		float dt = time->RealDeltaTime();
		system_life += dt;

		if (!looping && system_life >= duration)
			return;
		else
		{
			if (duration * cycles < system_life)
			{
				++cycles;
				for (int b = 0; b < bursts.size(); ++b)
					bursts[b].completed = false;
			}
		}

		spawn_timer += dt;

		if (spawn_timer >= spawn_time)
		{
			int num_particles_to_spawn = spawn_timer / spawn_time;

			for (int i = 0; i < num_particles_to_spawn; i++)
				SpawnParticle(i);

			spawn_timer -= spawn_time * num_particles_to_spawn;
		}

		for (int b = 0; b < bursts.size(); ++b)
		{
			if (!bursts[b].completed && system_life >= duration * (cycles -1) + bursts[b].time)
			{
				bursts[b].completed = true;
				int spawn_rate_rnd = App->rnd->RandomInt(bursts[b].min_particles, bursts[b].max_particles);
				for (int p = 0; p < spawn_rate_rnd; ++p)
					SpawnParticle(0);
			}
		}
	}

}

void ComponentParticleSystem::PostUpdate()
{
	BROFILER_CATEGORY("ComponentParticleSystem::PostUpdate", Profiler::Color::Navy);

	if (state == PS_PLAYING || state == PS_PAUSE)
	{
		float dt = time->RealDeltaTime();

		if (state == PS_PLAYING)
		{
			num_alive_particles = 0;

			Quat rotation = game_object->GetGlobalMatrix().RotatePart().ToQuat();
			float3 sys_position = game_object->GetGlobalMatrix().TranslatePart();

			float item_life;
			float item_life_pc;
			float c_pc;
			ColorTimeItem* previous = nullptr;
			ColorTimeItem* next = nullptr;
			//Update positions & color
			for (int i = 0; i < top_max_particles; ++i)
			{
				Particle& p = particles_container[i];

				if (p.life > 0.0f)
				{
					p.life -= dt;

					if (p.life > 0.0f)
					{
						item_life = life_time - p.life;

						if(simulation_space_local)
							p.position = sys_position + p.dst_origin + (rotation * p.speed) * item_life;
						else
							p.position = p.origin + (rotation * p.speed) * item_life;

						if (color_over_time_active)
						{
							item_life_pc = (item_life / life_time) * 100.0;

							if (color_time[p.next_c_id]->position <= item_life_pc)
								p.next_c_id++;

							int ct_id = (p.next_c_id - 1 >= 0) ? p.next_c_id - 1 : 0;
							previous = color_time[ct_id];
							next = color_time[p.next_c_id];

							c_pc = (item_life_pc - previous->position) / (next->position - previous->position);
							p.color.x = previous->color.x * (1.0f - c_pc) + next->color.x * c_pc;
							p.color.y = previous->color.y * (1.0f - c_pc) + next->color.y * c_pc;
							p.color.z = previous->color.z * (1.0f - c_pc) + next->color.z * c_pc;
							p.color.w = previous->alpha * (1.0f - c_pc) + next->alpha * c_pc;
						}	

						++num_alive_particles;
					}
					else
					{
						p.cam_distance = -1.0f;
						p.life = -1.0f;
					}
				}
				else
				{
					p.cam_distance = -1.0f;
					p.life = -1.0f;
				}

			}
			
		}	
		App->renderer3D->AddToDrawParticle(this);
	}
	
}

void ComponentParticleSystem::OnTransformModified()
{
	//TODO:Optimize this only for the current shape
	box_shape_obb.pos = game_object->GetGlobalMatrix().TranslatePart();
	Quat rotation = game_object->GetGlobalMatrix().RotatePart().ToQuat();
	box_shape_obb.axis[0] = rotation * float3::unitX;
	box_shape_obb.axis[1] = rotation * float3::unitY;
	box_shape_obb.axis[2] = rotation * float3::unitZ;

	sphere_shape.pos = game_object->GetGlobalMatrix().TranslatePart();


	bounding_box.Translate((game_object->transform->GetGlobalMatrix().TranslatePart() + bb_pos_offset) - bounding_box.CenterPoint());
}

void ComponentParticleSystem::OnPlay()
{
	if (play_on_awake)
	{
		state = PSState::PS_PLAYING;
		spawn_timer = spawn_time;
	}
}

void ComponentParticleSystem::OnPause()
{
	state = PSState::PS_PAUSE;
}

void ComponentParticleSystem::OnStop()
{
	StopAll();
}

unsigned int ComponentParticleSystem::GetTextureId() const
{
	return (texture) ? texture->GetTexture() : 0;
}

void ComponentParticleSystem::SortParticles(ComponentCamera * cam)
{
	BROFILER_CATEGORY("ComponentParticleSystem::SortParticles", Profiler::Color::Navy);
	float3 cam_pos;
	if (cam != App->camera->GetEditorCamera())
		cam_pos = cam->GetGameObject()->GetGlobalMatrix().TranslatePart();
	else
		cam_pos = cam->GetPos();

	for (int i = 0; i < max_particles; ++i)
	{
		if (particles_container[i].life > 0.0f)
			particles_container[i].cam_distance = (cam_pos - particles_container[i].position).Length();
	}

	std::sort(particles_container.begin(), particles_container.end());

	for (int i = 0; i < num_alive_particles; ++i)
	{
		alive_particles_position[i] = particles_container[i].position;
		alive_particles_color[i] = particles_container[i].color;
		alive_particles_life[i] = particles_container[i].life;
	}

	//Testing sort
	float previous_cam_dist = 200000.0f;
	for (int i = 0; i < num_alive_particles; ++i)
	{
		if (alive_particles_position[i].x != particles_container[i].position.x && alive_particles_position[i].y != particles_container[i].position.y && alive_particles_position[i].z != particles_container[i].position.z)
		{
			LOG("Particle Error: alive particles position not equal to container position");
		}

		if (particles_container[i].life < 0)
		{
			LOG("Particle Error: dead particle in alive particle buffer");
		}

		if (particles_container[i].cam_distance > previous_cam_dist)
		{
			LOG("Particle Error: cam distance wrong");
		}
		previous_cam_dist = particles_container[i].cam_distance;
	}

	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glBufferData(GL_ARRAY_BUFFER, top_max_particles * 3 * sizeof(float), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, num_alive_particles * sizeof(float) * 3, alive_particles_position.data());

	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glBufferData(GL_ARRAY_BUFFER, top_max_particles * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, num_alive_particles * sizeof(float) * 4, alive_particles_color.data());

	glBindBuffer(GL_ARRAY_BUFFER, life_buffer);
	glBufferData(GL_ARRAY_BUFFER, top_max_particles * sizeof(float), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, num_alive_particles * sizeof(float), alive_particles_life.data());
}

void ComponentParticleSystem::StopAll()
{
	for (int i = 0; i < top_max_particles; ++i)
		particles_container[i].life = -1.0f;

	num_alive_particles = 0;
	cycles = 0;
	state = PSState::PS_STOP;
}

void ComponentParticleSystem::Play()
{
	state = PSState::PS_PLAYING;
	system_life = 0.0f;
	cycles = 0;
}

void ComponentParticleSystem::Pause()
{
	state = PSState::PS_PAUSE;
}

bool ComponentParticleSystem::operator<(ComponentParticleSystem & b)
{
	return this->cam_distance > b.cam_distance;;
}

void ComponentParticleSystem::InspectorDelete()
{
	if (ImGui::IsItemClicked(1))
		ImGui::OpenPopup("delete##ps_delete");
	if (ImGui::BeginPopup("delete##ps_delete"))
	{
		if (ImGui::MenuItem("Delete"))
			Remove();
		ImGui::EndPopup();
	}
}

void ComponentParticleSystem::InspectorChangeTexture()
{
	vector<string> textures;
	App->editor->assets->GetAllFilesByType(FileType::IMAGE, textures);

	for (vector<string>::iterator it = textures.begin(); it != textures.end(); ++it)
	{
		if (ImGui::MenuItem((*it).data()))
		{
			string u_sampler2d = App->resource_manager->FindFile(*it);
			ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(u_sampler2d, ResourceFileType::RES_TEXTURE);
			if (rc_tmp)
			{
				if (texture)
					texture->Unload();

				texture = rc_tmp;
				img_size.x = rc_tmp->GetWidth() / 100.0f;
				img_size.y = rc_tmp->GetHeight() / 100.0f;
			}
			else
			{
				LOG("[ERROR] Loading failure on particle system %s %s", game_object->name.data(), (*it).data());
				App->editor->DisplayWarning(WarningType::W_ERROR, "Loading failure on particle system %s %s", game_object->name.data(), (*it).data());
			}
		}
	}
}

void ComponentParticleSystem::InspectorSimulation()
{
	ImGui::SetNextWindowPos(ImVec2(App->window->GetScreenWidth() * 3 / 5, 600));
	bool open = true;
	ImGui::Begin("##ps_simulation", &open, ImVec2(0, 0), 0.6f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	if (ImGui::Button("Play##ps_stop"))
	{
		state = PS_PLAYING;
	}
	ImGui::SameLine();
	if (ImGui::Button("Pause##ps_pause"))
	{
		state = PS_PAUSE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop##ps_stop"))
	{
		state = PS_STOP;
		system_life = 0.0f;
		StopAll();
	}
	ImGui::Text("Playback Time: %.2f", system_life);
	ImGui::Text("Cycles: %i", cycles);
	ImGui::End();
}

void ComponentParticleSystem::InspectorColorOverTime()
{
	if (ImGui::CollapsingHeader("Color over time"))
	{
		ImGui::Text("Active:"); ImGui::SameLine(); ImGui::Checkbox("###ps_cot_active_cb", &color_over_time_active);
		if (color_over_time_active)
		{
			ImGui::Text("WARNING: do not remove while the system is playing");
			vector<int> items_to_remove;
			for (int c = 0; c < color_time.size(); ++c)
			{
				ImGui::Text("%.2f%%", color_time[c]->position);
				ImGui::SameLine();
				ImGui::ColorButton(ImVec4(color_time[c]->color.x, color_time[c]->color.y, color_time[c]->color.z, color_time[c]->alpha));
				ImGui::SameLine();
				ImGui::PushID(c);
				if (ImGui::Button("Remove"))
				{
					items_to_remove.push_back(c);
				}
				ImGui::PopID();
			}
			ImGui::DragFloat("Position:##ps_cti_entry_pos", &cti_entry.position, 0.01, 0, 100.0);
			ImGui::ColorPicker("Color: ###ps_cti_entry_color", cti_entry.color.ptr());
			ImGui::DragFloat("Alpha:##ps_cti_entry_alpha", &cti_entry.alpha, 0.01, 0, 1.0);
			
			if (ImGui::Button("Add entry ###ps_cot_entry"))
			{
				for (vector<ColorTimeItem*>::iterator it = color_time.begin(); it != color_time.end(); ++it)
				{
					if (cti_entry.position < (*it)->position)
					{
						ColorTimeItem* item = new ColorTimeItem(cti_entry);
						color_time.insert(it, item);
						break;
					}
				}

				if (color_time.size() < 2)
				{
					ColorTimeItem* item = new ColorTimeItem(cti_entry);
					color_time.push_back(item);
				}

				cti_entry.alpha = 1.0f;
				cti_entry.color = float3(1);
				cti_entry.position = 0.0f;
			}

			if (items_to_remove.size() > 0)
				for (int i = 0; i < items_to_remove.size(); ++i)
					color_time.erase(color_time.begin() + items_to_remove[i]);
		}
	}
}

void ComponentParticleSystem::InspectorTextureAnimation()
{
	if (ImGui::CollapsingHeader("Texture Animation"))
	{
		ImGui::Text("Active:"); ImGui::SameLine(); ImGui::Checkbox("###ps_tex_anim_active_cb", &texture_anim);
		if (texture_anim)
		{
			int rows = tex_anim_data.x;
			int columns = tex_anim_data.y;
			int cycles = tex_anim_data.z;
			if (ImGui::InputInt("Rows: ##ps_ta_rows", &rows)) tex_anim_data.x = rows;
			if (ImGui::InputInt("Columns: ##ps_ta_columns", &columns)) tex_anim_data.y = columns;
			if (ImGui::InputInt("Cycles: ##ps_ta_cycles", &cycles)) tex_anim_data.z = cycles;
		}		
	}
}

void ComponentParticleSystem::InspectorShape()
{
	if (ImGui::CollapsingHeader("Shape ##ps_shape"))
	{
		const char* menu_text;

		switch (shape_type)
		{
		case SHAPE_BOX:
			menu_text = "Shape: Box";
			break;
		case SHAPE_SPHERE:
			menu_text = "Shape: Sphere";
			break;
		default:
			menu_text = "Shape: UNKNOWN";
			break;
		}
		if (ImGui::BeginMenu(menu_text))
		{
			if (ImGui::MenuItem("Box ##ps_shape_box"))
				shape_type = SHAPE_BOX;

			if (ImGui::MenuItem("Sphere ##ps_shape_sphere"))
				shape_type = SHAPE_SPHERE;
			ImGui::EndMenu();
		}

		switch (shape_type)
		{
		case SHAPE_BOX:
			ImGui::Text("Box X/Y/Z: "); ImGui::SameLine();
			if (ImGui::DragFloat3("##ps_bs", box_shape.ptr(), 0.1, 0.001, 1000))
			{
				box_shape_obb.r = box_shape * 0.5f;
			}
			break;
		case SHAPE_SPHERE:
			ImGui::DragFloat("Radius: ##ps_shape_sphere_radius", &sphere_shape.r, 0.1f, 0.00001f, 1000000.0f);
			ImGui::Checkbox("Emit from Shell: ", &sphere_emit_from_shell);
			break;
		}
		
	}
}

void ComponentParticleSystem::InspectorBoundingBox()
{
	if (ImGui::CollapsingHeader("Bounding Box ###ps_bounding_box"))
	{
		if (ImGui::DragFloat3("Size ##ps_size_bb", bb_size.ptr(), 0.01f))
		{
			float3 center = bounding_box.CenterPoint();
			float3 half = bb_size * 0.5f;
			bounding_box.minPoint = center - half;
			bounding_box.maxPoint = center + half;
		}

		if (ImGui::DragFloat3("Offset ##ps_pos_offset_bb", bb_pos_offset.ptr(), 0.01f))
		{
			bounding_box.Translate((game_object->transform->GetPosition() + bb_pos_offset) - bounding_box.CenterPoint());
		}
		g_Debug->AddAABB(bounding_box, g_Debug->red, 2.0f);		
	}
}

void ComponentParticleSystem::InspectorEmission()
{
	if (ImGui::CollapsingHeader("Emission ###ps_emission"))
	{
		ImGui::Text("Emission rate: "); ImGui::SameLine();
		if (ImGui::DragFloat("###ps_emission_drag", &emission_rate, 1.0f, 0.0f, 500.0f))
		{
			spawn_time = 1.0f / emission_rate;
			spawn_timer = 0.0f;
		}

		ImGui::Text("Burst");
		ImGui::Separator();

		vector<int> bursts_to_remove;

		for (int i = 0; i < bursts.size(); ++i)
		{
			ImGui::PushID(i*4);
			ImGui::DragFloat("Time: ###ps_burst_time", &bursts[i].time);
			ImGui::PopID(); ImGui::PushID(i * 4 + 1);
			ImGui::DragInt("Min particles: ###ps_burst_min_p", &bursts[i].min_particles);
			ImGui::PopID(); ImGui::PushID(i*4 + 2);
			ImGui::DragInt("Max particles: ###ps_burst_min_p", &bursts[i].max_particles);
			ImGui::PopID();
			ImGui::PushID(i * 4 + 3);
			if (ImGui::Button("Remove ##ps_remove_burst"))
				bursts_to_remove.push_back(i);
			ImGui::PopID();
			ImGui::Separator();
		}

		if (ImGui::Button("Add burst"))
		{
			bursts.push_back(Burst());
		}

		for (int i = 0; i < bursts_to_remove.size(); ++i)
			bursts.erase(bursts.begin() + bursts_to_remove[i]);
		
	}	
}

void ComponentParticleSystem::InspectorSimulationSpace()
{
	if (simulation_space_local)
		ImGui::Text("Simulation Space: Local");
	else
		ImGui::Text("Simulation Space: World");
	ImGui::SameLine();
	if (ImGui::BeginMenu("###ps_simulation_space_menu"))
	{
		if (ImGui::MenuItem("Local Space"))
			simulation_space_local = true;

		if (ImGui::MenuItem("World Space"))
			simulation_space_local = false;

		ImGui::EndMenu();
	}
}

void ComponentParticleSystem::SpawnParticle(int delay)
{
	int id = FindUnusedParticle();

	Particle& p = particles_container[id];

	p.life = life_time - delay * spawn_time + time->RealDeltaTime();
	p.position = math::float3(0.0f);
	p.next_c_id = 0;
	p.cam_distance = -1.0f;
	p.color = float4(color, 1.0f);

	switch (shape_type)
	{
	case SHAPE_BOX:
		p.origin = box_shape_obb.RandomPointInside(rnd);
		if (simulation_space_local)
			p.dst_origin = p.origin - game_object->GetGlobalMatrix().TranslatePart();
		p.speed = math::float3(0, speed, 0);
		break;
	case SHAPE_SPHERE:
		if (!sphere_emit_from_shell)
			p.origin = sphere_shape.RandomPointInside(rnd);
		else
			p.origin = sphere_shape.RandomPointOnSurface(rnd);
		p.speed = (p.origin - game_object->GetGlobalMatrix().TranslatePart()).Normalized() * speed;
		if (simulation_space_local)
			p.dst_origin = p.origin - game_object->GetGlobalMatrix().TranslatePart();
		break;
	default:
		p.origin = float3(0.0f);
		break;
	}
	
}

int ComponentParticleSystem::FindUnusedParticle()
{
	for (int i = 0; i < top_max_particles; ++i)
	{
		if (particles_container[i].life < 0)
		{
			return i;
		}
	}

	return 0;
}

bool Particle::operator<(Particle & b)
{
	return this->cam_distance > b.cam_distance;
}

ColorTimeItem::ColorTimeItem(float alpha, float position, const math::float3 & color) : alpha(alpha), position(position), color(color)
{}
