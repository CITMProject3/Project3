#ifndef __COMPONENT_CAMERA_H__
#define __COMPONENT_CAMERA_H__

#include "Component.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include "Observer.h"

class ResourceFileRenderTexture;

class ComponentCamera : public Component, public Observer
{
	friend class ModuleCamera3D;
public:
	ComponentCamera(ComponentType type, GameObject* game_object);
	~ComponentCamera();

	void PreUpdate();
	void Update();

	void OnInspector(bool debug);
	void OnTransformModified();
	void OnNotify(void* entity, Event event);
	void UpdateViewportDimensions();

	float GetNearPlane()const;
	float GetFarPlane()const;
	float GetFOV()const;
	math::float3 GetFront()const;
	math::float3 GetUp()const;
	math::float3 GetWorldRight()const;
	math::float3 GetPos()const;

	math::float4x4 GetProjectionMatrix()const;
	math::float4x4 GetViewMatrix()const;
	math::float4x4 GetWorldMatrix()const;
	math::Frustum GetFrustum()const { return frustum; }

	math::float3 GetBackgroundColor()const;

	int GetLayerMask()const;

	void SetNearPlane(float value);
	void SetFarPlane(float value);
	void SetFOV(float value);
	void SetAspectRatio(float value);

	void LookAt(const math::float3& point);
	void Center(const float3& position, float distance);

	void SetBackgroundColor(const math::float3 color);
	bool Intersects(const math::AABB& box)const;

	void Save(Data& file)const;
	void Load(Data& conf);

	math::Ray CastCameraRay(math::float2 screen_pos);
private:
	void UpdateCameraFrustum();

public:

	bool renderTerrain = true;
	bool smoothFollow = false;
	bool block_z_rotation = true;

	bool render_skybox = true;

	bool properties_modified = false;
	ResourceFileRenderTexture* render_texture = nullptr;

	float2 viewport_rel_position;
	float2 viewport_rel_size;

	float2 viewport_position;
	float2 viewport_size;

protected:
	math::Frustum frustum;
private:
	float near_plane = 0.3f;
	float far_plane = 1000.0f;
	float fov = 60;
	float aspect_ratio;

	math::float3 color; 
	int layer_mask = -1;
	
	///Assets path
	std::string render_texture_path;
	std::string render_texture_path_lib;

	math::float4x4 desiredTransform = float4x4::identity;
	math::float4x4 currentTransform = float4x4::identity;
	float followMoveSpeed = 0.1f;
	float followRotateSpeed = 0.1f;

};
#endif // !__COMPONENT_MATERIAL_H__
