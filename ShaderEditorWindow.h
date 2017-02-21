#ifndef __SHADEREDITORWINDOW_H__
#define __SHADEREDITORWINDOW_H__

#include "Window.h"
#include <string>

class ShaderEditorWindow : public Window
{
public:
	ShaderEditorWindow();
	~ShaderEditorWindow();

	void Draw();
private:
	bool edit_vertex_enabled = true;
	std::string vertex_path = "";
	std::string fragment_path = "";

	char* program = nullptr;

	//texture testing
	int id = 0;
};

#endif
