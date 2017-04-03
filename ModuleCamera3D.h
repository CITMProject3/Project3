#ifndef __MODULECAMERA3D_H__
#define __MODULECAMERA3D_H__

#include "Module.h"
#include "MathGeoLib\include\MathGeoLib.h"

class ComponentCamera;

class ModuleCamera3D : public Module
{
public:

	ModuleCamera3D(const char* name, bool start_enabled = true);
	~ModuleCamera3D();

	bool Init(Data& config);
	bool Start();
	update_status Update();
	bool CleanUp();

	void OnPlay();
	void OnStop();

	math::float3 GetPosition() const;
	math::float4x4 GetViewMatrix() const;

	float GetNearPlane()const;
	float GetFarPlane()const;
	float GetFOV()const;
	float GetAspectRatio() const;

	void SetNearPlane(const float& near_plane);
	void SetFarPlane(const float& far_plane);
	void SetFOV(const float& fov);
	void SetBackgroundColor(const math::float3& color);
	void SetAspectRatio(float ar);

	//Movement---------------------
	bool MoveArrows(float dt);
	bool MoveMouse(float dt);
	void Orbit(int x, int y);
	void Zoom(float value);

	void Center(const float3& position);

	math::float3 GetBackgroundColor()const;
	ComponentCamera* GetEditorCamera() const;

	void AddSceneCamera(ComponentCamera* cam);
	void RemoveSceneCamera(ComponentCamera* cam);

private:

	float3 reference;
	ComponentCamera* camera = nullptr;

	void EditorCameraMovement(float dt);	
	std::vector<ComponentCamera*> scene_cameras; //Cameras in the current scene
};

#endif // !__MODULECAMERA3D_H__
