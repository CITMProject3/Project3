#ifndef __COMPONENTUITEXT_H__
#define __COMPONENTUITEXT_H__

#include "Globals.h"
#include "Component.h"
//#include <vector>
class ComponentMaterial;
class ResourceFileMesh;
class Mesh;

class ComponentUiText : public Component
{

public:

	ComponentUiText(ComponentType type, GameObject* game_object);
	~ComponentUiText();
	void Update();
	void CleanUp();

	void OnInspector(bool debug);

	void Save(Data& file)const;
	void Load(Data& conf);
	ComponentMaterial* UImaterial = nullptr;
	
	int GetLenght() const;
	string GetText() const;
	string GetArrayValues() const;
	int GetCharRows() const;
	int GetCharOffset() const;
	float GetCharwidth(int i) const;
	float GetCharHeight(int i) const;
	float GetImgWidth() const;
	float GetImgHeight()const;

	void SetText(string &text);
	void SetDisplayText(string text);
	void SetCharOffset(int off);

	vector<Mesh*> meshes;
private:
	void GenerateFont();
	bool OnChangeTexture();
	void GeneratePlane();
	bool change_text = false;
	bool change_array_values = false;
	vector<ResourceFileMesh*> planes;
	string text = "";
	string array_values = "";
	uint len = 0;
	uint img_width = 0;
	uint img_height = 0;
	int* char_w;
	int* char_h;
	int char_offset = 0;
	int text_type = 0;
	string current_text_changing = "";
	ResourceFileMesh* tplane = nullptr;
};

#endif // !__COMPONENTUITEXT_H__