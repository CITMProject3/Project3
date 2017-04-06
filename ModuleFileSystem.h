#ifndef __MODULEFILESYSTEM_H__
#define __MODULEFILESYSTEM_H__

#include "Module.h"

struct SDL_RWops;
int close_sdl_rwops(SDL_RWops *rw);

struct MD5;

class ModuleFileSystem : public Module
{
public:

	ModuleFileSystem(const char* name, bool start_enabled = true);

	// Destructor
	virtual ~ModuleFileSystem();

	bool Init(Data& config);

	// Called before quitting
	bool CleanUp();

	// Utility functions
	bool AddPath(const char* path_or_zip, const char* mount_point = nullptr);
	bool Exists(const char* file) const;
	bool IsDirectory(const char* file) const;
	bool IsDirectoryOutside(const char* file)const;
	const char* GetSaveDirectory() const
	{
		return "save/";
	}

	// Open for Read/Write
	unsigned int Load(const char* file, char** buffer) const;
	SDL_RWops* Load(const char* file) const;

	unsigned int Save(const char* file, const void* buffer, unsigned int size) const;
	//File = filename without extension. Path = Directory to save. Extension = extension to save without the "."
	bool SaveUnique(const char* file, const void* buffer, unsigned int size, const char* path, const char* extension,std::string& output_name);
	bool Save(const char* file, const void* buffer, unsigned int size, const char* path, const char* extension, std::string& output_name);

	bool GetEnumerateFiles(const char* dir, std::vector<std::string>& buffer);
	void GetFilesAndDirectories(const char* dir, std::vector<std::string>& folders, std::vector<std::string>& files, bool only_meta_files = false)const;
	void GetFilesAndDirectoriesOutside(const char* dir, std::vector<std::string>& folders, std::vector<std::string>& files);
	bool CopyFromOutsideFile(const char* from_path, const char* to_path)const;

	std::string GetNameFromPath(const std::string& path)const;
	double GetLastModificationTime(const char* file_path)const;
	const char *GetRealPath(const std::string &virtual_path) const;
	bool GenerateDirectory(const char* path)const;
	bool Delete(std::string filename)const;
	bool DuplicateFile(const char* src, const char* dst)const;

	char* GetMD5(const char* path)const;

private:

	void SearchResourceFolders();

private:
	MD5* md5;

};

#endif //__MODULEFILESYSTEM_H__