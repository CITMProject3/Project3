#include "Application.h"
#include "ModuleResourceManager.h"
#include "ResourcesWindow.h"
#include "Globals.h"

#include "Glew\include\glew.h"
#include "SDL\include\SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>

#define GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX          0x9047
#define GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    0x9048
#define GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  0x9049
#define GPU_MEMORY_INFO_EVICTION_COUNT_NVX            0x904A
#define GPU_MEMORY_INFO_EVICTED_MEMORY_NVX            0x904B

ResourcesWindow::ResourcesWindow()
{}

ResourcesWindow::~ResourcesWindow()
{}

void ResourcesWindow::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Resources", &active, flags);

	glGetIntegerv(GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &mem_info_dedicated);
	glGetIntegerv(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &mem_info_total_available);

	int current_mem = 0;
	glGetIntegerv(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &current_mem);
	if (current_mem != mem_info_current_available)
	{
		mem_current_variation = current_mem - mem_info_current_available;
		mem_info_current_available = current_mem;
	}

	glGetIntegerv(GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &mem_info_eviction_count);
	glGetIntegerv(GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &mem_info_evicted);

	ImGui::Text("Memory dedicated: %i kbs", mem_info_dedicated);
	ImGui::Text("Total memory available: %i kbs", mem_info_total_available);
	ImGui::Text("Current memory available: %i kbs", mem_info_current_available);
	if (mem_current_variation != 0)
	{
		ImGui::SameLine();
		if (mem_current_variation > 0)
			ImGui::TextColored(ImVec4(0, 200, 0, 255), "   +%i", mem_current_variation);
		else
			ImGui::TextColored(ImVec4(200, 0, 0, 255), "   -%i", mem_current_variation);
	}
	ImGui::Text("Total evictions: %i ", mem_info_eviction_count);
	ImGui::Text("Evicted memory: %i kbs", mem_info_evicted);
	
	ImGui::End();
}

//OLD RESOURCE CODE - DO NOT REMOVE

/*ImGui::Text("Num resources: %i", App->resource_manager->GetNumberResources());
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
}*/