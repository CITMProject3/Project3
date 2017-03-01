#ifndef __FPSGRAPH_H__
#define __FPSGRAPH_H__

#include "Window.h"
#include <vector>

using namespace std;

class FPSGraph : public Window
{
public:
	FPSGraph();
	~FPSGraph();

	void Draw();
private:
	vector<float> frames;
	int max_fps = 60;
};

#endif
