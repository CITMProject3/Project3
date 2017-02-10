#include "PhysBody3D.h"
#include "Bullet\include\btBulletDynamicsCommon.h"

// =================================================
PhysBody3D::PhysBody3D(btRigidBody* body) : body(body)
{
	body->setUserPointer(this);
}

// ---------------------------------------------------------
PhysBody3D::~PhysBody3D()
{
	delete body;
}

// ---------------------------------------------------------
void PhysBody3D::Push(float x, float y, float z)
{
	body->applyCentralImpulse(btVector3(x, y, z));
}

// ---------------------------------------------------------
void PhysBody3D::GetTransform(float* matrix) const
{
	if(body != NULL && matrix != NULL)
	{
		body->getWorldTransform().getOpenGLMatrix(matrix);
	}
}

// ---------------------------------------------------------
void PhysBody3D::SetTransform(const float* matrix) const
{
	if(body != NULL && matrix != NULL)
	{
		btTransform t;
		t.setFromOpenGLMatrix(matrix);
		body->setWorldTransform(t);
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
void PhysBody3D::SetBounciness(float restitution, float friction)
{
	body->setFriction(friction);
	body->setRestitution(restitution);
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
