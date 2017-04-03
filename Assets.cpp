#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleEditor.h"

#include "Assets.h"
#include "TextureImporter.h"
#include "MaterialCreatorWindow.h"
#include "RenderTexEditorWindow.h"

#include "SDL/include/SDL_filesystem.h"

#include <stack>
#include <algorithm>

Assets::Assets()
{
	Init();
	flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
}

Assets::~Assets()
{
	CleanUp();
}

void Assets::Draw()
{
	if (!active)
		return;

	ImGui::SetNextWindowSize(current_size);
	ImGui::SetNextWindowPos(current_position);

	//TODO: use flags
	ImGui::Begin("Assets", &active, flags);

	//Options
	if (ImGui::IsMouseHoveringWindow())
		if (ImGui::IsMouseClicked(1))
			ImGui::OpenPopup("GeneralOptions");
	

	ImGui::Text(current_dir->path.data());

	bool refresh_needed = false;

	//Back folder
	if (current_dir->parent != nullptr)
	{
		ImGui::Image((ImTextureID)folder_id, ImVec2(15, 15));
		ImGui::SameLine();

		if (ImGui::Selectable("../"))
			current_dir = current_dir->parent;	
	}	

	//Print folders
	std::vector<Directory*>::iterator dir = current_dir->directories.begin();
	for (dir; dir != current_dir->directories.end(); dir++)
	{
		ImGui::Image((ImTextureID)folder_id, ImVec2(15, 15));
		ImGui::SameLine();
		
		// Renaming option enabled
		if ((*dir) == dir_to_rename)   // I think Directory pointers are deleted so this could be useful...
		{
			static char new_folder_name[128];
			if (ImGui::InputText("", new_folder_name, 128, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				RenameFolder((*dir), new_folder_name);
				refresh_needed = true; // Can be changed by an event? CRZ

				// Disabling renaming option and cleaning new folder name
				dir_to_rename = nullptr;
				new_folder_name[0] = '\0';
			}				
		}
		// Showing folder name otherwise
		else if(ImGui::Selectable((*dir)->name.data()))
		{
			ImGui::OpenPopup("DirectoryOptions");
			dir_selected = (*dir);
		}
	}

	//Print files
	std::vector<AssetFile*>::iterator file = current_dir->files.begin();
	for (file; file != current_dir->files.end(); file++)
	{
		if ((*file)->type != FileType::FOLDER)
		{
			switch ((*file)->type)
			{
			case FileType::IMAGE:
				ImGui::Image((ImTextureID)file_id, ImVec2(15, 15)); 
				ImGui::SameLine();
				if (ImGui::Selectable((*file)->name.data()))
				{
					file_selected = (*file);
					ImGui::OpenPopup("FileImageOptions");
					//imgui open popup fileimageoptions TODO
				}
				break;
			case FileType::MESH:
				ImGui::Image((ImTextureID)mesh_id, ImVec2(15, 15));
				ImGui::SameLine();
				if (ImGui::Selectable((*file)->name.data()))
				{
					file_selected = (*file);
					ImGui::OpenPopup("FileMeshOptions");
				}
				break;
			case FileType::SCENE:
				ImGui::Image((ImTextureID)scene_id, ImVec2(15, 15));
				ImGui::SameLine();
				if (ImGui::Selectable((*file)->name.data()))
				{
					file_selected = (*file);
					ImGui::OpenPopup("FileSceneOptions");
				}
					break;
			case FileType::PREFAB:
				ImGui::Image((ImTextureID)prefab_id, ImVec2(15, 15));
				ImGui::SameLine();
				if (ImGui::Selectable((*file)->name.data()))
				{
					file_selected = (*file);
					ImGui::OpenPopup("FilePrefabOptions");
				}
				break;
			case FileType::VERTEX:
				ImGui::Image((ImTextureID)vertex_id, ImVec2(15, 15));
				ImGui::SameLine();
				if (ImGui::Selectable((*file)->name.data()))
				{
					file_selected = (*file);
					ImGui::OpenPopup("VertexFragmentOptions");
				}
				break;
			case FileType::FRAGMENT:
				ImGui::Image((ImTextureID)fragment_id, ImVec2(15, 15));
				ImGui::SameLine();
				if (ImGui::Selectable((*file)->name.data()))
				{
					file_selected = (*file);
					ImGui::OpenPopup("VertexFragmentOptions");
				}
				break;
			case FileType::MATERIAL:
				ImGui::Image((ImTextureID)material_id, ImVec2(15, 15));
				ImGui::SameLine();
				if (ImGui::Selectable((*file)->name.data()))
				{
					file_selected = (*file);
					ImGui::OpenPopup("MaterialOptions");
				}
				break;
			case FileType::RENDER_TEXTURE:
				ImGui::Image((ImTextureID)file_id, ImVec2(15, 15));
				ImGui::SameLine();
				if (ImGui::Selectable((*file)->name.data()))
				{
					file_selected = (*file);
					ImGui::OpenPopup("RenderTextureOptions");
				}
				break;
			case FileType::SOUNDBANK:
				ImGui::Image((ImTextureID)soundbank_id, ImVec2(15, 15));
				ImGui::SameLine();
				if (ImGui::Selectable((*file)->name.data()))
				{
					file_selected = (*file);
					ImGui::OpenPopup("SoundbankOptions");
				}
				break;
			}			
		}
	}

	if (refresh_needed)
		Refresh();

	//PopUps File type options --------------------------------------------------

	DirectoryOptions();
	MeshFileOptions();
	ImageFileOptions();
	SceneFileOptions();
	PrefabFileOptions();
	MaterialFileOptions();
	RenderTextureOptions();
	GeneralOptions();
	VertexFragmentOptions();
	SoundbankOptions();

	ImGui::End();
}

string Assets::CurrentDirectory() const
{
	return current_dir->path;
}

string Assets::CurrentLibraryDirectory() const
{
	return current_dir->library_path;
}

string Assets::FindLibraryDirectory(const string & assets_dir) const
{
	Directory* dir = FindDirectory(assets_dir);
	return dir->library_path;
}

void Assets::Init()
{
	//start active
	active = true;

	root = new Directory();
	root->path = ASSETS_FOLDER;
	root->name = "Assets";
	root->library_path = LIBRARY_FOLDER;
	FillDirectoriesRecursive(root);

	current_dir = root;

	folder_id = TextureImporter::LoadSimpleFile("Resources/folder.dds"); 
	file_id = TextureImporter::LoadSimpleFile("Resources/file.dds");
	mesh_id = TextureImporter::LoadSimpleFile("Resources/mesh.dds");
	scene_id = TextureImporter::LoadSimpleFile("Resources/scene.dds");
	prefab_id = TextureImporter::LoadSimpleFile("Resources/prefab.dds");
	vertex_id = TextureImporter::LoadSimpleFile("Resources/vertex.dds");
	fragment_id = TextureImporter::LoadSimpleFile("Resources/fragment.dds");
	material_id = TextureImporter::LoadSimpleFile("Resources/material.dds");
	soundbank_id = TextureImporter::LoadSimpleFile("Resources/soundbank.dds");
}

void Assets::FillDirectoriesRecursive(Directory* root_dir)
{
	vector<string> folders;
	vector<string> files;
	App->file_system->GetFilesAndDirectories(root_dir->path.data(), folders, files, true);

	for (vector<string>::iterator file = files.begin(); file != files.end(); ++file)
	{
		char* buffer = nullptr;
		uint size = App->file_system->Load((root_dir->path + (*file)).data(), &buffer);
		if (size == 0)
		{
			LOG("[ERROR] Loading failure Meta file: %s", (root_dir->path + (*file)).data());
			App->editor->DisplayWarning(WarningType::W_ERROR, "Loading failure on Meta file: %s", (root_dir->path + (*file)).data());

			if (buffer)
				delete[] buffer;
			return;
		}
		Data meta(buffer);

		FileType type = static_cast<FileType>(meta.GetInt("Type"));

		if (type == FOLDER)
		{
			Directory* dir = new Directory();
			string folder_name = (*file).substr(0, (*file).length() - 5); //Substract extension (.meta)
			dir->path = root_dir->path + folder_name + "/";
			dir->name = folder_name;
			dir->parent = root_dir;
			dir->library_path = root_dir->library_path + std::to_string(meta.GetUInt("UUID")) + "/";
			root_dir->directories.push_back(dir);

			FillDirectoriesRecursive(dir);
		}
		else
		{
			AssetFile* a_file = new AssetFile();
			a_file->type = type;
			a_file->name = (*file).substr(0, (*file).length() - 5); //Substract extension (.meta)
			a_file->file_path = root_dir->path + (*file);
			a_file->uuid = meta.GetUInt("UUID");
			a_file->content_path = meta.GetString("library_path");
			a_file->original_file = meta.GetString("original_file");
			a_file->time_mod = App->file_system->GetLastModificationTime(a_file->original_file.data());
			a_file->directory = root_dir;
			root_dir->files.push_back(a_file);
		}
		delete[] buffer;
	}
}

bool Assets::IsMeshExtension(const std::string& fn)const
{
	string extension = fn.substr(fn.find_last_of(".") + 1);

	//TODO: improve this approach
	char* mesh_ext[] = { "fbx", "FBX", "obj", "OBJ" };

	for (int i = 0; i < 4; i++)
	{
		if (extension.compare(mesh_ext[i]) == 0)
			return true;
	}

	return false;
}

void Assets::CleanUp()
{
	file_selected = nullptr;
	dir_selected = nullptr;
	DeleteDirectoriesRecursive(root);
}

void Assets::RenameFolder(Directory *dir_to_rename, const char *new_folder_name) const
{
	// Renaming Windows Explorer Window
	string src_folder = "." + dir_to_rename->parent->path + dir_to_rename->name;  // Old name directory
	string dest_folder = "." + dir_to_rename->parent->path + new_folder_name;	  // New name directory
	MoveFile(src_folder.c_str(), dest_folder.c_str());

	// Deleting old meta file and adapting each file and directory meta file recursively from this directory
	UpdateFoldersMetaInfo(dir_to_rename, dir_to_rename->name.c_str(), new_folder_name);
}

void Assets::UpdateFoldersMetaInfo(Directory *curr_dir, string old_folder_name, string new_folder_name) const
{
	// Changing folder meta file with new folder name
	string meta_file = curr_dir->name + ".meta";
	App->resource_manager->NameFolderUpdate(meta_file, curr_dir->parent->path, old_folder_name, new_folder_name);

	// Changing old values for the new ones on Directory struct
	//curr_dir->name = new_folder_name;
	
	size_t pos = curr_dir->path.find(old_folder_name);
	string dir_path = curr_dir->path;
	dir_path.replace(pos, old_folder_name.length(), new_folder_name);
	
	std::vector<AssetFile*>::iterator it_file = curr_dir->files.begin();
	for (it_file; it_file != curr_dir->files.end(); ++it_file)
	{
		// Changing meta file with new folder name on each file inside this curr_dir
		meta_file = (*it_file)->name + ".meta";
		App->resource_manager->NameFolderUpdate(meta_file, dir_path, old_folder_name, new_folder_name, true);
	}

	std::vector<Directory*>::iterator it_dir = curr_dir->directories.begin();
	for (; it_dir != curr_dir->directories.end(); it_dir++)
	{
		UpdateFoldersMetaInfo((*it_dir), old_folder_name, new_folder_name); // Recursive iteration for deeper directory
	}	

}

void Assets::DeleteDirectoriesRecursive(Directory* root_dir, bool keep_root)
{
	vector<Directory*>::iterator dir = root_dir->directories.begin();

	for (dir; dir != root_dir->directories.end(); dir++)
	{
		DeleteDirectoriesRecursive((*dir));
	}

	root_dir->directories.clear();
	
	vector<AssetFile*>::iterator file = root_dir->files.begin();
	for (file; file != root_dir->files.end(); ++file)
		delete *file;
	root_dir->files.clear();

	if(!keep_root)
		delete root_dir;

}

Directory * Assets::FindDirectory(const string & dir)const
{
	if (dir == root->path)
	{
		return root;
	}

	string dir_part;
	vector<string> dir_splitted;
	dir_part = root->path;
	while (dir_part != dir)
	{
		string tmp = dir.substr(dir_part.length(), dir.length());
		size_t pos = tmp.find_first_of("/\\");
		if (pos != string::npos)
		{
			dir_part.append(tmp, 0, pos+1);
			dir_splitted.push_back(dir_part);
		}
		else
			break;
	}

	Directory* found = root;
	bool path_found = false;
	for (vector<string>::iterator split = dir_splitted.begin(); split != dir_splitted.end(); ++split)
	{
		path_found = false;
		for (vector<Directory*>::iterator directory = found->directories.begin(); directory != found->directories.end(); ++directory)
		{
			if ((*directory)->path == (*split))
			{
				found = (*directory);
				path_found = true;
				break;
			}
		}
		if (!path_found)
			break;
	}

	return (path_found) ? found : nullptr;
}

AssetFile* Assets::FindAssetFile(const string & file)
{
	return FindAssetFileRecursive(file, root);
}

string Assets::FindAssetFileFromLibrary(const string & lib_path) const
{
	string ret;

	if (root != nullptr)
	{
		stack<Directory*> stack;
		stack.push(root);
		Directory* item = nullptr;
		bool found = false;
		while (!found && !stack.empty())
		{
			item = stack.top();
			stack.pop();

			for (vector<AssetFile*>::const_iterator it = item->files.begin(); it != item->files.end(); ++it)
			{
				if ((*it)->content_path.compare(lib_path) == 0)
				{
					found = true;
					ret = (*it)->original_file;
					break;
				}
			}

			if (!found)
				for (vector<Directory*>::const_iterator it = item->directories.begin(); it != item->directories.end(); ++it)
					stack.push((*it));
		}
	}

	return ret;
}

AssetFile* Assets::FindAssetFileRecursive(const string& file, Directory* directory)
{
	for (vector<AssetFile*>::iterator asset = directory->files.begin(); asset != directory->files.end(); ++asset)
	{
		if ((*asset)->original_file == file)
		{
			return *asset;
		}
	}

	for (vector<Directory*>::iterator dir = directory->directories.begin(); dir != directory->directories.end(); ++dir)
	{
		AssetFile* ret = FindAssetFileRecursive(file, *dir);
		if (ret != nullptr)
		{
			return ret;
		}
	}
	return nullptr;
}

bool Assets::IsSceneExtension(const std::string& file_name) const
{
	string extension = file_name.substr(file_name.find_last_of(".") + 1);

	const char* ext = "ezx";

	return (extension.compare(ext) == 0) ? true : false;
}

void Assets::Refresh()
{
	string file, dir, to_rename, current;
	file = (file_selected) ? file_selected->original_file : "";
	dir = (dir_selected) ? dir_selected->path : "";
	to_rename = (dir_to_rename) ? dir_to_rename->path : "";
	current = (current_dir) ? current_dir->path : "";	

	// Deleting and creating directory tree
	DeleteDirectoriesRecursive(root, true);
	FillDirectoriesRecursive(root);
	
	if (file.length() > 0)
		file_selected = FindAssetFile(file);
	if (dir.length() > 0)
		dir_selected = FindDirectory(dir);
	if (to_rename.length() > 0)
		dir_to_rename = FindDirectory(to_rename);

	current_dir = nullptr;
	if (current.length() > 0)
		current_dir = FindDirectory(current);
	if(current_dir == nullptr)
		current_dir = root;
}

void Assets::OpenInExplorer(const std::string* file_name)const
{
	char* base_path = SDL_GetBasePath();
	std::string base = base_path;
	//Remove /Debug/ and replace it for/Game/
#if _DEBUG
	base.erase(base.length() - 7, base.length());
	base.append("/Game/");
#endif
	
	if (file_name)
		base += *file_name;
	else
		base += current_dir->path;
	ShellExecute(NULL, "open", base.data(), NULL, NULL, SW_SHOWNORMAL);
	SDL_free(base_path);
}

void Assets::GeneralOptions()
{
	if (ImGui::BeginPopup("GeneralOptions"))
	{
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("Folder"))
			{
				string tmp_new_folder = current_dir->path + "New_folder";
				App->file_system->GenerateDirectory(tmp_new_folder.c_str());
				string tmp_lib_path = current_dir->library_path;
				App->resource_manager->CreateFolder(tmp_new_folder.c_str(), tmp_lib_path);
			}
			if (ImGui::MenuItem("Render Texture"))
			{
				App->resource_manager->CreateRenderTexture(current_dir->path, current_dir->library_path);
			}
			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}
}

void Assets::MeshFileOptions()
{
	if (ImGui::BeginPopup("FileMeshOptions"))
	{
		if (ImGui::Selectable("Load to scene"))
		{
			App->resource_manager->LoadFile(file_selected->content_path, MESH);
		}

		if (ImGui::Selectable("Remove"))
		{
			DeleteAssetFile(file_selected);
		}

		if (ImGui::Selectable("Open"))
		{
			OpenInExplorer(&file_selected->file_path);
		}

		if (ImGui::Selectable("Refresh"))
		{
			Refresh();
		}
		if (ImGui::Selectable("Open in Explorer"))
		{
			OpenInExplorer();
		}

		ImGui::EndPopup();
	}
}

void Assets::ImageFileOptions()
{
	if (ImGui::BeginPopup("FileImageOptions"))
	{
		if (ImGui::Selectable("Remove"))
		{
			DeleteAssetFile(file_selected);
		}
		ImGui::EndPopup();
	}
}
void Assets::SceneFileOptions()
{
	if (ImGui::BeginPopup("FileSceneOptions"))
	{
		if (ImGui::Selectable("Open scene"))
		{
			App->resource_manager->LoadScene(file_selected->content_path.data()); 
		}
		if (ImGui::Selectable("Remove"))
		{
			DeleteAssetFile(file_selected);
		}
		ImGui::EndPopup();
	}
}

void Assets::DirectoryOptions()
{
	if (ImGui::BeginPopup("DirectoryOptions"))
	{
		if (ImGui::Selectable("Open"))
		{
			current_dir = dir_selected;
			dir_selected = nullptr;
		}
		if (ImGui::Selectable("Rename"))
		{			
			dir_to_rename = dir_selected;
			dir_selected = nullptr;
		}
		if (ImGui::Selectable("Remove"))
		{
			DeleteAssetDirectory(dir_selected);
			dir_selected = nullptr;
		}
		ImGui::EndPopup();		
	}
}

void Assets::PrefabFileOptions()
{
	if (ImGui::BeginPopup("FilePrefabOptions"))
	{
		if (ImGui::Selectable("Load to scene"))
		{
			App->resource_manager->LoadFile(file_selected->content_path, PREFAB);
		}

		if (ImGui::Selectable("Remove"))
		{
			DeleteAssetFile(file_selected);
		}

		if (ImGui::Selectable("Open"))
		{
			OpenInExplorer(&file_selected->file_path);
		}

		if (ImGui::Selectable("Refresh"))
		{
			Refresh();
		}
		if (ImGui::Selectable("Open in Explorer"))
		{
			OpenInExplorer();
		}

		ImGui::EndPopup();
	}
}

void Assets::MaterialFileOptions()
{
	if (ImGui::BeginPopup("MaterialOptions"))
	{
		if (ImGui::Selectable("Edit"))
		{
			App->editor->material_creator_win->SetActive(true);
			App->editor->material_creator_win->LoadToEdit(file_selected->content_path.data());
		}

		if (ImGui::Selectable("Remove"))
		{
			DeleteAssetFile(file_selected);
		}

		ImGui::EndPopup();
	}
}

void Assets::RenderTextureOptions()
{
	if (ImGui::BeginPopup("RenderTextureOptions"))
	{
		if (ImGui::Selectable("Edit"))
		{
			App->editor->rendertex_win->SetActive(true);
			App->editor->rendertex_win->LoadToEdit(file_selected->content_path.data());
		}

		if (ImGui::Selectable("Remove"))
		{
			DeleteAssetFile(file_selected);
		}

		ImGui::EndPopup();
	}
}

void Assets::VertexFragmentOptions()
{
	if (ImGui::BeginPopup("VertexFragmentOptions"))
	{
		if (ImGui::MenuItem("Edit"))
		{
			OpenInExplorer(&file_selected->original_file);
		}

		if (ImGui::Selectable("Remove"))
		{
			DeleteAssetFile(file_selected);
		}

		ImGui::EndPopup();
	}
}

void Assets::SoundbankOptions()
{
	if (ImGui::BeginPopup("SoundbankOptions"))
	{
		if (ImGui::Selectable("Remove")) DeleteAssetFile(file_selected);
		if (ImGui::Selectable("Refresh")) Refresh();
		if (ImGui::Selectable("Open in Explorer")) OpenInExplorer();

		ImGui::EndPopup();
	}
}

void Assets::DeleteAssetDirectory(Directory * directory)
{
	if (directory == nullptr)
		return;
	App->file_system->Delete(directory->library_path);
	App->file_system->Delete(directory->path);
	string meta_file = directory->parent->path + directory->name + ".meta";
	App->file_system->Delete(meta_file);

	Directory* parent = directory->parent;
	parent->directories.erase(std::remove(parent->directories.begin(), parent->directories.end(), directory), parent->directories.end());
	delete directory;
	directory = nullptr;
}

void Assets::DeleteAssetFile(AssetFile * file)
{
	if (file == nullptr)
		return;
	if (file == file_selected)
		file_selected = nullptr;
	//TODO: check if the file is being used
	string library_folder = file->content_path.substr(0, file->content_path.find_last_of("\//"));
	
	App->file_system->Delete(library_folder.data());
	App->file_system->Delete(file->file_path.data());
	App->file_system->Delete(file->original_file.data());


	Directory* dir = file->directory;
	dir->files.erase(std::remove(dir->files.begin(), dir->files.end(), file), dir->files.end());
	delete file;
	file = nullptr;
}

void Assets::DeleteMetaAndLibraryFile(AssetFile * file)
{
	if (file == nullptr)
		return;

	string library_folder = file->content_path.substr(0, file->content_path.find_last_of("\//"));

	App->file_system->Delete(library_folder.data());
	App->file_system->Delete(file->file_path.data());
}

void Assets::GetAllFilesByType(FileType type, vector<string>& result)const
{
	if (root == nullptr)
		return;

	stack<Directory*> stack;
	Directory* current = root;

	while (current != nullptr || stack.size() != 0)
	{
		if (current)
		{
			for (vector<AssetFile*>::iterator file = current->files.begin(); file != current->files.end(); ++file)
				if ((*file)->type == type)
					result.push_back((*file)->original_file);

			for (vector<Directory*>::iterator dir = current->directories.begin(); dir != current->directories.end(); ++dir)
				stack.push(*dir);
			current = nullptr;
		}
		else
		{
			current = stack.top();
			stack.pop();
		}
	}
}
