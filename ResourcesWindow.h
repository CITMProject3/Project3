#ifndef __RRESOURCESWINDOW_H__
#define __RRESOURCESWINDOW_H__

#include "Window.h"

class ResourcesWindow : public Window
{
public:
	ResourcesWindow();
	~ResourcesWindow();

	void Draw();
private:
	int mem_info_dedicated = 0; //dedicated video memory, total size (in kb) of the GPU memory
	int mem_info_total_available = 0; //total available memory, total size (in Kb) of the memory available for allocations
	int mem_info_current_available = 0; //current available dedicated video memory (in kb), currently unused GPU memory
	int mem_info_eviction_count = 0; //count of total evictions seen by system
	int mem_info_evicted = 0; //size of total video memory evicted (in kb)

	int mem_current_variation = 0;
};

#endif
