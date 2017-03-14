#include "FieldInfo.h"

#include "MonoLibraries.h"

#include "ClassInfo.h"

FieldInfo::FieldInfo(ClassInfo *_class)
{
	parent_class = _class;
}

const char *FieldInfo::FieldInfo::GetName() const
{
	return mono_field_get_name(mono_field);
}

MonoDataType FieldInfo::FieldInfo::GetType() const
{
	return type;
}

void FieldInfo::GetValue(int *value) const
{
	mono_field_get_value(parent_class->instanced_class, mono_field, (void*)value);
}

void FieldInfo::GetValue(float *value) const
{
	mono_field_get_value(parent_class->instanced_class, mono_field, (void*)value);
}

void FieldInfo::SetValue(int *value)
{
	mono_field_set_value(parent_class->instanced_class, mono_field, (void*)value);
}