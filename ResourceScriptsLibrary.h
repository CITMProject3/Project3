#ifndef __RESOURCESCRIPTSLIBRARY_H__
#define __RESOURCESCRIPTSLIBRARY_H__

#include "ResourceFile.h"
#include "Globals.h"
#include <map>
#include <string>
#include <vector>

using namespace std;
class Application;
typedef void(*f_GetScriptNames)(Application* engine_app); 
typedef void(*f_GetPublics)(map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_float, map<const char*, bool>* public_bools);

class ResourceScriptsLibrary : public ResourceFile
{
public:
	ResourceScriptsLibrary(ResourceFileType type, const std::string& file_path, unsigned int uuid);
	~ResourceScriptsLibrary();

	void GetPublicVars(const char* script_name, map<const char*, string>* public_chars, map<const char*, int>* public_ints, map<const char*, float>* public_floats, map<const char*, bool>* public_bools);

	HINSTANCE lib;
	bool finded_script_names;
	vector<const char*> script_names;
private:

	void LoadInMemory();
	void UnloadInMemory();

	void LoadScriptNames();
	void LoadScriptPublicVars();

	DWORD last_error = 0;

	map<const char*, map<const char*, string>> public_chars;
	map<const char*, map<const char*, int>> public_ints;
	map<const char*, map<const char*, float>> public_floats;
	map<const char*, map<const char*, bool>> public_bools;

};

#endif // !__RESOURCESCRIPTSLIBRARY_H__