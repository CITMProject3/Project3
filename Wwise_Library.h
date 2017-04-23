#ifndef __WWISE_LIBRARY_H__
#define __WWISE_LIBRARY_H__

// - INCLUDES - 
#include <AK/SoundEngine/Common/AkSoundEngine.h>	   // SoundEngine
#include <AK/MusicEngine/Common/AkMusicEngine.h>	   // MusicEngine
#include <AK/SoundEngine/Common/AkMemoryMgr.h>         // Memory Manager
#include <AK/SoundEngine/Common/AkModule.h>            // Default memory and stream managers  
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>   // Streaming Manager
#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>           // Thread defines

// http://stackoverflow.com/questions/7668200/error-lnk2038-mismatch-detected-for-iterator-debug-level-value-0-doesnt

// Input libraries
#ifdef _DEBUG  // Profile build configuration must be loaded instead of Debug
	#pragma comment( lib, "AK/Debug/lib/AkSoundEngine.lib")
	#pragma comment( lib, "AK/Debug/lib/AkMusicEngine.lib")
	#pragma comment( lib, "AK/Debug/lib/AkMemoryMgr.lib")
	#pragma comment( lib, "AK/Debug/lib/AkStreamMgr.lib")
#else
	#define AK_OPTIMIZED
	#pragma comment( lib, "AK/Release/lib/AkSoundEngine.lib")
	#pragma comment( lib, "AK/Release/lib/AkMusicEngine.lib")
	#pragma comment( lib, "AK/Release/lib/AkMemoryMgr.lib")
	#pragma comment( lib, "AK/Release/lib/AkStreamMgr.lib")
#endif

// External library dependencies
#pragma comment( lib, "AK/dinput8.lib") // Microsoft DirectX DirectInput.Needed by Motion to support DirectInput devices.
#pragma comment( lib, "AK/dsound.lib")  // Microsoft DirectX DirectSound library
#pragma comment( lib, "AK/dxguid.lib")  // Microsoft DirectX Audio GUIDs
//#pragma comment( lib, "Ak/xinput.lib")  // WARNING-> Do not exist on the project! Microsoft XInput.Needed by Motion to support XInput devices(Xbox controller).

// Include for communication between Wwise and the game -- Not needed in the release version
#ifndef AK_OPTIMIZED
	#include <AK/Comm/AkCommunication.h>
	#pragma comment( lib, "AK/Debug/lib/CommunicationCentral.lib")
	#pragma comment( lib, "AK/ws2_32.lib")  // Microsoft Winsock 2 library (used for Wwise profiling)
#endif

// Custom alloc/free functions. These are declared as "extern" in AkMemoryMgr.h
// and MUST be defined by the game developer.
namespace AK
{
#ifdef WIN32
	void * AllocHook(size_t in_size);
	void FreeHook(void * in_ptr);
	void * VirtualAllocHook( void * in_pMemAddress,	size_t in_size,	DWORD in_dwAllocationType, DWORD in_dwProtect);	
	void VirtualFreeHook( void * in_pMemAddress, size_t in_size,DWORD in_dwFreeType	);
#endif
}

#endif
