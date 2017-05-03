#include "Window.h"

Window::Window()
{}

Window::~Window()
{}

void Window::Draw()
{}

void Window::SetActive(bool value)
{
	active = value;
}

void Window::OnResize(int screen_width, int screen_height)
{
	//Main menu bar
	screen_height -= 19;

	current_position.x = screen_width * relative_position.x;
	current_position.y = 19 + screen_height * relative_position.y;

	current_size.x = screen_width * relative_size.x;
	current_size.y = screen_height * relative_size.y;
}

void Window::SetRelativeDimensions(ImVec2 position, ImVec2 size)
{
	relative_position = position;
	relative_size = size;
}