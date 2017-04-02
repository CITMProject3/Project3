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

// =================================================
class PhysBody3D
{
	friend class ModulePhysics3D;
public:

	//enum E_collisionOptions {//Flags for collision options. 8 bools stored in one byte
	//	co_isTrigger = 0x01,		//00000001
	//	co_isTransparent = 0x02,	//00000010
	//	co_isCar = 0x04,			//00000100
	//	co_isItem = 0x08,			//00001000
	//	co_isOutOfBounds = 0x10,	//00010000
	//	co_isCheckpoint = 0x20,		//00100000
	//	co_isFinishLane = 0x40,		//01000000
	//	co_none = 0x80				//10000000
	//};

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

	void SetTrigger(bool value)
	{
		isTrigger = value;
	}

	void SetCar(bool value)
	{
		isCar = value;
	}

	bool IsCar() const
	{
		return isCar;
	}

	bool IsTrigger() const
	{
		return isTrigger;
	}

private:

	btRigidBody* body = nullptr;
	
	ComponentCollider* collider = nullptr;
	ComponentCar* car = nullptr;

	bool isTrigger = false;
	bool isCar = false;

//public:

	//unsigned char collisionOptions = 0;
};

#endif // __PhysBody3D_H__