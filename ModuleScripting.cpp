#include "ModuleScripting.h"
#include "Application.h"
#include "Globals.h"
#include "ModuleFileSystem.h"

// DOCUMENTATION

// http://www.giorgosdimtsas.net/embedding-mono-in-your-c-application/
// http://www.c-sharpcorner.com/UploadFile/66489a/creating-C-Sharp-class-library-dll-using-visual-studio-2015-pre/

// Mono libraries
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>

#pragma comment( lib, "C:/Program Files (x86)/Mono/lib/mono-2.0-boehm.lib")
//#pragma comment( lib, "C:/Program Files (x86)/Mono/lib/mono-2.0-sgen.lib")

ModuleScripting::ModuleScripting(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	// point to the relevant directories of the Mono installation
	mono_set_dirs("C:\\Program Files (x86)\\Mono\\lib", "C:\\Program Files (x86)\\Mono\\etc");

	// load the default Mono configuration file in 'etc/mono/config'
	mono_config_parse(nullptr);
	MonoDomain* monoDomain = mono_jit_init_version("embedding_mono_domain", "v4.0.30319");

	// open our Example.dll assembly
	MonoAssembly* assembly = mono_domain_assembly_open(monoDomain, "GameScripts.dll");
	MonoImage* monoImage = mono_assembly_get_image(assembly);

	// find the Entity class in the image
	MonoClass* entityClass = mono_class_from_name(monoImage, "GameScripts", "Entity");
	const char *dir = mono_get_config_dir();

	// allocate memory for one Entity instance
	MonoObject* entityInstance = mono_object_new(monoDomain, entityClass);

	// find the Entity class constructor method that takes one parameter
	MonoMethod* constructorMethod = mono_class_get_method_from_name(entityClass, ".ctor", 1);

	// create a MonoString that will be passed to the constructor as an argument
	MonoString* arg = mono_string_new(mono_domain_get(), "Giorgos");
	void* args[1];
	args[0] = arg;

	// finally, invoke the constructor
	MonoObject* exception = NULL;
	mono_runtime_invoke(constructorMethod, entityInstance, args, &exception);

	// find the Process method that takes zero parameters
	MonoMethod* processMethod = mono_class_get_method_from_name(entityClass,"Process", 0);
	exception = nullptr;

	// invoke the method
	// if invoking static methods, then the second argument must be NULL
	mono_runtime_invoke(processMethod, entityInstance, nullptr, &exception);

	// check for any thrown exception
	if (exception)
	{
		std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr))
			<< std::endl;
	}

	// find the GetName method
	MonoMethod* getNameMethod = mono_class_get_method_from_name(entityClass, "GetName", 0);
	exception = nullptr;
	MonoString* ret = (MonoString*)mono_runtime_invoke(getNameMethod, entityInstance, nullptr, &exception);
	char* c = mono_string_to_utf8(ret);
	std::cout << "Value of 'name' is " << c << std::endl;

	// free the memory allocated from mono_string_to_utf8 ()
	mono_free(c);

	finded_script_names = false; 
	scripts_quantity = -1;
}

ModuleScripting::~ModuleScripting()
{
}

bool ModuleScripting::Init(Data &config)
{
	return true;
}

bool ModuleScripting::Start()
{
	
	LoadScriptsLibrary();
	return true;
}
/*
update_status ModuleScripting::PreUpdate()
{
	return UPDATE_CONTINUE;
}

update_status ModuleScripting::Update()
{
	return UPDATE_CONTINUE;
}

update_status ModuleScripting::PostUpdate()
{
	return UPDATE_CONTINUE;
}*/

bool ModuleScripting::CleanUp()
{
	// shutdown mono
	//mono_jit_cleanup(monoDomain);

	return true;
}

void ModuleScripting::SaveBeforeClosing(Data &data) const
{
}

DWORD ModuleScripting::GetError()
{
	return last_error;
}

void ModuleScripting::LoadScriptsLibrary()
{
	if (App->file_system->Exists("Game.dll"))
	{
		scripts_loaded = false;
		
		LOG("Can't find Game.dll", "Game.dll", last_error);
	}
	else
	{
		scripts_loaded = true;
		//LoadScriptNames();
	}
}

void ModuleScripting::LoadScriptNames()
{
	script_names.clear();
	script_names.push_back(" ");
	if (scripts_quantity != -1)
		names = "";

	if (scripts_loaded)
	{
		//ScriptNames::GetScriptNames(App);
	}
}

vector<const char*> ModuleScripting::GetScriptNamesList()const
{
	return script_names;
}

const char* ModuleScripting::GetScriptNames()const
{
	return names;
}

void ModuleScripting::SetScriptNames(const char* names)
{
	this->names = names;
}

void ModuleScripting::AddScriptName(const char* name)
{
	script_names.push_back(name);
}
