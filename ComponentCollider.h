#ifndef __COMPONENT_COLLIDER_H__
#define __COMPONENT_COLLIDER_H__

#include "Component.h"
#include "MathGeoLib\include\MathGeoLib.h"

enum Collider_Shapes
{
	S_CUBE = 0,
	S_PLANE,
	S_SPHERE
};


class ComponentCollider : public Component
{
public:
	ComponentCollider(GameObject* game_object);
	~ComponentCollider();

	void Update();

	void OnInspector(bool debug);

	void OnTransformModified();

	void Save(Data& file) const;
	void Load(Data& config);

	Collider_Shapes shape = S_CUBE;

};
#endif // !__COMPONENT_COLLIDER_H__
