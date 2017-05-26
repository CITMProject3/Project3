#include "PhysBody3D.h"

#include "Globals.h"
#include "Bullet\include\btBulletDynamicsCommon.h"

// =================================================
PhysBody3D::PhysBody3D(btRigidBody* body, ComponentCollider* col) : body(body)
{
	collider = col;
	body->setUserPointer(this);
}

PhysBody3D::PhysBody3D(btRigidBody* body, ComponentCar* col) : body(body)
{
	car = col;
	body->setUserPointer(this);
	SetCar(true);
}

// ---------------------------------------------------------
PhysBody3D::~PhysBody3D()
{
	if (body != nullptr)
	{
		delete body;
		body = nullptr;
	}
}

// ---------------------------------------------------------
void PhysBody3D::Push(float x, float y, float z)
{
	body->applyCentralImpulse(btVector3(x, y, z));
}

// ---------------------------------------------------------
float4x4 PhysBody3D::GetTransform() const
{
	float4x4 ret = float4x4::identity;
	if(body != NULL)
	{
		float tmp[16];
		body->getWorldTransform().getOpenGLMatrix(tmp);
		ret.Set(tmp);
	}
	return ret;
}

// ---------------------------------------------------------
void PhysBody3D::SetTransform(const float* matrix) const
{
	if(body != nullptr && matrix != nullptr)
	{
		btTransform t;
		t.setFromOpenGLMatrix(matrix);
		//body->setWorldTransform(t);
		body->getMotionState()->setWorldTransform(t);
	}
}

// ---------------------------------------------------------
void PhysBody3D::SetPos(float x, float y, float z)
{
	btTransform t = body->getWorldTransform();
	t.setOrigin(btVector3(x, y, z));
	body->setWorldTransform(t);
}

//----------------------------------------------------------
void PhysBody3D::SetRotation(float x, float y, float z)
{
	btTransform t = body->getWorldTransform();
	btQuaternion q;
	q.setEulerZYX(z, y, x);
	t.setRotation(q);
	body->setWorldTransform(t);
}

void PhysBody3D::SetRotation(Quat rot)
{
	btTransform t = body->getWorldTransform();
	btQuaternion q(rot.x, rot.y, rot.z, rot.w);
	t.setRotation(q);
	body->setWorldTransform(t);
}

// ---------------------------------------------------------
void PhysBody3D::Stop()
{
	body->setLinearVelocity(btVector3(0, 0, 0));
	body->setAngularVelocity(btVector3(0, 0, 0));
	body->clearForces();
}

// ---------------------------------------------------------
btTransform PhysBody3D::GetRealTransform()const
{
	return body->getWorldTransform();
}

//----------------------------------------------------------
void PhysBody3D::ApplyCentralForce(btVector3& force)
{
	body->applyCentralForce(force);
}

//---------------------------------------------------------
void PhysBody3D::SetFriction(float friction)
{
	body->setFriction(friction);
}

void PhysBody3D::SetBounciness(float restitution, float friction)
{
	body->setFriction(friction);
	body->setRestitution(restitution);
}

//----------------------------------------------------------
void PhysBody3D::SetAngularSpeed(float x, float y, float z)
{
	body->setAngularVelocity(btVector3(x, y, z));
}

//----------------------------------------------------------
void PhysBody3D::SetLinearSpeed(float x, float y, float z)
{
	body->setLinearVelocity(btVector3(x, y, z));
}

void PhysBody3D::SetModularSpeed(float s)
{
	btVector3 sp = body->getLinearVelocity();
	if (sp.length2() > 0.0f)
	{
		sp.normalize();
		sp *= s;

		body->setLinearVelocity(sp);
	}
}
//----------------------------------------------------------
math::vec PhysBody3D::GetPosition()const
{
	math::vec ret;
	ret.x = body->getWorldTransform().getOrigin().getX();
	ret.y = body->getWorldTransform().getOrigin().getY();
	ret.z = body->getWorldTransform().getOrigin().getZ();

	return ret;
}

void PhysBody3D::SetTrigger(bool value, TriggerType t_type)
{
	is_trigger = value;
	trigger_type = t_type;
}

void PhysBody3D::SetCar(bool value)
{
	is_car = value;
}

bool PhysBody3D::IsCar() const
{
	return is_car;
}

bool PhysBody3D::IsTrigger() const
{
	return is_trigger;
}

void PhysBody3D::SetActivationState(int state)
{
	body->forceActivationState(state);
}