#ifndef __COMPONENTUIIMAGE_H__
#define __COMPONENTUIIMAGE_H__

#include "Globals.h"
#include "Component.h"
#include "Material.h"
#include <string>
#include <map>

class ResourceFileTexture;
class ResourceFileMaterial;
class ResourceFile;

class ComponentUiImage : public Component
{

public:

	ComponentUiImage(ComponentType type, GameObject* game_object);

	void Update();
	void CleanUp();

	void OnInspector(bool debug);

	// Save
	void Save(Data& file)const;
	void Load(Data& conf);

public:
	std::string material_path; //To Library. If is "" means that this component uses the default material.
	ResourceFileMaterial* rc_material = nullptr;
	std::map<std::string, uint> texture_ids; //name of the variable texture in the shader and id
private:
	std::string material_name = "Default"; //Assets path
										   //Note: All materials must have model, view and projection uniforms. 
	std::vector<ResourceFile*> tex_resources;
	std::vector<std::string> list_textures_paths; //Default textures from fbx
	bool change_material_enabled = false;

};

#endif __COMPONENTUIIMAGE_H__