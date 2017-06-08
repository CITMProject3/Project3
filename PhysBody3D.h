#ifndef __PhysBody3D_H__
#define __PhysBody3D_H__

#include <list>
#include "MathGeoLib\include\MathGeoLib.h"

using namespace std;

class btRigidBody;
class btTransform;
class btVector3;
class Module;

class ComponentCollider;
class ComponentCar;

enum TriggerType;

// =================================================
class PhysBody3D
{
	friend class ModulePhysics3D;
public:

	PhysBody3D(btRigidBody* body, ComponentCollider* col);
	PhysBody3D(btRigidBody* body, ComponentCar* car);
	~PhysBody3D();

	void Push(float x, float y, float z);
	float4x4 GetTransform() const;
	void SetTransform(const float* matrix) const;
	void SetPos(float x, float y, float z);
	void Stop();
	void SetRotation(float x, float y, float z);
	void SetRotation(Quat rot);
	void SetAngularSpeed(float x, float y, float z);
	void SetLinearSpeed(float x, float y, float z);
	void SetModularSpeed(float s);

	math::vec GetPosition()const;

	btTransform GetRealTransform()const;
	void ApplyCentralForce(btVector3& force);

	void SetFriction(float friction);
	void SetBounciness(const float restitution,const float friction);

	ComponentCollider* GetCollider() { return collider; }
	ComponentCar* GetCar() { return car; }

	void SetTrigger(bool value, TriggerType t_type);
	void SetCar(bool value);
	bool IsCar() const;
	bool IsTrigger() const;
	void SetActivationState(int state);

private:

	btRigidBody* body = nullptr;
	
	ComponentCollider* collider = nullptr;
	ComponentCar* car = nullptr;

	bool is_trigger = false;
	TriggerType trigger_type;
	bool is_car = false;

};

#endif // __PhysBody3D_H__