#ifndef __MODULECAMERA3D_H__
#define __MODULECAMERA3D_H__

#include "Module.h"
#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"

class GameObject;
class ComponentCamera;
class ComponentTransform;

class ModuleCamera3D : public Module
{
public:
	ModuleCamera3D(const char* name, bool start_enabled = true);
	~ModuleCamera3D();

	bool Init(Data& config);
	bool Start();
	update_status Update();
	bool CleanUp();

	math::float3 GetPosition() const;
	math::float4x4 GetViewMatrix() const;

	float GetNearPlane()const;
	float GetFarPlane()const;
	float GetFOV()const;

	void SetNearPlane(const float& near_plane);
	void SetFarPlane(const float& far_plane);
	void SetFOV(const float& fov);
	void SetBackgroundColor(const math::float3& color);

	math::float3 GetBackgroundColor()const;
	ComponentCamera* GetCurrentCamera()const;
	void ChangeCurrentCamera(ComponentCamera* camera);
	ComponentCamera* GetEditorCamera() const;

private:

	void EditorCameraMovement(float dt);

private:
	GameObject* go_cam = nullptr;
	ComponentTransform* cam_transform = nullptr;
	ComponentCamera* editor_cam = nullptr;

	ComponentCamera* current_camera = nullptr;

};

#endif // !__MODULECAMERA3D_H__
