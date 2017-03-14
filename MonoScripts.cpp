#include "MonoScripts.h"

#include "Globals.h"

#include "MonoLibraries.h"
#include "ClassInfo.h"
#include "MethodInfo.h"
#include "FieldInfo.h"

#pragma comment( lib, "C:/Program Files (x86)/Mono/lib/mono-2.0-boehm.lib")

void MonoScripts::Init()
{
	InitMonoLibrary();
}

void MonoScripts::Terminate()
{
	TerminateMonoLibrary();
}

void MonoScripts::GetScripts(std::vector<ClassInfo*> &scripts)
{
	for (std::vector<ClassInfo*>::const_iterator it_cl = script_collection.begin(); it_cl != script_collection.end(); ++it_cl)
		scripts.push_back(*it_cl);
}

bool MonoScripts::LoadScript(ClassInfo *script)
{
	bool ret = true;

	// allocate memory for one Entity instance
	script->instanced_class = mono_object_new(mono_domain, script->mono_class);

	// find the Entity class constructor method that takes one parameter
	MonoMethod* constructor = mono_class_get_method_from_name(script->mono_class, ".ctor", 0);  // TODO: Constructor must need more than zero arguments

	// finally, invoke the constructor
	MonoObject* exception = nullptr;
	mono_runtime_invoke(constructor, script->instanced_class, nullptr, &exception);

	if (exception != nullptr)
	{
		LOG("Script %s hasn't been loaded", script->GetName());
		//mono_free(script->instanced_class);
		script->instanced_class = nullptr;
		ret = false;
	}

	return ret;
}

MonoDataType MonoScripts::TypeConversion(MonoType* type)
{
	// name_type will be useful in the future, in order to know what is the fucking variable type
	const char *name_type = mono_type_get_name(type);
	MonoTypeEnum type_mono = (MonoTypeEnum)mono_type_get_type(type);

	MonoDataType return_type;

	switch (type_mono)
	{
	// INT
	case(MonoTypeEnum::MONO_TYPE_I2): return_type = MonoDataType::MONO_DATA_TYPE_INT16; break;  // Int 16 bits ( 2 bytes )
	case(MonoTypeEnum::MONO_TYPE_I4): return_type = MonoDataType::MONO_DATA_TYPE_INT32; break;  // Int 32 bits ( 4 bytes )
	case(MonoTypeEnum::MONO_TYPE_I8): return_type = MonoDataType::MONO_DATA_TYPE_INT64; break;  // Int 64 bits ( 8 bytes )

	// UINT																				
	case(MonoTypeEnum::MONO_TYPE_U2): return_type = MonoDataType::MONO_DATA_TYPE_UINT16; break;  // Uint 16 bits ( 2 bytes )
	case(MonoTypeEnum::MONO_TYPE_U4): return_type = MonoDataType::MONO_DATA_TYPE_UINT32; break;  // Uint 32 bits ( 4 bytes )
	case(MonoTypeEnum::MONO_TYPE_U8): return_type = MonoDataType::MONO_DATA_TYPE_UINT64; break;  // Uint 64 bits ( 8 bytes )

	// FLOATS																					
	case(MonoTypeEnum::MONO_TYPE_R4): return_type = MonoDataType::MONO_DATA_TYPE_FLOAT; break;	 // Floating point 32 bits ( 4 bytes )
	case(MonoTypeEnum::MONO_TYPE_R8): return_type = MonoDataType::MONO_DATA_TYPE_DOUBLE; break;  // Floating point 64 bits ( 8 bytes )

	// CHARS
	case(MonoTypeEnum::MONO_TYPE_CHAR): return_type = MonoDataType::MONO_DATA_TYPE_CHAR; break;		 // Char
	case(MonoTypeEnum::MONO_TYPE_STRING): return_type = MonoDataType::MONO_DATA_TYPE_STRING; break;  // String

	// There are more types that are not considered
	
	default: return_type = MonoDataType::MONO_DATA_TYPE_NONE;  // Error: No Type conversion available

	}

	return return_type;
}

MonoDataType MonoScripts::TypeConversion(MonoClassField* field)
{
	return TypeConversion(mono_field_get_type(field));
}

void MonoScripts::CreateFieldInfo(ClassInfo *class_info, MonoClassField *field)
{
	MonoDataType type = TypeConversion(field);

	switch (type)
	{
		case(MonoDataType::MONO_DATA_TYPE_INT16):
		case(MonoDataType::MONO_DATA_TYPE_INT32):
		case(MonoDataType::MONO_DATA_TYPE_INT64):
		{
			
			break;
		}
	}	
}

void MonoScripts::InitMonoLibrary()
{
	// point to the relevant directories of the Mono installation
	mono_set_dirs("C:\\Program Files (x86)\\Mono\\lib", "C:\\Program Files (x86)\\Mono\\etc");

	// load the default Mono configuration file in 'etc/mono/config'
	mono_config_parse(nullptr);
	//mono_domain = mono_jit_init_version("embedding_mono_domain", "v4.0.30319");
	mono_domain = mono_jit_init("GameScripts.dll");

	// open our GameScripts.dll assembly
	MonoAssembly* assembly = mono_domain_assembly_open(mono_domain, ASSEMBLY_CSHARP_NAME);
	MonoImage* mono_image = mono_assembly_get_image(assembly);

	LoadClassesInfo(mono_image);
}

void MonoScripts::TerminateMonoLibrary()
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

	//shutdown Mono
	if (mono_domain) mono_jit_cleanup(mono_domain);
}

void MonoScripts::LoadClassesInfo(MonoImage *mono_image)
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
		class_info->mono_class = mono_class_from_name(mono_image, "GameScripts", mono_metadata_string_heap(mono_image, cols[MONO_TYPEDEF_NAME]));

		// Getting methods information
		void *iter = nullptr;
		MonoMethod *method;

		while (method = mono_class_get_methods(class_info->mono_class, &iter))
		{
			MethodInfo *method_info = new MethodInfo();
			method_info->mono_method = method;
			class_info->methods.push_back(method_info);
		}

		// Getting fields information
		iter = nullptr;
		MonoClassField *field;

		while (field = mono_class_get_fields(class_info->mono_class, &iter))
		{
			FieldInfo *field_info = new FieldInfo(class_info);
			field_info->mono_field = field;
			field_info->type = TypeConversion(field);
			class_info->fields.push_back(field_info);
		}

		script_collection.push_back(class_info);
	}
}