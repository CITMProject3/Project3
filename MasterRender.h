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

private:
	void InitDefaultShader();
	void InitDefaultNormalShader();

public:
	DFShader df_shader;
};


#endif // !__MASTER_RENDER_H__
