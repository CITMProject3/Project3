#include "ModuleScripting.h"

ModuleScripting::ModuleScripting(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	//// point to the relevant directories of the Mono installation
	//mono_set_dirs("C:\\Program Files (x86)\\Mono\\lib", "C:\\Program Files (x86)\\Mono\\etc");

	//// load the default Mono configuration file in 'etc/mono/config'
	//mono_config_parse(nullptr);
	//MonoDomain* monoDomain = mono_jit_init_version("embedding_mono_domain", "v4.0.30319");

	//// open our Example.dll assembly
	//MonoAssembly* assembly = mono_domain_assembly_open(monoDomain, "GameScripts.dll");
	//MonoImage* monoImage = mono_assembly_get_image(assembly);

	//// find the Entity class in the image
	//MonoClass* entityClass = mono_class_from_name(monoImage, "GameScripts", "Entity");
	//void *iter = 0;
	//MonoClassField *field = mono_class_get_fields(entityClass, &iter);
	//while (field)
	//{
	//	field = mono_class_get_fields(entityClass, &iter);
	//	
	//}
	//
	//
	//const char *dir = mono_get_config_dir();

	//// allocate memory for one Entity instance
	//MonoObject* entityInstance = mono_object_new(monoDomain, entityClass);

	//// find the Entity class constructor method that takes one parameter
	//MonoMethod* constructorMethod = mono_class_get_method_from_name(entityClass, ".ctor", 1);

	//// create a MonoString that will be passed to the constructor as an argument
	//MonoString* arg = mono_string_new(mono_domain_get(), "Giorgos");
	//void* args[1];
	//args[0] = arg;

	//// finally, invoke the constructor
	//MonoObject* exception = NULL;
	//mono_runtime_invoke(constructorMethod, entityInstance, args, &exception);

	//// find the Process method that takes zero parameters
	//MonoMethod* processMethod = mono_class_get_method_from_name(entityClass,"Process", 0);
	//exception = nullptr;

	//// invoke the method
	//// if invoking static methods, then the second argument must be NULL
	//mono_runtime_invoke(processMethod, entityInstance, nullptr, &exception);

	//// check for any thrown exception
	//if (exception)
	//{
	//	std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr))
	//		<< std::endl;
	//}

	//// find the GetName method
	//MonoMethod* getNameMethod = mono_class_get_method_from_name(entityClass, "GetName", 0);
	//exception = nullptr;
	//MonoString* ret = (MonoString*)mono_runtime_invoke(getNameMethod, entityInstance, nullptr, &exception);
	//char* c = mono_string_to_utf8(ret);
	//std::cout << "Value of 'name' is " << c << std::endl;

	//// free the memory allocated from mono_string_to_utf8 ()
	//mono_free(c);
}

ModuleScripting::~ModuleScripting()
{
}

bool ModuleScripting::Init(Data &config)
{
	mono_scripts.InitMonoLibrary();
	return true;
}

bool ModuleScripting::Start()
{
	return true;
}


bool ModuleScripting::CleanUp()
{
	mono_scripts.TerminateMonoLibrary();
	return true;
}

void ModuleScripting::SaveBeforeClosing(Data &data) const
{ }

void ModuleScripting::ObtainScriptNames(std::vector<std::string> &script_names)
{
	mono_scripts.GetClassNames(script_names);
}



