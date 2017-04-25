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

#include "ModuleEditor.h"
#include "ModuleWindow.h"
#include "ModuleRenderer3D.h"

#include "ComponentMesh.h"
#include "ComponentTransform.h"

#include "Brofiler\include\Brofiler.h"

#include <string>
using namespace std;

ComponentParticleSystem::ComponentParticleSystem(ComponentType type, GameObject* game_object) : Component(type, game_object)
{
	BROFILER_CATEGORY("ComponentParticleSystem::Init", Profiler::Color::Navy);
	particles.resize(1024);
	std::fill(particles.begin(), particles.end(), 0);

	for (int i = 0; i < 1024; i++)
		available_ids.push(i);

	fboA = OpenGLFunc::CreateFBOColorOnly(32, 32, textureA);
	fboB = OpenGLFunc::CreateFBOColorOnly(32, 32, textureB);

	update_position_shader = App->resource_manager->GetDefaultParticlePositionShaderId();
	quad_position = App->resource_manager->GetDefaultQuadParticleMesh();

	live_particles_id.resize(1024);
	std::fill(live_particles_id.begin(), live_particles_id.end(), 0.0f);

	glGenTextures(1, &p_lifes_tex);
	glBindTexture(GL_TEXTURE_2D, p_lifes_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, 32, 32, 0, GL_RED, GL_FLOAT, particles.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenBuffers(1, &live_particles_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, live_particles_buffer);
	glBufferData(GL_ARRAY_BUFFER, 1024 * sizeof(float), NULL, GL_STREAM_DRAW);

}

ComponentParticleSystem::~ComponentParticleSystem()
{
	particles.clear();
}

void ComponentParticleSystem::OnInspector(bool debug)
{
	if (ImGui::CollapsingHeader("ParticleSystem", ImGuiTreeNodeFlags_DefaultOpen))
	{
		InspectorDelete();

		//Main Options
		ImGui::Text("Lifetime: "); ImGui::SameLine(); ImGui::DragFloat("###ps_lifetime", &life_time, 1.0f, 0.0f, 1000.0f);
		ImGui::Text("Speed: "); ImGui::SameLine(); ImGui::DragFloat("###ps_speed", &speed, 1.0f, 0.0, 1000.0f);
		ImGui::Text("Max particles: "); ImGui::SameLine(); ImGui::DragInt("###max_particles", &max_particles, 1, 0, 1000);

		ImGui::Text("Emission rate: "); ImGui::SameLine(); 
		if (ImGui::DragFloat("###ps_emission", &emission_rate, 1.0f, 0.0f, 500.0f))
		{
			spawn_time = 1.0f / emission_rate;
			spawn_timer = 0.0f;
		}

		//Debug
		if (debug)
		{
			ImGui::Text("Life particles %i", life_particles);
		}

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

	live_particles_id.clear();
	spawn_timer += time->RealDeltaTime();

	if (spawn_timer >= spawn_time)
	{
		int num_particles_to_spawn = spawn_timer / spawn_time;

		for(int i = 0; i < num_particles_to_spawn; i++)
			SpawnParticle();

		spawn_timer -= spawn_time * num_particles_to_spawn;
	}

	double current_time = time->RealTimeSinceStartup();
	for (int i = 0; i < particles.size(); ++i)
	{
		if (particles[i] > 0) //Live
		{
			if (current_time - particles[i] >= life_time)
			{
				particles[i] = 0;
				available_ids.push(i);
				--life_particles;
			}
			else
				live_particles_id.push_back(i);
		}
	}
}

void ComponentParticleSystem::PostUpdate()
{
	BROFILER_CATEGORY("ComponentParticleSystem::UpdatePositions", Profiler::Color::Navy);
	if (pingpong_tex)
		UpdateParticlesPosition(fboA, textureB);
	else
		UpdateParticlesPosition(fboB, textureA);

	pingpong_tex = !pingpong_tex;

	glBindBuffer(GL_ARRAY_BUFFER, live_particles_buffer);
	glBufferData(GL_ARRAY_BUFFER, 1024 * sizeof(float), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, live_particles_id.size() * sizeof(float), live_particles_id.data());

	App->renderer3D->AddToDrawParticle(this);
}

unsigned int ComponentParticleSystem::GetTextureId() const
{
	return (texture) ? texture->GetTexture() : 0;
}

unsigned int ComponentParticleSystem::GetPositionTextureId() const
{
	return (pingpong_tex) ? textureA : textureB;
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

void ComponentParticleSystem::SpawnParticle()
{
	if (available_ids.size() > 0 && life_particles < max_particles)
	{
		unsigned int id = available_ids.top();
		available_ids.pop();

		particles[id] = time->RealTimeSinceStartup();

		++life_particles;
	}
}

void ComponentParticleSystem::UpdateParticlesPosition(unsigned int fbo, unsigned int tex)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo); //Ping -pong fbo & texture
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, 32, 32);

	glUseProgram(update_position_shader);

	glBindTexture(GL_TEXTURE_2D, p_lifes_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, 32, 32, 0, GL_RED, GL_FLOAT, particles.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	//Uniforms
	GLint origin_location = glGetUniformLocation(update_position_shader, "origin");
	GLint speed_location = glGetUniformLocation(update_position_shader, "speed");
	GLint current_time_location = glGetUniformLocation(update_position_shader, "current_time");
	GLint p_life_location = glGetUniformLocation(update_position_shader, "p_life");
	GLint rotation = glGetUniformLocation(update_position_shader, "rotation");

	glUniform3fv(origin_location, 1, reinterpret_cast<const GLfloat*>(game_object->transform->GetGlobalMatrix().TranslatePart().ptr()));
	glUniform1f(speed_location, speed); 
	glUniform1f(current_time_location, time->RealTimeSinceStartup());
	glUniform1f(p_life_location, life_time);
	glUniform4fv(rotation, 1, reinterpret_cast<GLfloat*>(game_object->transform->GetGlobalMatrix().RotatePart().ToQuat().Inverted().ptr()));

	glActiveTexture(GL_TEXTURE0);
	GLint texture_location = glGetUniformLocation(update_position_shader, "tex");
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(texture_location, 0);


	glActiveTexture(GL_TEXTURE1);
	GLint texture2_location = glGetUniformLocation(update_position_shader, "particles_life");
	glBindTexture(GL_TEXTURE_2D, p_lifes_tex);
	glUniform1i(texture2_location, 1);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_position->id_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_position->id_indices);
	glDrawElements(GL_TRIANGLES, quad_position->num_indices, GL_UNSIGNED_INT, (void*)0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, App->window->GetScreenWidth(), App->window->GetScreenHeight());
}
