#include "ResourceScriptsLibrary.h"

#include "Application.h"
#include <string>

ResourceScriptsLibrary::ResourceScriptsLibrary(ResourceFileType type, const std::string& file_path, unsigned int uuid) : ResourceFile(type, file_path, uuid)
{
	finded_script_names = false;
}

ResourceScriptsLibrary::~ResourceScriptsLibrary()
{ }

void ResourceScriptsLibrary::LoadInMemory()
{
	string path = file_path.data();
	path.erase(0, 1);// erase the first '/'
	lib = LoadLibrary(path.c_str());
	if (lib != NULL)
	{
		LoadScriptNames();
	}
}

void ResourceScriptsLibrary::UnloadInMemory()
{
	FreeLibrary(lib);
}

void ResourceScriptsLibrary::LoadScriptNames()
{
	script_names.clear();

	if (f_GetScriptNames get_script_names = (f_GetScriptNames)GetProcAddress(lib, "GetScriptNames"))
	{
		finded_script_names = true;
		get_script_names(&script_names);
		LoadScriptPublicVars();
	}
	else
	{
		finded_script_names = false;
		last_error = GetLastError();

		if (last_error == 126)
		{
			LOG("Can't find script names function");
		}
		else
			LOG("Unknown error loading script names");
	}
}

void ResourceScriptsLibrary::LoadScriptPublicVars()
{
	for (int i = 0; i < script_names.size(); i++)
	{
		string update_path = script_names[i];
		update_path.append("_GetPublics");
		if (f_GetPublics getPublics = (f_GetPublics)GetProcAddress(lib, update_path.c_str()))
		{
			map<const char*, string> tmp_public_chars;
			map<const char*, int> tmp_public_ints;
			map<const char*, float> tmp_public_floats;
			map<const char*, bool> tmp_public_bools;
			getPublics(&tmp_public_chars, &tmp_public_ints, &tmp_public_floats, &tmp_public_bools);

			if (!tmp_public_chars.empty())
			{
				public_chars.insert(pair<const char*, map<const char*, string>>(script_names[i], tmp_public_chars));
			}
			if (!tmp_public_ints.empty())
			{
				public_ints.insert(pair<const char*, map<const char*, int>>(script_names[i], tmp_public_ints));
			}
			if (!tmp_public_floats.empty())
			{
				public_floats.insert(pair<const char*, map<const char*, float>>(script_names[i], tmp_public_floats));
			}
			if (!tmp_public_bools.empty())
			{
				public_bools.insert(pair<const char*, map<const char*, bool>>(script_names[i], tmp_public_bools));
			}
		}
	}
}

void ResourceScriptsLibrary::GetPublicVars(const char* script_name, map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_floats, map<const char*, bool>* public_bools)
{
	string str = script_name;
	for (map<const char*, map<const char*, string>>::iterator it = this->public_chars.begin(); it != this->public_chars.end(); it++)
	{
		if (str.compare((*it).first) == 0)
		{
			for(map<const char*, string>::iterator it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++)
				public_chars->insert(pair<const char*, string>((*it2).first, (*it2).second));
		}
	}
	for (map<const char*, map<const char*, int>>::iterator it = this->public_ints.begin(); it != this->public_ints.end(); it++)
	{
		if (str.compare((*it).first) == 0)
		{
			for (map<const char*, int>::iterator it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++)
				public_ints->insert(pair<const char*, int>((*it2).first, (*it2).second));
		}
	}
	for (map<const char*, map<const char*, float>>::iterator it = this->public_floats.begin(); it != this->public_floats.end(); it++)
	{
		if (str.compare((*it).first) == 0)
		{
			for (map<const char*, float>::iterator it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++)
				public_floats->insert(pair<const char*, float>((*it2).first, (*it2).second));
		}
	}
	for (map<const char*, map<const char*, bool>>::iterator it = this->public_bools.begin(); it != this->public_bools.end(); it++)
	{
		if (str.compare((*it).first) == 0)
		{
			for (map<const char*, bool>::iterator it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++)
				public_bools->insert(pair<const char*, bool>((*it2).first, (*it2).second));
		}
	}
}
/*if (!this->public_chars.empty())
	{
		for (map<const char*, string>::iterator it = this->public_chars.at(script_name).begin(); it != this->public_chars.at(script_name).end(); it++)
			public_chars->insert(pair<const char*, string>((*it).first, (*it).second));
	}

	if (!this->public_ints.empty())
	{
		for (map<const char*, int>::iterator it = this->public_ints.at(script_name).begin(); it != this->public_ints.at(script_name).end(); it++)
			public_ints->insert(pair<const char*, int>((*it).first, (*it).second));
	}

	if (!this->public_floats.empty())
	{
		for (map<const char*, float>::iterator it = this->public_floats.at(script_name).begin(); it != this->public_floats.at(script_name).end(); it++)
			public_floats->insert(pair<const char*, float>((*it).first, (*it).second));
	}

	if (!this->public_bools.empty())
	{
		for (map<const char*, bool>::iterator it = this->public_bools.at(script_name).begin(); it != this->public_bools.at(script_name).end(); it++)
			public_bools->insert(pair<const char*, bool>((*it).first, (*it).second));
	}*/