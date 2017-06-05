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

	void Render(const math::float4x4& light_matrix, const std::vector<GameObject*>& entities); //Creates the ShadowMap
	unsigned int GetShadowMapId()const;
	math::float4x4 GetShadowView()const;
	math::float4x4 GetShadowProjection()const;

private:

	void UpdateShadowBox(const math::float4x4& light_matrix);

private:
	unsigned int fbo_id = 0;
	unsigned int shadow_map_id = 0;

	ComponentCamera* cam;
	int width, height;

	math::Frustum frustum;

	math::float4x4 shadowView;

	//Utilities
	math::float4x4 shadowProjection;

	math::OBB obb;
};

#endif // !__SHADOWMAP_H_

