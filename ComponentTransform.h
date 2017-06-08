#ifndef __COMPONENT_TRANSFORM_H__
#define __COMPONENT_TRANSFORM_H__

#include "Component.h"
#include "MathGeoLib\include\MathGeoLib.h"

enum GuizmoOperation
{
	TRANSLATE,
	ROTATION,
	SCALE
};

class ComponentTransform : public Component
{
public:
	ComponentTransform(ComponentType type, GameObject* game_object, math::float4x4** global_matrix);
	~ComponentTransform();

	void Update();

	void OnInspector(bool debug);

	void SetPosition(const math::float3& pos);
	void SetRotation(const math::float3 &rot_euler);
	void SetRotation(const math::Quat& rot);
	void SetScale(const math::float3& scale);
	void Set(math::float4x4 matrix);
	void SetGlobal(float4x4 global);

	

	math::float3 GetPosition()const;
	math::float3 GetRotationEuler()const;
	math::Quat GetRotation()const;
	math::float3 GetScale()const;

	void Rotate(const math::Quat& quaternion);

	math::float4x4 GetLocalTransformMatrix()const;
	// Returns the final transformation matrix. Not the local one!
	math::float4x4 GetTransformMatrix()const;
	math::float4x4 GetGlobalMatrix()const;

	math::float3 GetForward() const;

	void Save(Data& file)const;
	void Load(Data& conf);
	void Reset();
	void Remove();
	void SaveAsPrefab(Data& file)const;

	void UpdateMatrix();
private:
	void CalculateFinalTransform();

private:
	math::float3 position = math::float3::zero;
	math::Quat rotation = math::Quat::identity;
	math::float3 scale = math::float3::one;

	math::float3 rotation_euler = math::float3::zero;

	math::float4x4 transform_matrix = math::float4x4::identity;
	math::float4x4 final_transform_matrix = math::float4x4::identity;

	bool transform_modified = false;
};

#endif // !__COMPONENT_TRANSFORM_H__
