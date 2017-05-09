#include "Wheel.h"
#include "ComponentCar.h"

#include "RaycastHit.h"
#include "ModulePhysics3D.h"
#include "GameObject.h"
#include "ComponentTransform.h"

void Wheel::Cast()
{
	math::Ray Ray;
	float3 pos = kart->GetKartTRS()->GetPosition() + kart->kartY * 1.5f;
	Ray.pos = pos + kart->kartX * posOffset.x + kart->kartZ * posOffset.y;
	Ray.dir = float3(kart->kartX * dir.x + kart->kartY * dir.y + kart->kartZ * dir.z);
	Ray.dir.Normalize();
	RaycastHit hitResult;

	hit = App->physics->RayCast(Ray, hitResult);
	if (hit)
	{
		distance = hitResult.distance;
		hitNormal = hitResult.normal;
		hitPoint = hitResult.point;
		angleFromY = hitResult.normal.AngleBetween(float3(0, 1, 0));
	}
	else
	{
		distance = floatMax - 10.0f;
		hitNormal = float3(0, 1, 0);
		hitPoint = float3(0, 0, 0);
		angleFromY = 0.0f;
	}
}