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

ComponentParticleSystem::ComponentParticleSystem(ComponentType type, GameObject* game_object) : Component(type, game_object), color(1), cti_entry(1, 0, float3(1)), tex_anim_data(1)
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
		ImGui::Text("Lifetime: "); ImGui::SameLine(); ImGui::DragFloat("###ps_lifetime", &life_time, 1.0f, 0.0f, 1000.0f);
		ImGui::Text("Speed: "); ImGui::SameLine(); ImGui::DragFloat("###ps_speed", &speed, 1.0f, 0.0, 1000.0f);
		ImGui::Text("Size: "); ImGui::SameLine(); ImGui::DragFloat("###ps_size", &size, 1.0f, 0.0, 1000.0f);
		if (ImGui::CollapsingHeader("Start color: ")) { ImGui::ColorPicker("###ps_start_color", color.ptr()); }
		ImGui::Text("Max particles: "); ImGui::SameLine(); ImGui::DragInt("###max_particles", &max_particles, 1, 0, 1000);
		ImGui::Text("Play On Awake: "); ImGui::SameLine(); ImGui::Checkbox("###ps_play_awake", &play_on_awake);

		ImGui::Text("Emission rate: "); ImGui::SameLine(); 
		if (ImGui::DragFloat("###ps_emission", &emission_rate, 1.0f, 0.0f, 500.0f))
		{
			spawn_time = 1.0f / emission_rate;
			spawn_timer = 0.0f;
		}

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

	shape_type = (ParticleShapeType)conf.GetInt("box_shape");
	switch (shape_type)
	{
	case SHAPE_BOX:
		box_shape = conf.GetFloat3("box_shape");
		break;
	case SHAPE_SPHERE:
		sphere_shape.r = conf.GetFloat("sphere_radius");
		sphere_emit_from_shell = conf.GetBool("sphere_emit_from_shell");
		break;
	}
	

	texture_anim = conf.GetBool("texture_anim");
	if (texture_anim)
		tex_anim_data = conf.GetFloat3("tex_anim_data");

	string tex_path = conf.GetString("texture");
	if (tex_path.size() > 0)
	{
		ResourceFileTexture* rc_tmp = (ResourceFileTexture*)App->resource_manager->LoadResource(tex_path, ResourceFileType::RES_TEXTURE);
		if (rc_tmp)
			texture = rc_tmp;
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

	if (playing_editor || is_playing)
	{
		spawn_timer += time->RealDeltaTime();

		if (spawn_timer >= spawn_time)
		{
			int num_particles_to_spawn = spawn_timer / spawn_time;

			for (int i = 0; i < num_particles_to_spawn; i++)
				SpawnParticle(i);

			spawn_timer -= spawn_time * num_particles_to_spawn;
		}

	}

}

void ComponentParticleSystem::PostUpdate()
{
	BROFILER_CATEGORY("ComponentParticleSystem::PostUpdate", Profiler::Color::Navy);

	if (playing_editor || is_playing)
	{
		float dt = time->RealDeltaTime();

		num_alive_particles = 0;

		Quat rotation = game_object->GetGlobalMatrix().RotatePart().ToQuat();

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
					p.position = p.origin + (rotation * p.speed) * item_life;

					item_life_pc = (item_life / life_time) * 100.0;

					if (color_time[p.next_c_id]->position <= item_life_pc)
						p.next_c_id++;

					previous = color_time[p.next_c_id - 1];
					next = color_time[p.next_c_id];
			
					c_pc = (item_life_pc - previous->position) / (next->position - previous->position);
					p.color.x = previous->color.x * (1.0f - c_pc) + next->color.x * c_pc;
					p.color.y = previous->color.y * (1.0f - c_pc) + next->color.y * c_pc;
					p.color.z = previous->color.z * (1.0f - c_pc) + next->color.z * c_pc;
					p.color.w = previous->alpha * (1.0f - c_pc) + next->alpha * c_pc;

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

		App->renderer3D->AddToDrawParticle(this);

		if(playing_editor)
			simulation_time += dt;
	}
	
}

void ComponentParticleSystem::OnTransformModified()
{
	box_shape_obb.pos = game_object->GetGlobalMatrix().TranslatePart();
	Quat rotation = game_object->GetGlobalMatrix().RotatePart().ToQuat();
	box_shape_obb.axis[0] = rotation * float3::unitX;
	box_shape_obb.axis[1] = rotation * float3::unitY;
	box_shape_obb.axis[2] = rotation * float3::unitZ;

	sphere_shape.pos = game_object->GetGlobalMatrix().TranslatePart();
}

void ComponentParticleSystem::OnPlay()
{
	if (play_on_awake)
	{
		is_playing = true;
	}
}

void ComponentParticleSystem::OnPause()
{
	//Do not create new particles and do not update positions
}

void ComponentParticleSystem::OnStop()
{
	if (is_playing)
	{
		is_playing = false;
		StopAll();
	}
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
		playing_editor = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop##ps_stop"))
	{
		playing_editor = false;
		simulation_time = 0.0f;
		StopAll();
	}
	ImGui::Text("Playback Time: %.2f", simulation_time);
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

void ComponentParticleSystem::SpawnParticle(int delay)
{
	int id = FindUnusedParticle();

	Particle& p = particles_container[id];

	p.life = life_time - delay * spawn_time + time->RealDeltaTime();
	p.position = math::float3(0.0f);
	p.next_c_id = 0;
	p.cam_distance = -1.0f;

	switch (shape_type)
	{
	case SHAPE_BOX:
		p.origin = box_shape_obb.RandomPointInside(rnd);
		p.speed = math::float3(0, speed, 0);
		break;
	case SHAPE_SPHERE:
		if (!sphere_emit_from_shell)
			p.origin = sphere_shape.RandomPointInside(rnd);
		else
			p.origin = sphere_shape.RandomPointOnSurface(rnd);
		p.speed = (p.origin - game_object->transform->GetPosition()).Normalized() * speed;
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
