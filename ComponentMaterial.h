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

	void OnInspector();
	void Update();

	void Save(Data& file)const;
	void Load(Data& conf);

private:
	void PrintMaterialProperties();
	void CleanUp();

public:
	std::string material_path; //To Library. If is "" means that this component uses the default material.
	ResourceFileMaterial* rc_material = nullptr;
	std::map<string, uint> texture_ids; //name of the variable texture in the shader and id
private:
	std::string material_name; //Assets path
	//Note: All materials must have model, view and projection uniforms. 
	std::vector<ResourceFile*> tex_resources;
	std::vector<std::string> list_textures_paths; //Default textures from fbx
	bool change_material_enabled = false;

};
#endif // !__COMPONENT_MATERIAL_H__
