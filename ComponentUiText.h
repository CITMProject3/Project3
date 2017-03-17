#ifndef __COMPONENTUITEXT_H__
#define __COMPONENTUITEXT_H__

#include "Component.h"
#include <string>

class ComponentUiText : public Component
{

public:

	ComponentUiText(GameObject* game_object);

	void Update(float dt);
	void PostUpdate();
	void CleanUp();

	void OnInspector(bool debug);

	void SetText(std::string);

	void Save(Data& file)const;
	void Load(Data& conf);

private:
	char buffer[200];
	std::string text;
};

#endif // !__COMPONENTUITEXT_H__