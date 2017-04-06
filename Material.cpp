#include "Application.h"
#include "Material.h"
#include "Data.h"
#include "ModuleFileSystem.h"

Material::Material()
{
}

Material::~Material()
{
	CleanUp();
}

void Material::AddUniform(const std::string& name, UniformType type, char* value)
{
	Uniform* uni = new Uniform();
	uni->name = name.data();
	uni->type = type;

	switch (type)
	{
	case U_BOOL:
		uni->value = new char[sizeof(bool)];
		memcpy(uni->value, value, sizeof(bool));
		break;
	case U_INT:
		uni->value = new char[sizeof(int)];
		memcpy(uni->value, value, sizeof(int));
		break;
	case U_FLOAT:
		uni->value = new char[sizeof(float)];
		memcpy(uni->value, value, sizeof(float));
		break;
	case U_VEC2:
		uni->value = new char[sizeof(float) * 2];
		memcpy(uni->value, value, sizeof(float) * 2);
		break;
	case U_VEC3:
		uni->value = new char[sizeof(float) * 3];
		memcpy(uni->value, value, sizeof(float) * 3);
		break;
	case U_VEC4:
		uni->value = new char[sizeof(float) * 4];
		memcpy(uni->value, value, sizeof(float) * 4);
		break;
	case U_MAT4X4:
		uni->value = new char[sizeof(float) * 16];
		memcpy(uni->value, value, sizeof(float) * 16);
		break;
	case U_SAMPLER2D:
		int size;
		memcpy(&size, value, sizeof(int));
		uni->value = new char[sizeof(int) + sizeof(char) * size];
		memcpy(uni->value, value, sizeof(int) + sizeof(char) * size);
		break;

	}

	uniforms.push_back(uni);
}

bool Material::Save(const char * path) const
{
	//vertex_path(string) - fragment_path(string) - Uniforms(any type)
	unsigned int header[3] =
	{
		vertex_path.size(),
		fragment_path.size(),
		uniforms.size()
	};

	unsigned int size = sizeof(header) + sizeof(char) * header[0] + sizeof(char) * header[1];

	for (std::vector<Uniform*>::const_iterator uni = uniforms.begin(); uni != uniforms.end(); ++uni)
	{
		size += sizeof(int); //name length
		size += (*uni)->name.size() * sizeof(char); //name
		size += sizeof(int); //type
		switch ((*uni)->type)
		{
		case U_BOOL:
			size += sizeof(bool);
			break;
		case U_INT:
			size += sizeof(int);
			break;
		case U_FLOAT:
			size += sizeof(float);
			break;
		case U_VEC2:
			size += sizeof(float) * 2;
			break;
		case U_VEC3:
			size += sizeof(float) * 3;
			break;
		case U_VEC4:
			size += sizeof(float) * 4;
			break;
		case U_MAT4X4:
			size += sizeof(float) * 16;
			break;
		case U_SAMPLER2D:
			int sampler_size;
			memcpy(&sampler_size, (*uni)->value, sizeof(int));
			size += sizeof(int) + sizeof(char) * sampler_size;
			break;
		}
	}

	char* data = new char[size];
	char* cursor = data;
	unsigned int bytes;

	//Header
	bytes = sizeof(header);
	memcpy(cursor, header, bytes);
	cursor += bytes;

	//Vertex_path
	bytes = sizeof(char) * header[0];
	memcpy(cursor, vertex_path.c_str(), bytes);
	cursor += bytes;

	//Fragment_path
	bytes = sizeof(char) * header[1];
	memcpy(cursor, fragment_path.c_str(), bytes);
	cursor += bytes;

	for (std::vector<Uniform*>::const_iterator uni = uniforms.begin(); uni != uniforms.end(); ++uni)
	{
		bytes = sizeof(int);
		int name_size = (*uni)->name.size();
		memcpy(cursor, &name_size, bytes);
		cursor += bytes;

		bytes = (*uni)->name.size() * sizeof(char); //name
		memcpy(cursor, (*uni)->name.c_str(), bytes);
		cursor += bytes;

		bytes = sizeof(int); //type
		memcpy(cursor, &(*uni)->type, bytes);
		cursor += bytes;

		switch ((*uni)->type)
		{
		case U_BOOL:
			bytes= sizeof(bool);
			break;
		case U_INT:
			bytes= sizeof(int);
			break;
		case U_FLOAT:
			bytes= sizeof(float);
			break;
		case U_VEC2:
			bytes= sizeof(float) * 2;
			break;
		case U_VEC3:
			bytes= sizeof(float) * 3;
			break;
		case U_VEC4:
			bytes = sizeof(float) * 4;
			break;
		case U_MAT4X4:
			bytes= sizeof(float) * 16;
			break;
		case U_SAMPLER2D:
			int sampler_size;
			memcpy(&sampler_size, (*uni)->value, sizeof(int));
			bytes = sizeof(int) + sizeof(char) * sampler_size;
			break;
		}
		memcpy(cursor, (*uni)->value, bytes);
		cursor += bytes;
	}

	bool ret = App->file_system->Save(path, data, size) != 0 ? true : false;
	delete[] data;

	return ret;
}

void Material::Load(const char * path)
{
	CleanUp();

	string s_path = path;
	size_t init = s_path.find_last_of("/\\") + 1;
	size_t end = s_path.find_last_of('.');
	s_path = s_path.substr(init, end - init);

	uuid = std::stoul(s_path);

	char* buffer = nullptr;

	if (App->file_system->Load(path, &buffer) != 0)
	{
		char* cursor = buffer;

		uint header[3];
		uint bytes = sizeof(header);
		memcpy(header, cursor, bytes);
		cursor += bytes;

		vertex_path.resize(header[0]);
		bytes = sizeof(char) * header[0];
		memcpy(vertex_path._Myptr(), cursor, bytes);
		cursor += bytes;

		fragment_path.resize(header[1]);
		bytes = sizeof(char) * header[1];
		memcpy(fragment_path._Myptr(), cursor, bytes);
		cursor += bytes;

		int num_uniforms = header[2];

		for (int i = 0; i < num_uniforms; i++)
		{
			Uniform* uniform = new Uniform();
			
			int name_size;
			bytes = sizeof(int);
			memcpy(&name_size, cursor, bytes);
			cursor += bytes;

			bytes = sizeof(char) * name_size;
			uniform->name.resize(bytes);
			memcpy(uniform->name._Myptr(), cursor, bytes);
			cursor += bytes;

			bytes = sizeof(int);
			memcpy(&uniform->type, cursor, bytes);
			cursor += bytes;
			
			switch (uniform->type)
			{
			case U_BOOL:
				bytes = sizeof(bool);
				break;
			case U_INT:
				bytes = sizeof(int);
				break;
			case U_FLOAT:
				bytes = sizeof(float);
				break;
			case U_VEC2:
				bytes = sizeof(float) * 2;
				break;
			case U_VEC3:
				bytes = sizeof(float) * 3;
				break;
			case U_VEC4:
				bytes = sizeof(float) * 4;
				break;
			case U_MAT4X4:
				bytes = sizeof(float) * 16;
				break;
			case U_SAMPLER2D:
				int sampler_size;
				memcpy(&sampler_size, cursor, sizeof(int));
				bytes = sizeof(int) + sizeof(char) * sampler_size;
				break;
			}
			uniform->value = new char[bytes];
			memcpy(uniform->value, cursor, bytes);
			cursor += bytes;
			uniforms.push_back(uniform);
		}
	}

	if (buffer)
		delete[] buffer;
}

void Material::CleanUp()
{
	for (std::vector<Uniform*>::iterator it = uniforms.begin(); it != uniforms.end(); ++it)
		delete *it;

	uniforms.clear();
	uuid = 0;
}

Uniform::~Uniform()
{
	if (value != nullptr)
	{
		delete[] value;
		value = nullptr;
	}
}
