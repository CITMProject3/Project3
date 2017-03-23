#ifndef __COMPONENT_COLLIDER_H__
#define __COMPONENT_COLLIDER_H__

#include "Component.h"
#include "MathGeoLib\include\MathGeoLib.h"

class btConvexHullShape;
struct PhysBody3D;
class Primitive;

enum Collider_Shapes
{
	S_NONE = 0,
	S_CUBE,
	S_SPHERE,
	S_CONVEX
};


class ComponentCollider : public Component
{
public:
	ComponentCollider(GameObject* game_object);
	~ComponentCollider();

	void Update();

	void OnPlay();
	void OnStop();

	void OnInspector(bool debug);

	void OnTransformModified();

	void Save(Data& file) const;
	void Load(Data& config);

	void SetShape(Collider_Shapes new_shape);

private:
	void LoadShape();
public:
	bool Static = true;
	float mass = 10.0f;

	Collider_Shapes shape = S_CUBE;
	float3 offset_pos;
	float3 offset_scale;
	PhysBody3D* body;
	Primitive* primitive = nullptr;
	btConvexHullShape* convexShape = nullptr;

	unsigned char collision_flags = 0;
	int n = 0;
};
#endif // !__COMPONENT_COLLIDER_H__
