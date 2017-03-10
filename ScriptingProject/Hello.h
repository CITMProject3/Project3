#ifndef __HELLO_H__
#define __HELLO_H__

#include "Scripts.h"

class __declspec(dllexport) Hello : public Scripts
{
public:
	void Say();
};

#endif // __HELLO_H__
