#include "ModuleUIManager.h"

ModuleUIManager::ModuleUIManager(const char * name, bool start_enabled) : Module(name,start_enabled)
{
}

ModuleUIManager::~ModuleUIManager()
{
}

bool ModuleUIManager::Init(Data & config)
{
	return false;
}

bool ModuleUIManager::Start()
{
	return false;
}

update_status ModuleUIManager::PreUpdate()
{

	return UPDATE_CONTINUE;
}

update_status ModuleUIManager::Update()
{

	return UPDATE_CONTINUE;
}

void ModuleUIManager::SaveBeforeClosing(Data & data) const
{
}

GameObject * ModuleUIManager::CreateUIGameObject(GameObject * parent)
{
	return nullptr;
}
