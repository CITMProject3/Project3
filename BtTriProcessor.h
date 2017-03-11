#ifndef __BT_TRI_PROCESSOR__
#define __BT_TRI_PROCESSOR__

#include "Application.h"
#include "Bullet\include\btBulletDynamicsCommon.h"

class BtTriPRocessor : public btTriangleCallback
{
public:
	BtTriPRocessor();

	void processTriangle(btVector3* triangle, int partId, int triangleIndex);

	bool useWire = false;
	float3 color;
};

#endif