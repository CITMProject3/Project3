#ifndef __FIELDINFO_H__
#define __FIELDINFO_H__

#include <string>

class ClassInfo;

typedef struct _MonoClassField MonoClassField;
typedef struct _MonoString MonoString;
enum MonoDataType;

class FieldInfo
{
public:

	FieldInfo(ClassInfo *_class);

	ClassInfo *parent_class = nullptr;
	MonoClassField *mono_field = nullptr;

	MonoDataType type;

	// Tools
	const char *GetName() const;
	MonoDataType GetType() const;

	void GetValue(int *value) const;
	void GetValue(float *value) const;

	void SetValue(int *value);

};

#endif __FIELDINFO_H__
