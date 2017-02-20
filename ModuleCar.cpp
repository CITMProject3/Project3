#include "Globals.h"
#include "Application.h"
#include "ModuleCar.h"

ModuleCar::ModuleCar(const char* name, bool start_enabled) : Module(name, start_enabled)
{
}

// Destructor
ModuleCar::~ModuleCar()
{
}

// Called before render is available
bool ModuleCar::Init(Data& config)
{
	bool ret = true;
	return ret;
}

// Called every draw update
update_status ModuleCar::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModuleCar::Update()
{
	return UPDATE_CONTINUE;
}

update_status ModuleCar::PostUpdate()
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleCar::CleanUp()
{
	return true;
}