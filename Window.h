#ifndef __WINDOW_H_
#define __WINDOW_H_

#include "Imgui\imgui.h"

class Window 
{
public:
	Window();
	~Window();

	virtual void Draw(ImGuiWindowFlags flags);
	void SetActive(bool value);

protected:
	bool active = false;
	ImVec2 position;
	ImVec2 size;
};

#endif // !__WINDOW_H_
