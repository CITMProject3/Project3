#ifndef __COMPONENTPARTICLESYSTEM_H__
#define __COMPONENTPARTICLESYSTEM_H__

#include "Component.h"
#include <vector>
#include <stack>
#include "MathGeoLib\include\MathGeoLib.h"

class ResourceFileTexture;
struct Mesh;
class ComponentCamera;

struct Particle
{
	math::float3 position;
	math::float3 speed;
	float life = -1.0f;
	float cam_distance = -1.0f;

	bool operator<(Particle& b);
};

class ComponentParticleSystem : public Component
{
public:
	ComponentParticleSystem(ComponentType type, GameObject* game_object);
	~ComponentParticleSystem();

	void OnInspector(bool debug);
	void Save(Data& file) const;
	void Load(Data& conf);
	void Update();
	void PostUpdate();

	void OnPlay();
	void OnPause();
	void OnStop();

	unsigned int GetTextureId()const;

	void SortParticles(ComponentCamera* cam);

	void StopAll(); //Stops the particle system and removes the alive particles

private:

	void InspectorDelete();
	void InspectorChangeTexture();
	void InspectorSimulation(); //Play/Stop simulation

	void SpawnParticle();
	int FindUnusedParticle();

private:

	//Properties
	float life_time = 5.0f;
	int max_particles = 1000;
	float emission_rate = 10.0f;
	float speed = 1.0f;
	bool play_on_awake = true;
	
	ResourceFileTexture* texture = nullptr;

	float spawn_time = 0.1f; // 1 / emission rate
	float spawn_timer = 0.0;
	
	const int top_max_particles = 1000; //Maximum number of particles (not editable for the user)
	std::vector<Particle> particles_container;
	std::vector<math::float3> alive_particles_position; 
	int last_used_particle = 0; //Id of the last used particle

	//Simulation in editor
	float simulation_time = 0.0f;
	bool playing_editor = false; 

	bool is_playing = false;
public:
	//Buffers
	unsigned int position_buffer = 0;

	int num_alive_particles = 0; //Number of particles alive

	//Properites
	float size = 1.0f;
};

#endif // !__COMPONENTPARTICLESYTEM_H__
