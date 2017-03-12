#include "ModuleScripting.h"

// DOCUMENTATION
// http://www.giorgosdimtsas.net/embedding-mono-in-your-c-application/
// http://www.c-sharpcorner.com/UploadFile/66489a/creating-C-Sharp-class-library-dll-using-visual-studio-2015-pre/
// http://docs.go-mono.com/?link=root:/embed

// Mono libraries
#include <mono/jit/jit.h>
#include <mono/metadata/metadata.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>

#pragma comment( lib, "C:/Program Files (x86)/Mono/lib/mono-2.0-boehm.lib")
//#pragma comment( lib, "C:/Program Files (x86)/Mono/lib/mono-2.0-sgen.lib")

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
	InitMonoLibrary();
	return true;
}

bool ModuleScripting::Start()
{
	return true;
}


bool ModuleScripting::CleanUp()
{
	// Freeing script collection
	for (std::vector<ClassInfo*>::iterator it_class = script_collection.begin(); it_class != script_collection.end(); ++it_class)
	{
		for (std::vector<MethodInfo*>::iterator it_method = (*it_class)->methods.begin(); it_method != (*it_class)->methods.end(); ++it_method)
			delete (*it_method);

		for (std::vector<FieldInfo*>::iterator it_field = (*it_class)->fields.begin(); it_field != (*it_class)->fields.end(); ++it_field)
			delete (*it_field);

		delete (*it_class);
	}

	TerminateMonoLibrary();
	return true;
}

void ModuleScripting::SaveBeforeClosing(Data &data) const
{ }

void ModuleScripting::ObtainScripts(std::vector<ClassInfo*> &scripts) const
{
	for (std::vector<ClassInfo*>::const_iterator it_cl = script_collection.begin(); it_cl != script_collection.end(); ++it_cl)
		scripts.push_back(*it_cl);
}

void ModuleScripting::InitMonoLibrary()
{
	// point to the relevant directories of the Mono installation
	mono_set_dirs("C:\\Program Files (x86)\\Mono\\lib", "C:\\Program Files (x86)\\Mono\\etc");

	// load the default Mono configuration file in 'etc/mono/config'
	mono_config_parse(nullptr);
	mono_domain = mono_jit_init_version("embedding_mono_domain", "v4.0.30319");

	// open our GameScripts.dll assembly
	MonoAssembly* assembly = mono_domain_assembly_open(mono_domain, "GameScripts.dll");
	MonoImage* mono_image = mono_assembly_get_image(assembly);

	LoadClasses(mono_image);
}

void ModuleScripting::TerminateMonoLibrary()
{
	//shutdown mono
	if(mono_domain)	mono_jit_cleanup(mono_domain);
}

void ModuleScripting::LoadClasses(MonoImage *mono_image)
{
	// Get a pointer to the MonoTableTypeDef metadata table
	const MonoTableInfo *t = mono_image_get_table_info(mono_image, MONO_TABLE_TYPEDEF);

	// Fetch the number of rows available in the table
	int rows = mono_table_info_get_rows(t);

	// For each row, print some of its values
	for (int i = 1; i < rows; i++)
	{
		/* Space where we extract one row from the metadata table */
		uint32_t cols[MONO_TYPEDEF_SIZE];

		/* Extract the row into the array cols */
		mono_metadata_decode_row(t, i, cols, MONO_TYPEDEF_SIZE);

		ClassInfo *class_info = new ClassInfo();
		class_info->name = mono_metadata_string_heap(mono_image, cols[MONO_TYPEDEF_NAME]);
		MonoClass* mono_class = mono_class_from_name(mono_image, "GameScripts", class_info->name.c_str());		
		
		// Getting methods information
		void *iter = nullptr;
		MonoMethod *method;

		while (method = mono_class_get_methods(mono_class, &iter))
		{
			MethodInfo *method_info = new MethodInfo();
			MonoMethodSignature* sig = mono_method_signature(method);

			MonoType* type; 
			void *iter2 = nullptr;
			while (type = mono_signature_get_params(sig, &iter2))
			{
				method_info->types.push_back(type);
			}

			class_info->methods.push_back(method_info);
			method_info->name = mono_method_get_name(method);			
		}

		// Getting fields information
		iter = nullptr;
		MonoClassField *field;

		while (field = mono_class_get_fields(mono_class, &iter))
		{
			FieldInfo *field_info = new FieldInfo();
			class_info->fields.push_back(field_info);
			field_info->name = mono_field_get_name(field);
			const char *type = mono_type_get_name(mono_field_get_type(field));
			int j = 0;
		}

		script_collection.push_back(class_info);
	}
}
