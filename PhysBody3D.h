#ifndef __PhysBody3D_H__
#define __PhysBody3D_H__

#include <list>
#include "MathGeoLib\include\MathGeoLib.h"

using namespace std;

class btRigidBody;
class btTransform;
class btVector3;
class Module;
class GameObject;

// =================================================
struct PhysBody3D
{
	friend class ModulePhysics3D;
public:

	enum E_collisionOptions {
		co_isTrigger = 0x01,
		co_isTransparent = 0x02,
		co_isPlayer = 0x04,
		co_isItem = 0x08,
		co_isOutOfBounds = 0x10,
		co_isCheckpoint = 0x20,
		co_isFinishLane = 0x40,
		co_none = 0x80
	};

	PhysBody3D(btRigidBody* body, GameObject* go);
	~PhysBody3D();

	void Push(float x, float y, float z);
	float4x4 GetTransform() const;
	void SetTransform(const float* matrix) const;
	void SetPos(float x, float y, float z);
	void Stop();
	void SetRotation(float x, float y, float z);
	void SetAngularSpeed(float x, float y, float z);
	void SetLinearSpeed(float x, float y, float z);
	void SetModularSpeed(float s);

	math::vec GetPosition()const;

	btTransform GetRealTransform()const;
	void ApplyCentralForce(btVector3& force);

	void SetFriction(float friction);
	void SetBounciness(const float restitution,const float friction);

	GameObject* GetGameobject() { return go; }

	void SetCollisionOptions(E_collisionOptions flag, bool value);
	bool GetCollisionOptions(E_collisionOptions flag);

private:
	btRigidBody* body = nullptr;
	GameObject* go = nullptr;
public:
	unsigned char collisionOptions = 0;
};

#endif // __PhysBody3D_H__