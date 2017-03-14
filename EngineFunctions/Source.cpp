#include <stdio.h>

extern "C"
{
	__declspec(dllexport) void DisplayHelloFromDLL()
	{
		printf("Hello from DLL !");
	}
}