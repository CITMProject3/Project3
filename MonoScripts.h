#ifndef __MONOSCRIPTS_H__
#define __MONOSCRIPTS_H__

// DOCUMENTATION MONO
 
// --- Implementation ---
// http://www.giorgosdimtsas.net/embedding-mono-in-your-c-application/
// http://www.c-sharpcorner.com/UploadFile/66489a/creating-C-Sharp-class-library-dll-using-visual-studio-2015-pre/
// http://docs.go-mono.com/?link=root:/embed

// --- C++/C bridge ---
// http://stackoverflow.com/questions/11425202/is-it-possible-to-call-a-c-function-from-c-net
// https://www.codeproject.com/articles/9826/how-to-create-a-dll-library-in-c-and-then-use-it-w
// http://tirania.org/blog/archive/2011/Dec-19.html

#include <string>
#include <vector>

#define ASSEMBLY_CSHARP_NAME "GameScripts.dll"

// Forward declarations --->
typedef struct _MonoDomain MonoDomain;
typedef struct _MonoImage MonoImage;
typedef struct _MonoTest MonoTest;
typedef struct _MonoType MonoType;
typedef struct _MonoObject MonoObject;
typedef struct _MonoClassField MonoClassField;

class ClassInfo;
class FieldInfo;
// ---> Forward declarations

// Mono Types
enum MonoDataType
{
	MONO_DATA_TYPE_NONE,

	MONO_DATA_TYPE_VOID,
	MONO_DATA_TYPE_OBJECT,

	MONO_DATA_TYPE_BOOLEAN,
		 
	MONO_DATA_TYPE_ARRAY,
		
	MONO_DATA_TYPE_ENUM,
		
	MONO_DATA_TYPE_INT16,
	MONO_DATA_TYPE_INT32,
	MONO_DATA_TYPE_INT64,
	MONO_DATA_TYPE_UINT16,
	MONO_DATA_TYPE_UINT32,
	MONO_DATA_TYPE_UINT64,
		 
	MONO_DATA_TYPE_INTPTR,
	MONO_DATA_TYPE_UINTPTR,
		
	MONO_DATA_TYPE_FLOAT,
	MONO_DATA_TYPE_DOUBLE,
		
	MONO_DATA_TYPE_BYTE,
	MONO_DATA_TYPE_SBYTE,
	MONO_DATA_TYPE_CHAR,
	MONO_DATA_TYPE_STRING,
		 
	MONO_DATA_TYPE_THREAD
};

class MonoScripts
{
	
private:

	MonoDomain *mono_domain = nullptr;
	std::vector<ClassInfo*> script_collection;	

	void InitMonoLibrary();
	void TerminateMonoLibrary();

	void LoadClassesInfo(MonoImage *mono_image);

public:

	void GetScripts(std::vector<ClassInfo*> &scripts);
	bool LoadScript(ClassInfo *script);

	void Init();
	void Terminate();	

	MonoDataType TypeConversion(MonoType* type);
	MonoDataType TypeConversion(MonoClassField* field);

	void CreateFieldInfo(ClassInfo *class_info, MonoClassField *field);
};


#endif // __MONOSCRIPTS_H__