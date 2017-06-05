#ifndef __INSPECTOR_H__
#define __INSPECTOR_H__

#include "Window.h"

class Inspector : public Window
{
public:
	Inspector();
	~Inspector();

	void Draw();

public:
	bool debug = false;
	bool staticAffectsChilds = false;

};

#endif

