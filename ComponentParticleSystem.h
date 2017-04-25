#ifndef __COMPONENTPARTICLESYSTEM_H__
#define __COMPONENTPARTICLESYSTEM_H__

#include "Component.h"
#include <vector>
#include <stack>

class ResourceFileTexture;
struct Mesh;

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

	unsigned int GetTextureId()const;
	unsigned int GetPositionTextureId()const;

private:

	void InspectorDelete();
	void InspectorChangeTexture();

	void SpawnParticle();
	void UpdateParticlesPosition(unsigned int fbo, unsigned int tex);

private:

	//Properties
	float life_time = 5.0f;
	int max_particles = 1000;
	float emission_rate = 10.0f;
	float speed = 1.0f;

	ResourceFileTexture* texture = nullptr;

	std::vector<float> particles; //Handles life & death
	std::stack<unsigned int> available_ids;
	float spawn_time = 0.1f; // 1 / emission rate
	float spawn_timer = 0.0;

	//Save positions in textures
	unsigned int fboA = 0;
	unsigned int fboB = 0;
	unsigned int textureA = 0;
	unsigned int textureB = 0;
	unsigned int p_lifes_tex = 0;
	bool pingpong_tex = true;
	

	unsigned int update_position_shader = 0;
	Mesh* quad_position = nullptr;


	int life_particles = 0;
	

public:
	unsigned int particles_position_buffer = 0;

	unsigned int test_buffer = 0;

	unsigned int live_particles_buffer = 0;
	std::vector<float> live_particles_id;
};

#endif // !__COMPONENTPARTICLESYTEM_H__
