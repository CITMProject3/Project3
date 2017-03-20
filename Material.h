#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <string>
#include <vector>

enum UniformType
{
	U_BOOL,
	U_INT,
	U_FLOAT,
	U_VEC2,
	U_VEC3,
	U_VEC4,
	U_MAT4X4,
	U_SAMPLER2D
};

struct Uniform
{
	~Uniform();
	std::string name;
	UniformType type;
	char* value = nullptr;
};

class Material
{
public:
	Material();
	~Material();

	void AddUniform(const std::string& name, UniformType type, char* value);
	bool Save(const char* path)const;
	void Load(const char* path);

private:
	void CleanUp();
	
public:
	std::string vertex_path;
	std::string fragment_path;
	std::vector<Uniform*> uniforms;
	uint uuid = 0;
	
	bool has_color = false;
};

#endif // !__MATERIAL_H__
