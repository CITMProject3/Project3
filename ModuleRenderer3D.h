#ifndef __MODULERENDERER3D_H__
#define __MODULERENDERER3D_H__

#include "Module.h"
#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include "Light.h"
#include <vector>
#include "Subject.h"
#include "ModuleLighting.h"

#define MAX_LIGHTS 8

using namespace math;

class Mesh;
class GameObject;
class ComponentCamera;

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
	void SetPerspective(const math::float4x4& perspective);
	void AddToDraw(GameObject* obj);

	void SetClearColor(const math::float3& color)const;
	void RemoveBuffer(unsigned int id);

private:

	void DrawScene(ComponentCamera* cam, bool has_render_tex = false)const;
	void Draw(GameObject* obj, const LightInfo& light, ComponentCamera* cam)const;

public:

	Light lights[MAX_LIGHTS];
	SDL_GLContext context;
	float3x3 NormalMatrix;
	float4x4 ModelMatrix, ViewMatrix, ProjectionMatrix;

private:

	vector<GameObject*> objects_to_draw;

};

#endif // !__MODULERENDERER3D_H__