#include "ModuleFileSystem.h"

#include "PhysFS/include/physfs.h"
#include "SDL/include/SDL_filesystem.h"
#include "SDL/include/SDL_stdinc.h"
#include "SDL/include/SDL_rwops.h"

#include "md5.h"

#pragma comment( lib, "PhysFS/libx86/physfs.lib" )

ModuleFileSystem::ModuleFileSystem(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	// need to be created before Awake so other modules can use it
	char* base_path = SDL_GetBasePath();
	PHYSFS_init(base_path);
	SDL_free(base_path);

	// By default we include executable's own directory
	AddPath(".");
	AddPath("Resources.zip");

	md5 = new MD5();
}

// Destructor
ModuleFileSystem::~ModuleFileSystem()
{
	delete md5;
	PHYSFS_deinit();
}

// Called before render is available
bool ModuleFileSystem::Init(Data& config)
{
	LOG("Loading File System");
	bool ret = true;

	// Add all paths in configuration in order
	//AddPath() manually

	if (PHYSFS_setWriteDir(".") == 0)
	{
		LOG("File System error while creating write dir: %s\n", PHYSFS_getLastError());
	}
	//Search Assets and Library folders
	SearchResourceFolders();

	return ret;
}

// Called before quitting
bool ModuleFileSystem::CleanUp()
{
	//LOG("Freeing File System subsystem");
	return true;
}

// Add a new zip file or folder
bool ModuleFileSystem::AddPath(const char* path_or_zip, const char* mount_point)
{
	bool ret = false;

	if (PHYSFS_mount(path_or_zip, mount_point, 1) == 0)
	{
		LOG("File System error while adding a path or zip(%s): %s\n", path_or_zip, PHYSFS_getLastError());
	}	
	else
		ret = true;

	return ret;
}

// Check if a file exists
bool ModuleFileSystem::Exists(const char* file) const
{
	return PHYSFS_exists(file) != 0;
}

// Check if a file is a directory
bool ModuleFileSystem::IsDirectory(const char* file) const
{
	return PHYSFS_isDirectory(file) != 0;
}

bool ModuleFileSystem::IsDirectoryOutside(const char * file) const
{
	return (GetFileAttributes((LPCTSTR)file) == FILE_ATTRIBUTE_DIRECTORY) ? true : false;
}

// Read a whole file and put it in a new buffer
unsigned int ModuleFileSystem::Load(const char* file, char** buffer) const
{
	unsigned int ret = 0;

	PHYSFS_file* fs_file = PHYSFS_openRead(file);

	if (fs_file != NULL)
	{
		PHYSFS_sint64 size = PHYSFS_fileLength(fs_file);

		if (size >= 0)
		{
			*buffer = new char[(uint)size];
			PHYSFS_sint64 readed = PHYSFS_read(fs_file, *buffer, 1, (PHYSFS_sint32)size);
			if (readed != size)
			{
				LOG("File System error while reading from file %s: %s\n", file, PHYSFS_getLastError());
				if (buffer)
					delete[] buffer;
			}
			else
				ret = (uint)readed;
		}

		if (PHYSFS_close(fs_file) == 0)
			LOG("File System error while closing file %s: %s\n", file, PHYSFS_getLastError());
	}
	else
		LOG("File System error while opening file to load %s: %s\n", file, PHYSFS_getLastError());

	return ret;
}

// Read a whole file and put it in a new buffer
SDL_RWops* ModuleFileSystem::Load(const char* file) const
{
	char* buffer;
	int size = Load(file, &buffer);

	if (size > 0)
	{
		SDL_RWops* r = SDL_RWFromConstMem(buffer, size);
		if (r != NULL)
			r->close = close_sdl_rwops;

		return r;
	}
	else
		return NULL;
}

int close_sdl_rwops(SDL_RWops *rw)
{
	if (rw->hidden.mem.base)
		delete rw->hidden.mem.base;
	SDL_FreeRW(rw);
	return 0;
}

// Save a whole buffer to disk
unsigned int ModuleFileSystem::Save(const char* file, const void* buffer, unsigned int size) const
{
	unsigned int ret = 0;
	
	PHYSFS_file* fs_file = PHYSFS_openWrite(file);

	if (fs_file != NULL)
	{
		PHYSFS_sint64 written = PHYSFS_write(fs_file, (const void*)buffer, 1, size);
		if (written != size)
		{
			LOG("File System error while writing to file %s: %s\n", file, PHYSFS_getLastError());
		}
		else
			ret = (uint)written;

		if (PHYSFS_close(fs_file) == 0)
			LOG("File System error while closing file %s: %s\n", file, PHYSFS_getLastError());
	}
	else
		LOG("File System error while opening file to save %s: %s\n", file, PHYSFS_getLastError());

	return ret;
}

bool ModuleFileSystem::SaveUnique(const char * file, const void* buffer, unsigned int size, const char * path, const char * extension,std::string& output_name)
{ //TODO: check if the file already exists
	
	char name_to_save[50];
	sprintf_s(name_to_save, 50, "%s.%s", file, extension);

	std::vector<std::string> files_in_path;
	GetEnumerateFiles(path, files_in_path);

	int copies = 0;

	bool name_unique = false;

	while (name_unique == false)
	{
		name_unique = true;

		std::vector<std::string>::iterator name_file = files_in_path.begin();
		for (name_file; name_file != files_in_path.end(); name_file++)
		{
			
			if ((*name_file).compare(name_to_save) == 0)
			{
				//add +1 to the file number
				++copies;
				sprintf_s(name_to_save, 50, "%s%d.%s", file, copies, extension);
				name_unique = false;
				break;
			}
		}	
	}

	char name[300];
	sprintf_s(name, 300, "%s%s", path, name_to_save);

	if (Save(name, buffer, size) > 0)
	{
		output_name = name;
		return true;
	}
	else
		return false;
}

bool ModuleFileSystem::Save(const char * file, const void * buffer, unsigned int size, const char * path, const char * extension, std::string & output_name)
{
	char name_to_save[50];
	sprintf_s(name_to_save, 50, "%s.%s", file, extension);

	char name[300];
	sprintf_s(name, 300, "%s%s", path, name_to_save);

	if (Save(name, buffer, size) > 0)
	{
		output_name = name;
		return true;
	}
	else
		return false;
}

bool ModuleFileSystem::GetEnumerateFiles(const char * dir, std::vector<std::string>& buffer)
{
	char** ef =PHYSFS_enumerateFiles(dir);

	for (char** i = ef; *i != NULL; i++)
	{
		buffer.push_back(*i);
	}

	PHYSFS_freeList(ef);

	return (ef != NULL) ? true : false;
}

void ModuleFileSystem::GetFilesAndDirectories(const char * dir, std::vector<std::string>& folders, std::vector<std::string>& files, bool only_meta_files) const
{
	char** ef = PHYSFS_enumerateFiles(dir);

	std::string directory(dir);
	std::string file_name;
	for (char**i = ef; *i != NULL; i++)
	{
		if (PHYSFS_isDirectory((directory+(*i)).c_str()))
		{
			folders.push_back(*i);
		}
		else
		{
			if(!only_meta_files)
				files.push_back(*i);
			else
			{
				file_name = (*i);
				if (file_name.length() >= 6)
				{
					file_name = file_name.substr(file_name.length() - 4, 4);
					if (file_name.compare("meta") == 0)
						files.push_back(*i);
				}	
			}
		}
	}

	PHYSFS_freeList(ef);
}

void ModuleFileSystem::GetFilesAndDirectoriesOutside(const char * dir, std::vector<std::string>& folders, std::vector<std::string>& files)
{
	//Only for Windows
	std::string directory = dir;
	std::string search_path = directory + "/*.*";
	WIN32_FIND_DATA find_data;
	HANDLE handle_find = ::FindFirstFile(search_path.data(), &find_data);
	if (handle_find != INVALID_HANDLE_VALUE)
	{
		while (::FindNextFile(handle_find, &find_data))
		{
			if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				files.push_back(find_data.cFileName);
			else
			{
				if(strcmp("..", find_data.cFileName) != 0)
					folders.push_back(find_data.cFileName);
			}
				
		}
		::FindClose(handle_find);
	}
}

bool ModuleFileSystem::CopyFromOutsideFile(const char * from_path, const char * to_path) const
{
	bool ret = false;

	FILE* file;
	fopen_s(&file, from_path, "rb"); //rb-> read in binary
	PHYSFS_file* fs_file = PHYSFS_openWrite(to_path);
	char buffer[8192];
	
	if (fs_file && file)
	{
		size_t read_ret;
		PHYSFS_sint64 written = 0;
		while( read_ret = fread_s(buffer, 8192, 1, 8192, file))
			written += PHYSFS_write(fs_file, buffer, 1, read_ret);

		ret = true;
	}
	else
	{
		LOG("File System error while copying %s", from_path);
	}
	if (file)
	{
		fclose(file);
	}
	if (fs_file)
	{
		PHYSFS_close(fs_file);
	}

	return ret;
}

std::string ModuleFileSystem::GetNameFromPath(const std::string & path) const
{
	//Note: we supose the given path is a file and not a directory. 
	size_t name = path.find_last_of("/\\");
	return path.substr(name + 1);
}

double ModuleFileSystem::GetLastModificationTime(const char * file_path) const
{
	return PHYSFS_getLastModTime(file_path);
}

const char *ModuleFileSystem::GetRealPath(const std::string &virtual_path) const
{
	return PHYSFS_getRealDir(virtual_path.c_str());
}

bool ModuleFileSystem::GenerateDirectory(const char * path) const
{
	return PHYSFS_mkdir(path) != 0 ? true : false;
}

bool ModuleFileSystem::Delete(std::string filename) const
{
	int ret; 
	std::vector<std::string> directories, files;
	if (IsDirectory(filename.data()))
	{
		GetFilesAndDirectories(filename.data(), directories, files);
		for (std::vector<std::string>::const_iterator directory = directories.begin(); directory != directories.end(); ++directory)
		{
			std::string directory_name = filename + *directory + "/";
			Delete(directory_name.data());
		}
			
		for (std::vector<std::string>::const_iterator file = files.begin(); file != files.end(); ++file)
			PHYSFS_delete((filename + '/' + (*file)).data());
	}
	ret = PHYSFS_delete(filename.data());
	
	return (ret) ? true : false;
}

bool ModuleFileSystem::DuplicateFile(const char * src, const char * dst) const
{
	char* buffer = nullptr;
	int size = Load(src, &buffer);

	if (size == -1)
	{
		if(buffer != nullptr)
			delete[] buffer;
		LOG("Couldn't duplicate %s file to %s", src, dst);
		return false;
	}
	
	int success = Save(dst, buffer, size);
	delete[] buffer;

	return (success != 0) ? true : false;
}

char* ModuleFileSystem::GetMD5(const char * path) const
{
	char* ret = nullptr;
	char* buffer = nullptr;

	int size = Load(path, &buffer);

	if (size != -1)
		ret = md5->digestMemory((BYTE*)buffer, size);
	else
		LOG("Couldn't create md5 for path %s", path);

	if (buffer)
		delete[] buffer;

	return ret;
}

void ModuleFileSystem::SearchResourceFolders()
{
	const char* folders[] = { ASSETS_FOLDER, LIBRARY_FOLDER}; // 2 folders

	for (int i = 0; i < 2; i++)
		if (PHYSFS_exists(folders[i]) == 0)
			PHYSFS_mkdir(folders[i]);
	
}