#ifndef __WINDOW_H_
#define __WINDOW_H_

#include "Imgui\imgui.h"

class Window 
{
public:
	Window();
	~Window();

	virtual void Draw();
	void SetActive(bool value);

	//Screen position control
	void OnResize(int screen_width, int screen_height);
	void SetRelativeDimensions(ImVec2 position, ImVec2 size);

protected:
	bool active = false;

	ImVec2 current_position;
	ImVec2 current_size;

	//0 to 1 in screen
	ImVec2 relative_position;
	ImVec2 relative_size;

	ImGuiWindowFlags flags = 0;
};

#endif // !__WINDOW_H_
