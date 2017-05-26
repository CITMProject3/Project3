#ifndef __SHADOWMAP_H__
#define __SHADOWMAP_H__

#include "MathGeoLib\include\MathGeoLib.h"

class ComponentCamera;
class GameObject;

class ShadowMap
{
public:
	ShadowMap();
	~ShadowMap();

	void Init(int width, int height);
	void CleanUp();

	void Render(const math::float3& light_dir, const std::vector<GameObject*>& entities); //Creates the ShadowMap

private:

	void UpdateShadowBox(const math::float3& light_dir);

private:
	unsigned int fbo_id = 0;
	unsigned int shadow_map_id = 0;

	ComponentCamera* cam;
	int width, height;

	math::Frustum frustum;
};

#endif // !__SHADOWMAP_H_

