/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the 
"Apache License"); you may not use this file except in compliance with the 
Apache License. You may obtain a copy of the Apache License at 
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Version: v2016.2.1  Build: 5995
  Copyright (c) 2006-2016 Audiokinetic Inc.
*******************************************************************************/

/// \file 
/// Audiokinetic platform checks. This is where we detect which platform
/// is being compiled, and where we define the corresponding AK-specific
/// symbols.

#pragma once

#if defined( NN_PLATFORM_CTR )

	#include <AK/SoundEngine/Platforms/3DS/AkTypes.h>

#elif _XBOX_VER >= 200 // Check Xbox before WIN32 because WIN32 might also be defined in some cases in Xbox 360 projects
 
	#include <AK/SoundEngine/Platforms/XBox360/AkTypes.h>

#elif defined( _XBOX_ONE )

	#include <AK/SoundEngine/Platforms/XboxOne/AkTypes.h>

#elif defined( _WIN32 ) || defined ( _WIN64 ) || defined( WINAPI_FAMILY )

	#include <AK/SoundEngine/Platforms/Windows/AkTypes.h>

#elif defined( __APPLE__ )

	#include <AK/SoundEngine/Platforms/Mac/AkTypes.h>

#elif defined (__PPU__) || defined (__SPU__)

	#include <AK/SoundEngine/Platforms/PS3/AkTypes.h>

#elif defined( CAFE ) || defined( RVL_OS )

	#include <AK/SoundEngine/Platforms/WiiFamily/AkTypes.h>

#elif defined( __SCE__ ) && defined( __arm__ )

	#include <AK/SoundEngine/Platforms/Vita/AkTypes.h>
	
#elif defined( __ORBIS__ )

	#include <AK/SoundEngine/Platforms/PS4/AkTypes.h>

#elif defined( __ANDROID__ )

	#include <AK/SoundEngine/Platforms/Android/AkTypes.h>

#elif defined( __native_client__ )

	#include <AK/SoundEngine/Platforms/nacl/AkTypes.h>

#elif defined( __linux__ )

	#include <AK/SoundEngine/Platforms/Linux/AkTypes.h>

#elif defined( __EMSCRIPTEN__ )

	#include <AK/SoundEngine/Platforms/Emscripten/AkTypes.h>
	
#elif defined( __QNX__ )

	#include <AK/SoundEngine/Platforms/QNX/AkTypes.h>

#else

	#error Unsupported platform, or platform-specific symbols not defined

#endif
