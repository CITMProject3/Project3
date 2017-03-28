#ifndef __WARNINGWINDOW_H__
#define __WARNINGWINDOW_H__

#include "Window.h"

//#include <string>

enum WarningType
{
	W_ERROR,
	W_WARNING
};


struct WarningMessage
{
	WarningType type;
	std::string message;

	WarningMessage(const char *message, WarningType type)
	{
		this->message.assign(message);
		this->type = type;
	}	
};

class WarningWindow : public Window
{

public:

	WarningWindow();
	~WarningWindow();

	void AddMessage(const char *msg, WarningType type);
	void Draw();

private:

	std::vector<WarningMessage*> warning_messages;

	void RemoveMessages();
};

#endif
