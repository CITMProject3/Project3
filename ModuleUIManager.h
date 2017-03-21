#ifndef __MODULEUIMANAGER_H__
#define __MODULEUIMANAGER_H__

#include "Module.h"

class GameObject;

class ModuleUIManager : public Module
{
public:
	ModuleUIManager(const char* name, bool start_enabled = true);
	~ModuleUIManager();

	bool Init(Data& config);
	bool Start();
	update_status PreUpdate();
	update_status Update();
	void SaveBeforeClosing(Data& data)const;



	GameObject* CreateUIGameObject(GameObject* parent);


private:


};


#endif __MODULEUIMANAGER_H__


