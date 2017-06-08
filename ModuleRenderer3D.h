#ifndef __MODULERENDERER3D_H__
#define __MODULERENDERER3D_H__

#include "Module.h"
#include "MathGeoLib\include\MathGeoLib.h"

#include "Light.h"
#include "Subject.h"

#include <vector>
#include <utility> // for pair struct

#define MAX_LIGHTS 8

using namespace math;

struct Mesh;
class GameObject;
class ComponentCamera;
class ComponentMaterial;
typedef void *SDL_GLContext;
class ComponentSprite;
class ComponentParticleSystem;
class MasterRender;
class ShadowMap;

class ModuleRenderer3D : public Module, public Subject
{
public:
	ModuleRenderer3D(const char* name, bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(Data& config);
	update_status PreUpdate();
	update_status PostUpdate();
	bool CleanUp();

	void OnResize(int width, int height, float fovy);
	void UpdateProjectionMatrix(ComponentCamera* camera);
	const ComponentCamera* GetCamera() const;
	void SetCamera(ComponentCamera* camera);
	void AddCamera(ComponentCamera* camera);
	void CleanCameras();

	void AddToDraw(GameObject* obj);
	void AddToDrawSprite(ComponentSprite* sprite);
	void AddToDrawParticle(ComponentParticleSystem* particle_sys);

	void SetClearColor(const math::float3& color)const;
	void RemoveBuffer(unsigned int id);
	void RemoveTextureBuffer(unsigned int id);
	
	void DrawLine(float3 pos1, float3 pos2, float4 color = float4(1,1,1,1));
	void DrawLocator(float4x4 transform, float4 color = float4(1, 1, 1, 1));
	void DrawLocator(float3 pos, Quat rot, float4 color = float4(1, 1, 1, 1));
	void DrawAABB(float3 minPoint, float3 maxPoint, float4 color = float4(1, 1, 1, 1));
	void DrawUIImage(GameObject* obj)const;
	void DrawUIText(GameObject* obj)const;

private:

	void DrawScene(ComponentCamera* cam, bool has_render_tex = false);
	void Draw(GameObject* obj, const LightInfo& light, ComponentCamera* cam, std::pair<float, GameObject*>& alpha_object,bool alpha_render = false)const;
	void DrawAnimated(GameObject* obj, const LightInfo& light, ComponentCamera* cam, std::pair<float, GameObject*>& alpha_object, bool alpha_render = false)const;
	void DrawSprites(ComponentCamera* cam)const;
	void DrawParticles(ComponentCamera* cam);

	bool SetShaderAlpha(ComponentMaterial* material, ComponentCamera* cam, GameObject* obj, std::pair<float, GameObject*>& alpha_object, bool alpha_render = false)const;
	void SetShaderUniforms(unsigned int shader_id, GameObject* obj, ComponentCamera* cam, ComponentMaterial* material, const LightInfo& light, const float4& color)const;
	void ShaderMVPUniforms(unsigned int shader_id, GameObject* obj, ComponentCamera* cam)const;
	void ShaderTexturesUniforms(unsigned int shader_id, ComponentMaterial* material)const;
	void ShaderLightUniforms(unsigned int shader_id, const LightInfo& light)const;
	void ShaderCustomUniforms(unsigned int shader_id, ComponentMaterial* material)const;
	void ShaderBuiltInUniforms(unsigned int shader_id, ComponentCamera* cam, ComponentMaterial* material, const float4 color)const;

public:

	bool renderAABBs = false;
	Light lights[MAX_LIGHTS];
	SDL_GLContext context;
	float3x3 NormalMatrix;
	float4x4 ModelMatrix, ViewMatrix, ProjectionMatrix;
	std::vector<ComponentCamera*> cameras;

	MasterRender* ms_render;
	ShadowMap* shadow_map;
private:

	std::vector<GameObject*> objects_to_draw;
	std::vector<ComponentSprite*> sprites_to_draw;
	std::vector<ComponentParticleSystem*> particles_to_draw;
};

#endif // !__MODULERENDERER3D_H__