#ifndef __RENDERTEXEDITORWINDOW_H__
#define __RENDERTEXEDITORWINDOW_H__

#include "Window.h"

class RenderTexEditorWindow : public Window
{
public:
	RenderTexEditorWindow();
	~RenderTexEditorWindow();

	void Draw();

	void LoadToEdit(const char* path);

private:

	int width = 0;
	int height = 0;
	bool use_depth_as_texture = false;
	string assets_path;
	string library_path;	
};

#endif
