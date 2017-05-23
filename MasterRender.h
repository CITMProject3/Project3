#ifndef __MASTER_RENDER_H__
#define __MASTER_RENDER_H__

struct DFShader
{
	unsigned int id;

	unsigned int model, view, projection;
	unsigned int has_texture, texture;
	unsigned int Ia, Id, Is;
	unsigned int Ka, Kd;
	unsigned int L; 
	unsigned int material_color;
};

struct DFNormalShader
{
	unsigned int id;

	unsigned int model, view, projection;
	unsigned int texture, normal;
	unsigned int Ia, Id, Is;
	unsigned int Ka, Kd;
	unsigned int L;
	unsigned int material_color;
};

struct AnimShader
{
	unsigned int id;

	unsigned int model, view, projection;
	unsigned int has_texture, texture;
	unsigned int Ia, Id, Is;
	unsigned int Ka, Kd;
	unsigned int L;
	unsigned int material_color;
	unsigned int bones;
};

struct AnimNormalShader
{
	unsigned int id;

	unsigned int model, view, projection;
	unsigned int texture, normal;
	unsigned int Ia, Id, Is;
	unsigned int Ka, Kd;
	unsigned int L;
	unsigned int material_color;
	unsigned int bones;
};

class GameObject;
class ComponentCamera;
class ComponentMaterial;
struct LightInfo;

class MasterRender
{
public:
	MasterRender();
	~MasterRender();

	void Init();

	void RenderDefaultShader(const GameObject* obj, const ComponentCamera* cam, const ComponentMaterial* material,const LightInfo* light)const;
	void RenderNormalShader(const GameObject* obj, const ComponentCamera* cam, const ComponentMaterial* material, const LightInfo* light)const;
	void RenderAnimShader(const GameObject* obj, const ComponentCamera* cam, const ComponentMaterial* material, const LightInfo* light)const;
	void RenderAnimNormalShader(const GameObject* obj, const ComponentCamera* cam, const ComponentMaterial* material, const LightInfo* light)const;

private:
	void InitDefaultShader();
	void InitDefaultNormalShader();
	void InitAnimShader();
	void InitAnimNormalShader();

public:
	DFShader df_shader;
	DFNormalShader normal_shader;
	AnimShader anim_shader;
	AnimNormalShader anim_normal_shader;
};


#endif // !__MASTER_RENDER_H__
