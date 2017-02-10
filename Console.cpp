#include "Console.h"

Console::Console()
{}

Console::~Console()
{
	buffer.clear();
}

void Console::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Console", &active);

	ImGui::TextUnformatted(buffer.begin());
	if (ScrollToBottom)
		ImGui::SetScrollHere(1.0f);
	ScrollToBottom = false;

	ImGui::End();
}

void Console::Write(const char* text)
{
	buffer.append(text);
	ScrollToBottom = true;
}

