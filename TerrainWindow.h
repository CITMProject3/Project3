#ifndef __TERRAINWINDOW_H__
#define __TERRAINWINDOW_H__

#include "Window.h"

class TerrainWindow : public Window
{
public:
	TerrainWindow();
	~TerrainWindow();

	void Draw();

	string terrainPlacingObject;
	float timer = 0.0f;
};

#endif
