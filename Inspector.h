#ifndef __INSPECTOR_H__
#define __INSPECTOR_H__

#include "Window.h"

class Inspector : public Window
{
public:
	Inspector();
	~Inspector();

	void Draw(ImGuiWindowFlags flags);

public:
	bool debug = false;

};

#endif

