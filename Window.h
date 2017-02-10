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

protected:
	bool active = false;
};

#endif // !__WINDOW_H_
