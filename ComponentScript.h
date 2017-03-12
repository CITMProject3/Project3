#ifndef __COMPONENT_SCRIPT_H__
#define __COMPONENT_SCRIPT_H__

#include "Component.h"

#include <string>

struct ClassInfo;

class ComponentScript : public Component
{
public:

	ComponentScript(ComponentType type, GameObject* game_object);
	
	~ComponentScript();

	void Update();

	void OnInspector(bool debug);

	void Save(Data& file)const;
	void Load(Data& conf);

private:

	ClassInfo *current_script = nullptr;
	std::string script_selected;
	
};
#endif // !__COMPONENT_SCRIPT_H__
