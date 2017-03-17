#include <stdio.h>

#include "../GameObject.h"

extern "C"
{
	extern __declspec(dllexport) void DisplayHelloFromDLL()
	{
		GameObject *go = new GameObject();
		unsigned uuid = go->GetUUID();
		//LOG("%s","Hello from DLL !");
	}
}