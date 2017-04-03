#pragma once

#include "Globals.h"

unsigned char SetFlag(unsigned char container, uint flag, bool value)
{
	if (value) { return container |= flag; }
	return container &= ~flag;
}

bool ReadFlag(unsigned char container, uint flag)
{ 
	return container & flag;
}