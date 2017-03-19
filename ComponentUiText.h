#ifndef __COMPONENTUITEXT_H__
#define __COMPONENTUITEXT_H__

#include "Globals.h"
#include "Component.h"

class ComponentMaterial;

class ComponentUiText : public Component
{

public:

	ComponentUiText(ComponentType type, GameObject* game_object);
	~ComponentUiText();
	void Update(float dt);
	void CleanUp();

	void OnInspector(bool debug);

	void Save(Data& file)const;
	void Load(Data& conf);
	ComponentMaterial* UImaterial = nullptr;
	
	int GetLenght() const;
	string GetText() const;
	string GetArrayValues() const;
	int GetCharRows() const;
	float GetCharwidth() const;
	float GetCharHeight() const;
	float GetImgWidth() const;
	float GetImgHeight()const;
	void SetText(string &text);
private:
	void GenerateFont();
	bool OnChangeTexture();
	bool change_text = false;
	bool change_array_values = false;

	string text = "";
	string array_values = "";
	uint len = 0;
	uint img_width = 0;
	uint img_height = 0;
	uint char_row = 0;
	uint char_w = 0;
	uint char_h = 0;
	uint rows = 0;

	string current_text_changing = "";
};

#endif // !__COMPONENTUITEXT_H__