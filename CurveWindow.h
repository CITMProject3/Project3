#ifndef __CURVEWINDOW_H__
#define __CURVEWINDOW_H__

#include "Window.h"
#include <vector>

class CurveWindow : public Window
{
public:
	CurveWindow();
	~CurveWindow();

	void Draw();
	void ResetPoints();

	std::vector<std::vector<ImVec2>> points;
	ImGuiCurveEditorMode_ mode = ImGuiCurveEditorMode_EditPoints;
	bool show_axis[3] = { true, true, true };
	int edit_axis = 0;
};

#endif
