#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>    //GetModuleFileNameW
#include <direct.h>
#include <comdef.h> 
#else
#include <limits.h>
#include <unistd.h>     //readlink
#include <string.h>
#include <libgen.h>
#endif  

#include <string>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>

#include <locale>
#include <codecvt>        
#include <cstdint>      
#include <sstream> 

using namespace std;
using namespace std::string_view_literals;
using namespace std::string_literals;


#include "OpenVR/openvr.h"
#include "UnityInterfaces.h"
#include "CommonTypes.h"
#include "ProviderInterface/IUnityXRDisplay.h"

enum EVRMirrorViewMode
{
	Eye_None = 0,
	Eye_Left = 1,
	Eye_Right = 2,
	SteamVR_Eye_Both = 3,
};

enum EVRStereoRenderingModes
{
	MultiPass = 0,
	SinglePassInstanced = 1,
};

class UserProjectSettings
{
public:
	static EVRStereoRenderingModes GetStereoRenderingMode();
	static vr::EVRApplicationType GetInitializationType();
	static std::string GetEditorAppKey();
	static std::string GetActionManifestPath();
	static std::string GetAppName();
	static void Initialize();
	static std::string GetInitStartupInfo();
	static EVRMirrorViewMode GetMirrorViewMode();
	static int GetUnityMirrorViewMode();
	static std::string GetProjectDirectoryPath( bool bAddDataDirectory );
	static std::string GetCurrentWorkingPath();
	static bool FileExists( const std::string &fileName );


private:
	static bool FindSettingAndGetValue( const std::string &line, const std::string &lineKey, std::string &lineValue );
	static std::string GetExecutablePath();
	static void Trim( std::string &s );
	static bool DirectoryExists( const char *const path );
	static std::string RemoveFileExtension( const std::string &filename );
};
