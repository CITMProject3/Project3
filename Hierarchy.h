#ifndef __HIERARCHY_H__
#define __HIERARCHY_H__

#include "Window.h"
#include <vector>

class GameObject;

class Hierarchy : public Window
{
public:
	Hierarchy();
	~Hierarchy();

	void Draw(ImGuiWindowFlags flags);
	void DisplayGameObjectsChilds(const std::vector<GameObject*>* childs);

	bool settingParent = false;
};

#endif
