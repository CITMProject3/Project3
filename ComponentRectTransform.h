#ifndef __COMPONENTRECTTRANSFORM_H__
#define __COMPONENTRECTTRANSFORM_H__

#include "Component.h"
#include "MathGeoLib\include\MathGeoLib.h"

class ResourceFileMesh;
struct Mesh;
class ComponentRectTransform : public Component
{

private:

	float3 global_position;				// World position representation
	float3 local_position;					// Position representation;
	float3 local_middle_position;
	float2 rect_size;					// RectTransform;
	float3 size;							// Size representation;

	bool apply_transformation = false;

public:

	ComponentRectTransform(ComponentType type, GameObject* game_object);
	
	void GeneratePlane();

	void Update();

	void OnInspector(bool debug);

	void CalculateFinalTransform();
	void OnTransformModified();
	math::float4x4 GetFinalTransform();
	// Utilities
	void Move(const float3 &movement);

	Mesh* GetMesh();
	 
	bool ContainsPoint(const float2 &point) const;

	// Setters / Getters
	const float2 GetGlobalPos() const;
	const float2 GetLocalPos() const;

	void SetLocalPos(const float2 &local_pos);
	void SetSize(const float2 &size);
	void SetScale(const float2 &size);
	void SetZPlane(float z);
	float2 GetRectSize()const;
	void ResizePlane();
	
	// Save
	void Save(Data& file)const;
	void Load(Data& conf);
	void Remove();

	int order = 0; // Range 0 to 7

private:
	ResourceFileMesh* plane = nullptr;

	math::float4x4 transform_matrix = math::float4x4::identity;
	math::float4x4 final_transform_matrix = math::float4x4::identity;
};

#endif __COMPONENTRECTTRANSFORM_H__