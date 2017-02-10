#include "Application.h"
#include "ModuleResourceManager.h"
#include "ResourcesWindow.h"
#include "Globals.h"

ResourcesWindow::ResourcesWindow()
{}

ResourcesWindow::~ResourcesWindow()
{}

void ResourcesWindow::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Resources", &active);

	ImGui::Text("Num resources: %i", App->resource_manager->GetNumberResources());
	ImGui::Text("Num meshes: %i", App->resource_manager->GetNumberMeshes());
	ImGui::Text("Num textures: %i", App->resource_manager->GetNumberTexures());
	
	int bytes = App->resource_manager->GetTotalBytesInMemory();

	if (bytes > GIGABYTE)
	{
		float gigabytes = (float)bytes / (float)GIGABYTE;
		ImGui::Text("Total Memory: %0.3f GBs", gigabytes);
	}
	else if(bytes > MEGABYTE)
	{
		float megabytes = (float)bytes / (float)MEGABYTE;
		ImGui::Text("Total Memory: %0.3f MBs", megabytes);
	}
	else if (bytes > KILOBYTE)
	{
		float kilobytes = (float)bytes / (float)KILOBYTE;
		ImGui::Text("Total Memory: %0.3f KBs", kilobytes);
	}
	else 
	{
		ImGui::Text("Total Memory: %i bytes", bytes);
	}

	int texture_bytes = App->resource_manager->GetTextureBytes();
	if (texture_bytes > GIGABYTE)
	{
		float giga_bytes = (float)texture_bytes / (float)GIGABYTE;
		ImGui::Text("Total Texture: %0.3f GBs", giga_bytes);
	}
	else if (texture_bytes > MEGABYTE)
	{
		float mega_bytes = (float)texture_bytes / (float)MEGABYTE;
		ImGui::Text("Total Texture: %0.3f MBs", mega_bytes);
	}
	else if (texture_bytes > KILOBYTE)
	{
		float kilo_bytes = (float)texture_bytes / (float)KILOBYTE;
		ImGui::Text("Total Texture: %0.3f KBs", kilo_bytes);
	}
	else
	{
		ImGui::Text("Total Texture: %i bytes", texture_bytes);
	}

	int mesh_bytes = App->resource_manager->GetMeshBytes();
	if (mesh_bytes > GIGABYTE)
	{
		float giga_bytes = (float)mesh_bytes / (float)GIGABYTE;
		ImGui::Text("Total Mesh: %0.3f GBs", giga_bytes);
	}
	else if (mesh_bytes > MEGABYTE)
	{
		float mega_bytes = (float)mesh_bytes / (float)MEGABYTE;
		ImGui::Text("Total Mesh: %0.3f MBs", mega_bytes);
	}
	else if (mesh_bytes > KILOBYTE)
	{
		float kilo_bytes = (float)mesh_bytes / (float)KILOBYTE;
		ImGui::Text("Total Mesh: %0.3f KBs", kilo_bytes);
	}
	else
	{
		ImGui::Text("Total Mesh: %i bytes", mesh_bytes);
	}


	
	ImGui::End();
}

