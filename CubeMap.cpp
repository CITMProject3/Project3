#include "Application.h"
#include "CubeMap.h"
#include "Glew\include\glew.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "ResourceFileTexture.h"
#include "TextureImporter.h"
#include "Globals.h"
#include "ModuleFileSystem.h"

#include "Devil/include/il.h"
#include "Devil/include/ilut.h"
#pragma comment ( lib, "Devil/libx86/DevIL.lib" )
#pragma comment ( lib, "Devil/libx86/ILU.lib" )
#pragma comment ( lib, "Devil/libx86/ILUT.lib" )

CubeMap::CubeMap(const string & posx_filename, const string & negx_filename, const string & posy_filename, const string & negy_filename, const string & posz_filename, const string & negz_filename)
{
	textures_filenames.push_back(posx_filename);
	textures_filenames.push_back(negx_filename);
	textures_filenames.push_back(posy_filename);
	textures_filenames.push_back(negy_filename);
	textures_filenames.push_back(posz_filename);
	textures_filenames.push_back(negz_filename);
}

CubeMap::~CubeMap()
{}

bool CubeMap::Load()
{
	//TODO: do not hardcode this. Implement Devil's code inside the texture importer.
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	for (unsigned int i = 0; i < 6; i++)
	{
		char* buffer = nullptr;
		unsigned int size = App->file_system->Load(textures_filenames[i].data(), &buffer);

		if (size > 0)
		{
			ILuint il_id;
			ilGenImages(1, &il_id);
			ilBindImage(il_id);
			if (ilLoadL(IL_DDS, (const void*)buffer, size))
			{
				ILinfo info;
				iluGetImageInfo(&info);

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, info.Width, info.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, info.Data);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

				ilDeleteImages(1, &il_id);
			}
		}
		delete[] buffer;	
	}

	return true;
}

bool CubeMap::Unload()
{
	bool ret = true;

	glDeleteBuffers(1, (GLuint*)&id);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		LOG("Error removing buffer %i : %s", id, gluErrorString(error));
		ret = false;
	}

	return ret;	
}

void CubeMap::Bind(int texture_unit)
{
	glActiveTexture(texture_unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}
