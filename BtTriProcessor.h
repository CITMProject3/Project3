#ifndef __BT_TRI_PROCESSOR__
#define __BT_TRI_PROCESSOR__

#include "Application.h"
#include "ModuleRenderer3D.h"

#include "Bullet\include\btBulletDynamicsCommon.h"
#include "Bullet\include\btBulletCollisionCommon.h"

class BtTriPRocessor : public btTriangleCallback
{
	void processTriangle(btVector3* triangle, int partId, int triangleIndex)
	{
		App->renderer3D->DrawLine(float3(triangle[0].x(), triangle[0].y(), triangle[0].z()), float3(triangle[1].x(), triangle[1].y(), triangle[1].z()));
		App->renderer3D->DrawLine(float3(triangle[2].x(), triangle[2].y(), triangle[2].z()), float3(triangle[1].x(), triangle[1].y(), triangle[1].z()));
		App->renderer3D->DrawLine(float3(triangle[0].x(), triangle[0].y(), triangle[0].z()), float3(triangle[2].x(), triangle[2].y(), triangle[2].z()));
	}
};

#endif