#ifndef __COMPONENT_CAMERA_H__
#define __COMPONENT_CAMERA_H__

#include "Component.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include "Observer.h"

class ResourceFileRenderTexture;

class ComponentCamera : public Component, public Observer
{
public:
	ComponentCamera(ComponentType type, GameObject* game_object);
	~ComponentCamera();

	void Update();

	void OnInspector();
	void OnTransformModified();
	void OnNotify(void* entity, Event event);

	float GetNearPlane()const;
	float GetFarPlane()const;
	float GetFOV()const;
	math::float3 GetFront()const;
	math::float3 GetUp()const;
	math::float4x4 GetProjectionMatrix()const;
	math::float3 GetBackgroundColor()const;
	math::float3 GetWorldRight()const;
	math::float4x4 GetViewMatrix()const;
	math::float4x4 GetWorldMatrix()const;
	int GetLayerMask()const;

	void SetNearPlane(float value);
	void SetFarPlane(float value);
	void SetFOV(float value);
	void LookAt(const math::float3& point);
	void SetBackgroundColor(const math::float3 color);
	bool Intersects(const math::AABB& box)const;


	void Save(Data& file)const;
	void Load(Data& conf);

	math::Ray CastCameraRay(math::float2 screen_pos);

public:

	bool properties_modified = false;
	ResourceFileRenderTexture* render_texture = nullptr;
private:
	float near_plane = 0.3f;
	float far_plane = 1000.0f;
	float fov = 60;
	float aspect_ratio;
	math::Frustum frustum;
	math::float3 color; 
	int layer_mask = -1;
	
	///Assets path
	string render_texture_path;
	string render_texture_path_lib;

};
#endif // !__COMPONENT_MATERIAL_H__
