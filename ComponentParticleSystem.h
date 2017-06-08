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
	math::float3 origin;
	math::float3 dst_origin;
	float life = -1.0f;
	float cam_distance = -1.0f;
	int next_c_id = 0;
	math::float4 color;

	bool operator<(Particle& b);
};

struct ColorTimeItem
{
	float alpha;
	float position; //In %
	math::float3 color;

	ColorTimeItem(float alpha, float position, const math::float3& color);
};

enum ParticleShapeType
{
	SHAPE_BOX,
	SHAPE_SPHERE
};

enum PSState
{
	PS_PLAYING,
	PS_PAUSE,
	PS_STOP
};

struct Burst
{
	float time = 0.0f;
	int min_particles = 10;
	int max_particles = 10;
	bool completed = false;
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
	void OnTransformModified();

	void OnPlay();
	void OnPause();
	void OnStop();

	unsigned int GetTextureId()const;

	void SortParticles(ComponentCamera* cam);

	void StopAll(); //Stops the particle system and removes the alive particles
	void Play();
	void Pause();

	bool operator<(ComponentParticleSystem& b);

private:

	void InspectorDelete();
	void InspectorChangeTexture();
	void InspectorSimulation(); //Play/Stop simulation
	void InspectorColorOverTime();
	void InspectorTextureAnimation();
	void InspectorShape();
	void InspectorBoundingBox();
	void InspectorEmission();
	void InspectorSimulationSpace();

	void SpawnParticle(int delay);
	int FindUnusedParticle();

private:
	int cycles = 0; //Life cycles of the system

	//Properties
	int max_particles = 1000;
	float speed = 1.0f;
	bool play_on_awake = true;
	float duration = 5.0f;
	bool looping = true;
	bool simulation_space_local = true; //False -> simulation space world

	//Color over time
public:
	bool color_over_time_active = false;
private:
	std::vector<ColorTimeItem*> color_time;
	ColorTimeItem cti_entry;

	//Shape
	ParticleShapeType shape_type = SHAPE_BOX;
	math::float3 box_shape = math::float3(1);
	math::OBB box_shape_obb;
	math::Sphere sphere_shape;
	bool sphere_emit_from_shell = false;

	//Animated texture
public:
	bool texture_anim = false;
	math::float3 tex_anim_data; //x-rows y-columns z-cycles
	
private:
	ResourceFileTexture* texture = nullptr;

	//Emission
	float emission_rate = 10.0f;
	float spawn_time = 0.1f; // 1 / emission rate
	float spawn_timer = 0.0;
	std::vector<Burst> bursts;
	
	const int top_max_particles = 1000; //Maximum number of particles (not editable for the user)
	std::vector<Particle> particles_container;
	

	//Simulation in editor
	float system_life = 0.0f;
	PSState state;

	math::LCG rnd;

	//Bounding box
public:
	math::AABB bounding_box;
private:
	math::float3 bb_size;
	math::float3 bb_pos_offset;

public:
	std::vector<math::float3> alive_particles_position;
	std::vector<math::float4> alive_particles_color;
	std::vector<float> alive_particles_life;

	//Buffers
	unsigned int position_buffer = 0;
	unsigned int color_buffer = 0;
	unsigned int life_buffer = 0;

	int num_alive_particles = 0; //Number of particles alive

	//Properites
	float size = 1.0f;
	math::float2 img_size;
	math::float3 color;
	float life_time = 5.0f;

	//To sort the entire system
	float cam_distance = 0.0f;
};

#endif // !__COMPONENTPARTICLESYTEM_H__
