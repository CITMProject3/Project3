#include "Application.h"
#include "ModuleEditor.h"
#include "TextureImporter.h"
#include "ModuleFileSystem.h"
#include "ResourceFileTexture.h"
#include "Devil/include/il.h"
#include "Devil/include/ilut.h"
#pragma comment ( lib, "Devil/libx86/DevIL.lib" )
#pragma comment ( lib, "Devil/libx86/ILU.lib" )
#pragma comment ( lib, "Devil/libx86/ILUT.lib" )

bool TextureImporter::Import(const char* file, const char * path)
{
	bool ret = false;

	char* buffer = nullptr;
	unsigned int size = App->file_system->Load(path, &buffer);

	if (size > 0)
	{
		ILuint id;
		ilGenImages(1, &id);
		ilBindImage(id);

		if (ilLoadL(IL_TYPE_UNKNOWN, (const void*)buffer, size))
		{
			ILuint il_size;
			ILubyte *data;
			ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);
			il_size = ilSaveL(IL_DDS, NULL, 0);
			if (il_size > 0)
			{
				data = new ILubyte[il_size];
				if (ilSaveL(IL_DDS, data, il_size) > 0)
				{
					ret = App->file_system->Save(file, data, il_size) != 0 ? true : false;
				}
				delete[] data;
				ilDeleteImages(1, &id);
			}
		}
	}

	delete[] buffer;

	return ret;
}

bool TextureImporter::Load(ResourceFileTexture * res)
{
	bool ret = false;

	char* buffer = nullptr;
	unsigned int size = App->file_system->Load(res->GetFile(), &buffer);
	
	if (size > 0)
	{
		ILuint id;
		ilGenImages(1, &id);
		ilBindImage(id);
		if (ilLoadL(IL_DDS, (const void*)buffer, size))
		{		
			ILinfo info;
			iluGetImageInfo(&info);
			
			res->SetProperties(ilutGLBindTexImage(), info.Width, info.Height, info.Depth, info.NumMips, info.SizeOfData, info.Data);

			ilDeleteImages(1, &id);
			ret = true;
		}
	}
	else
	{
		LOG("[ERROR] Could not load texture %s", res->GetFile());
		App->editor->DisplayWarning(WarningType::W_ERROR, "Could not load texture %s", res->GetFile());
	}

	delete[] buffer;

	return ret;
}

int TextureImporter::LoadSimpleFile(const char * name)
{
	int ret = -1;

	char* buffer = nullptr;
	unsigned int size = App->file_system->Load(name, &buffer);

	if (size > 0)
	{
		ILuint id;
		ilGenImages(1, &id);
		ilBindImage(id);
		if (ilLoadL(IL_DDS, (const void*)buffer, size))
		{
			ret = ilutGLBindTexImage();
			ilDeleteImages(1, &id);			
		}
	}
	else
	{
		LOG("[ERROR] Could not load texture %s", name);
		App->editor->DisplayWarning(WarningType::W_ERROR, "Could not load texture %s", name);
	}

	delete[] buffer;

	return ret;
}

void TextureImporter::Unload(unsigned int id)
{
	ilDeleteImages(1, &id);
}
