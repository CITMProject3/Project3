#include "WarningWindow.h"

WarningWindow::WarningWindow()
{ }

WarningWindow::~WarningWindow()
{ 
	RemoveMessages();
}

void WarningWindow::AddMessage(const char *message, WarningType type)
{
	WarningMessage *msg = new WarningMessage(message, type);
	warning_messages.push_back(msg);
	SetActive(true);
}

void WarningWindow::Draw()
{
	if (!active)
		return;

	ImGui::Begin("Warning!", &active, flags);

	for (std::vector<WarningMessage*>::iterator it = warning_messages.begin(); it != warning_messages.end(); ++it)
	{
		switch ((*it)->type)
		{
			case(WarningType::W_ERROR):
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[ERROR] ");
				ImGui::SameLine();
				ImGui::Text((*it)->message.data());
				break;
			}
			case(WarningType::W_WARNING):
			{
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "[WARNING] ");
				ImGui::SameLine();
				ImGui::Text((*it)->message.data());
				break;
			}
		}
	}

	ImGui::Separator();
	
	if (ImGui::Button("CLEAN")) // remove all warning messages
		RemoveMessages();

	ImGui::SameLine();
	if (ImGui::Button("OK"))  // close window
		SetActive(false);

	ImGui::End();
}

void WarningWindow::RemoveMessages()
{
	for (std::vector<WarningMessage*>::iterator it = warning_messages.begin(); it != warning_messages.end(); ++it)
		delete (*it);
	warning_messages.clear();
}


