#ifndef __DATA_H__
#define __DATA_H__

#include "JSON\parson.h"
#include "MathGeoLib\include\MathGeoLib.h"

//Wrapper for Parson (JSON) library

class Data
{
public:
	Data();
	Data(const char* filename);
	Data(JSON_Object* root);
	~Data();

	//Append
	bool AppendArray(const char* name);
	bool AppendArrayValue(const Data& data);
	bool AppendString(const char* name, const char* string);
	bool AppendBool(const char* name, bool value);
	bool AppendUInt(const char* name, unsigned int value);
	bool AppendInt(const char* name, int value);
	bool AppendMatrix(const char* name, const math::float4x4& matrix);
	bool AppendFloat(const char* name, float value);
	bool AppendFloat3(const char* name,const float* value);
	bool AppendDouble(const char* name, double value);
	Data AppendJObject(const char* name);

	//Gets
	Data GetJObject(const char* name)const;
	const char* GetString(const char* name)const;
	int GetInt(const char* name)const;
	Data GetArray(const char* name,unsigned int index)const;
	size_t GetArraySize(const char* name)const;
	bool GetBool(const char* name)const;
	unsigned int GetUInt(const char* name)const;
	float4x4 GetMatrix(const char* name)const;
	float GetFloat(const char* name)const;
	float3 GetFloat3(const char* name)const;
	double GetDouble(const char* name)const;

	//Save
	size_t Serialize(char** buffer);

	bool IsNull()const;

private:
	JSON_Value* root_value = nullptr;
	JSON_Object* root = nullptr;
	JSON_Array* array = nullptr;

};

#endif // !__DATA_H__
