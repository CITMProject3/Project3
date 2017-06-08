#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

#include "Component.h"
#include "Material.h"
#include <string>
#include <map>

class ResourceFileTexture;
class ResourceFileMaterial;
class ResourceFile;

class ComponentMaterial : public Component
{
public:
	ComponentMaterial(ComponentType type, GameObject* game_object);
	~ComponentMaterial();

	void OnInspector(bool debug);

	void Save(Data& file)const;
	void Load(Data& conf);

	int GetIdToRender()const;
	void SetIdToRender(int new_id);

	bool DefaultMaterialInspector();
private:
	void PrintMaterialProperties();
	void ChooseAlphaType();
	bool ChangeTextureNoMaterial(std::string tex_num,int num);
	void ChangeTexture(std::string tex_num, Uniform* &value);
	void RefreshTextures();
	bool AddTexture();
	//Name in "texture_ids" map
	void RemoveTexture(std::string name);
	void CleanUp();

public:
	std::string material_path; //To Library. If is "" means that this component uses the default material.
	ResourceFileMaterial* rc_material = nullptr;
	std::map<std::string, uint> texture_ids; //name of the variable texture in the shader and id

	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	int alpha = 0;
	float alpha_test = 0.25f;
	int blend_type = 0;
	float specular = 0.0f;
	bool has_normal = false;

	std::vector<std::string> list_textures_paths; //Default textures from fbx

private:
	std::string material_name = "Default"; //Assets path
	//Note: All materials must have model, view and projection uniforms. 
	std::vector<ResourceFile*> tex_resources;
	bool change_material_enabled = false;
	bool texture_changed = false;
	std::string delete_texture_name;
	int id_to_render = 0;
	std::string material_assets_path;

};
#endif // !__COMPONENT_MATERIAL_H__
