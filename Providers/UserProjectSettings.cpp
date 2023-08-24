#pragma once
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS TRUE

#include "UserProjectSettings.h"
#include "CommonTypes.h"
#include "UnityInterfaces.h"
#include "Display/Display.h"

using namespace std;
using namespace std::string_view_literals;
using namespace std::string_literals;

#ifdef _WIN32
#include <windows.h>    //GetModuleFileNameW
#include <direct.h>
#include <comdef.h> 
#else
#include <limits.h>
#include <unistd.h>     //readlink
#include <string.h>
#include <libgen.h>
#endif
#include <sys/stat.h>
#include <algorithm>

#include <locale>
#include <codecvt>        
#include <cstdint>      
#include <sstream>   

#ifdef __linux__
#define MAX_PATH PATH_MAX
#define _getcwd getcwd
char *strcpy_s( char *dest, const char *src )
{
	return strcpy( dest, src );
}
char *strcpy_s( char *dest, unsigned long maxCharacters, const char *src )
{
	return strncpy( dest, src, maxCharacters );
}
#endif

typedef struct _UserDefinedSettings
{
	unsigned short stereoRenderingMode = 0;
	unsigned short initializationType = 0;
	unsigned short mirrorViewMode = 0;
	const char *editorAppKey = "";
	const char *actionManifestPath = "";
	const char *applicationName = "";
} UserDefinedSettings;

static UserDefinedSettings s_UserDefinedSettings;
static bool bInitialized = false;

const std::string kStereoRenderingMode = "StereoRenderingMode:";
const std::string kInitializationType = "InitializationType:";
const std::string kEditorAppKey = "EditorAppKey:";
const std::string kActionManifestFilePath = "ActionManifestFileRelativeFilePath:";
const std::string kMirrorViewMode = "MirrorView:";

#ifdef __linux__
const std::string kStreamingAssetsFilePath = "StreamingAssets/SteamVR/OpenVRSettings.asset";
#else
const string kStreamingAssetsFilePath = "StreamingAssets\\SteamVR\\OpenVRSettings.asset";
#endif

const string kVSDebugPath = "..\\..\\";


#ifndef __linux__
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
typedef std::codecvt_utf8< wchar_t > convert_type;

std::string UTF16to8( const wchar_t *in )
{
	static std::wstring_convert< convert_type, wchar_t > s_converter;  // construction of this can be expensive (or even serialized) depending on locale

	try
	{
		return s_converter.to_bytes( in );
	}
	catch ( ... )
	{
		return std::string();
	}
}

std::string UTF16to8( const std::wstring &in ) { return UTF16to8( in.c_str() ); }


std::wstring UTF8to16( const char *in )
{
	static std::wstring_convert< convert_type, wchar_t > s_converter;  // construction of this can be expensive (or even serialized) depending on locale

	try
	{
		return s_converter.from_bytes( in );
	}
	catch ( ... )
	{
		return std::wstring();
	}
}

std::wstring UTF8to16( const std::string &in ) { return UTF8to16( in.c_str() ); }
#endif

std::string UserProjectSettings::GetInitStartupInfo()
{
	std::string startupInfo = "{ \n";
	bool hasData = false;
	if ( s_UserDefinedSettings.editorAppKey && strlen( s_UserDefinedSettings.editorAppKey ) > 0 )
	{
		if ( hasData )
		{
			startupInfo += ",\n";
		}
		startupInfo += "\t\"app_key\": \"" + std::string( s_UserDefinedSettings.editorAppKey ) + "\"";
		hasData = true;
	}

	if ( s_UserDefinedSettings.applicationName && strlen( s_UserDefinedSettings.applicationName ) > 0 )
	{
		if ( hasData )
		{
			startupInfo += ",\n";
		}
		startupInfo += "\t\"app_name\": \"" + std::string( s_UserDefinedSettings.applicationName ) + "\"";
		hasData = true;
	}

	if ( s_UserDefinedSettings.actionManifestPath && strlen( s_UserDefinedSettings.actionManifestPath ) > 0 )
	{
		if ( hasData )
		{
			startupInfo += ",\n";
		}
		string actionManifestPath = s_UserDefinedSettings.actionManifestPath;
		std::replace( actionManifestPath.begin(), actionManifestPath.end(), '\\', '/' );
		startupInfo += "\t\"action_manifest_path\": \"" + actionManifestPath + "\"";
		hasData = true;
	}
	startupInfo += "\n}";


	if ( hasData )
	{
		return startupInfo;
	}
	else
	{
		return std::string();
	}
}

std::string UserProjectSettings::GetProjectDirectoryPath( bool bAddDataDirectory )
{
	std::string exePath = GetExecutablePath();

	XR_TRACE( "[OpenVR] Initializing application at: %s\n", exePath.c_str() );

	char fullExePath[MAX_PATH];
	strcpy_s( fullExePath, exePath.c_str() );

	char exePathDrive[MAX_PATH];
	char exePathDirectory[MAX_PATH];
	char exePathFilename[MAX_PATH];
	char exePathExtension[MAX_PATH];

	#ifndef __linux__
	_splitpath_s( fullExePath, exePathDrive, exePathDirectory, exePathFilename, exePathExtension );

	std::string basePath = std::string( exePathDrive ) + std::string( exePathDirectory );
	#else
	std::string basePath = std::string( dirname( fullExePath ) );
	#endif

	if ( !bAddDataDirectory )
	{
		return basePath;
	}
	else
	{
		std::string projectDirectoryName;
		
		#ifndef __linux__
		projectDirectoryName = std::string( exePathFilename ) + "_Data\\";
		std::string projectDirectoryPath = basePath + projectDirectoryName;
		if ( !DirectoryExists( projectDirectoryPath.c_str() ) )
		{
			//check for the path it might be in if we've made a vs debug build
			projectDirectoryPath = basePath + kVSDebugPath + projectDirectoryName;
		}
		return projectDirectoryPath;
		#else
		size_t lastindex = exePath.find_last_of("."); 
		string projectPath = exePath.substr(0, lastindex); 
		return projectPath + "_Data/";
		#endif
	}
}


bool UserProjectSettings::FindSettingAndGetValue( const std::string &line, const std::string &lineKey, std::string &lineValue )
{
	size_t index = line.find( lineKey );
	if ( index != std::string::npos )
	{
		if ( line.at( index + lineKey.length() ) == ' ' )
		{
			index++;
		}

		lineValue = line.substr( index + lineKey.length(), line.length() - index );

		if ( lineValue.length() > 0 )
		{
			return true;
		}
	}
	return false;
}

#ifndef __linux__
std::string UserProjectSettings::GetCurrentWorkingPath()
{
	WCHAR temp[MAX_PATH];
	if ( _wgetcwd( temp, MAX_PATH ) ) 
	{
		return UTF16to8( temp );
	}
	else
	{
		return std::string( "" );
	}
}
#else
std::string UserProjectSettings::GetCurrentWorkingPath()
{
	char temp[MAX_PATH];
	if ( _getcwd( temp, MAX_PATH ) ) 
	{
		return std::string( temp );
	}
	else
	{
		return std::string( "" );
	}
}
#endif

std::string UserProjectSettings::GetExecutablePath()
{
#ifdef _WIN32
	WCHAR wpath[MAX_PATH] = { 0 };
	HMODULE hModule = GetModuleHandle( NULL );
	GetModuleFileNameW( hModule, wpath, MAX_PATH );
	return UTF16to8( wpath );
#else	
	char result[PATH_MAX];
	ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
	return std::string( result, ( count > 0 ) ? count : 0 );
#endif
}

void UserProjectSettings::Trim( std::string &s )
{
	std::string::const_iterator it = s.begin();
	while ( it != s.end() && isspace( (unsigned char )* it ) )
		it++;

	std::string::const_reverse_iterator rit = s.rbegin();
	while ( rit.base() != it && isspace( (unsigned char )* rit ) )
		rit++;

	s = std::string( it, rit.base() );
}

std::string UserProjectSettings::RemoveFileExtension( const std::string& filename ) 
{
    size_t lastdot = filename.find_last_of( "." );
    if ( lastdot == std::string::npos ) 
		return filename;
    return filename.substr( 0, lastdot ); 
}

EVRStereoRenderingModes UserProjectSettings::GetStereoRenderingMode()
{
	return (EVRStereoRenderingModes )s_UserDefinedSettings.stereoRenderingMode;
}

EVRMirrorViewMode UserProjectSettings::GetMirrorViewMode()
{
	return (EVRMirrorViewMode )s_UserDefinedSettings.mirrorViewMode;
}

vr::EVRApplicationType UserProjectSettings::GetInitializationType()
{
	switch ( s_UserDefinedSettings.initializationType )
	{
	case 1:
		return vr::VRApplication_Scene;
	case 2:
		return vr::VRApplication_Overlay;
	default:
		XR_TRACE( "[OpenVR] [Error] Unsupported application type: %d\n", s_UserDefinedSettings.initializationType );
	}

	return ( vr::EVRApplicationType )s_UserDefinedSettings.initializationType;
}

std::string UserProjectSettings::GetEditorAppKey()
{
	if ( s_UserDefinedSettings.editorAppKey )
	{
		return s_UserDefinedSettings.editorAppKey;
	}
	else
	{
		return std::string();
	}
}

std::string UserProjectSettings::GetActionManifestPath()
{
	if ( s_UserDefinedSettings.actionManifestPath )
	{
		return s_UserDefinedSettings.actionManifestPath;
	}
	else
	{
		return std::string();
	}
}

std::string UserProjectSettings::GetAppName()
{
	if ( s_UserDefinedSettings.applicationName )
	{
		return s_UserDefinedSettings.applicationName;
	}
	else
	{
		return std::string();
	}
}

int UserProjectSettings::GetUnityMirrorViewMode()
{
	int unityMode = kUnityXRMirrorBlitNone;

	switch ( (EVRMirrorViewMode )GetMirrorViewMode() )
	{
	case EVRMirrorViewMode::Eye_None:
		unityMode = kUnityXRMirrorBlitNone;
		break;
	case EVRMirrorViewMode::Eye_Left:
		unityMode = kUnityXRMirrorBlitLeftEye;
		break;
	case EVRMirrorViewMode::Eye_Right:
		unityMode = kUnityXRMirrorBlitRightEye;
		break;
	case EVRMirrorViewMode::SteamVR_Eye_Both:
	default:
		unityMode = kUnityXRMirrorBlitDistort;
		break;
	}

	return unityMode;
}

const char *GetStereoRenderingModeString( unsigned short nStereoRenderingMode )
{
	switch ( nStereoRenderingMode )
	{
	case 0:
		return "Multi Pass";
	case 1:
		return "Single Pass Instanced";
	default:
		return "Unknown";
	}
}

const char *GetInitializationTypeString( unsigned short nInitializationType )
{
	switch ( nInitializationType )
	{
	case 1:
		return "Scene";
	case 2:
		return "Overlay";
	default:
		return "Unknown";
	}
}

const char *GetMirrorViewModeString( unsigned short nMirrorViewMode )
{
	switch ( nMirrorViewMode )
	{
	case 0:
		return "None";
	case 1:
		return "Left Eye";
	case 2:
		return "Right Eye";
	case 3:
		return "OpenVR View";
	default:
		return "Unknown";
	}
}

bool UserProjectSettings::FileExists( const std::string &fileName )
{
	#ifndef __linux__
	wstring wfileName = UTF8to16( fileName );
	std::ifstream infile( wfileName );
	#else
	std::ifstream infile( fileName );
	#endif
	return infile.good();
}

bool UserProjectSettings::DirectoryExists( const char *const path )
{
#ifndef __linux__
	struct _stat64i32 info;
	wstring wPath = UTF8to16( path );
	int statRC = _wstat( wPath.c_str(), &info );
#else
	struct stat info;
	int statRC = stat( path, &info );
#endif

	if ( statRC != 0 )
	{
		if ( errno == ENOENT ) { return false; } // something along the path does not exist
		if ( errno == ENOTDIR ) { return false; } // something in path prefix is not a dir
		return false;
	}
	
	if ( info.st_mode & S_IFDIR )
	{
		return true;
	}
	return false;
}

extern "C" uint16_t UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
GetMirrorViewMode()
{
	return s_UserDefinedSettings.mirrorViewMode;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
SetMirrorViewMode( uint16_t mirrorViewMode )
{
	XR_TRACE( "[OpenVR] Extern SetMirrorViewMode (%s)\n", GetMirrorViewModeString( mirrorViewMode ) );

	s_UserDefinedSettings.mirrorViewMode = mirrorViewMode;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
SetUserDefinedSettings( UserDefinedSettings settings )
{
	XR_TRACE( "[OpenVR] Loaded settings: \n\tEditor App Name : %s\n", settings.applicationName );
	XR_TRACE( "\tEditor App Key : %s\n", settings.editorAppKey );
	XR_TRACE( "\tAction Manifest Path : %s\n", settings.actionManifestPath );
	XR_TRACE( "\tStereo Rendering Mode : %s\n",
		GetStereoRenderingModeString( settings.stereoRenderingMode ) );
	XR_TRACE( "\tInitialization Type : %s\n",
		GetInitializationTypeString( settings.initializationType ) );
	XR_TRACE( "\tMirror View Mode : %s\n",
		GetMirrorViewModeString( settings.mirrorViewMode ) );

	s_UserDefinedSettings.stereoRenderingMode = settings.stereoRenderingMode;
	s_UserDefinedSettings.initializationType = settings.initializationType;
	s_UserDefinedSettings.mirrorViewMode = settings.mirrorViewMode;

	if ( settings.editorAppKey && strlen( settings.editorAppKey ) > 1 )
	{
		size_t strLen = strlen( settings.editorAppKey ) + 1;
		char *editorAppKey = new char[ strLen ];
		strcpy_s( editorAppKey, strLen, settings.editorAppKey );
		s_UserDefinedSettings.editorAppKey = editorAppKey;
	}

	if ( settings.actionManifestPath && strlen( settings.actionManifestPath ) > 1 )
	{
		#ifdef __linux__
		size_t actionManifestPathLength = strlen( settings.actionManifestPath );
		std::string actionManifestPath = settings.actionManifestPath;
		std::replace( actionManifestPath.begin(), actionManifestPath.end(), '\\', '/' );
		char *updatedActionManifestPath = new char[ actionManifestPathLength ];
		std::copy( actionManifestPath.begin(), actionManifestPath.end(), updatedActionManifestPath );
		settings.actionManifestPath = updatedActionManifestPath;
		#endif

		if ( UserProjectSettings::FileExists( std::string( settings.actionManifestPath ) ) )
		{
			size_t strLen = strlen( settings.actionManifestPath ) + 1;
			char *actionManifestPath = new char[ strLen ];
			strcpy_s( actionManifestPath, strLen, settings.actionManifestPath );
			s_UserDefinedSettings.actionManifestPath = actionManifestPath;
		}
		else
		{
			XR_TRACE( "[OpenVR] [path] %s\n", UserProjectSettings::GetCurrentWorkingPath().c_str() );

			#ifndef __linux__
			std::string fullPath = UserProjectSettings::GetCurrentWorkingPath() + "\\Assets\\" + settings.actionManifestPath;
			#else
			std::string fullPath = UserProjectSettings::GetCurrentWorkingPath() + "/Assets/" + settings.actionManifestPath;
			#endif

			char *actionManifestPath = new char[fullPath.size() + 1];
			std::copy( fullPath.begin(), fullPath.end(), actionManifestPath );
			actionManifestPath[fullPath.size()] = '\0';
			s_UserDefinedSettings.actionManifestPath = actionManifestPath;

			if ( UserProjectSettings::FileExists( fullPath ) == false )
			{
				XR_TRACE( "[OpenVR] [Error] Action manifest file does not exist at path (%s)\n", actionManifestPath );
			}
		}

	}

	if ( settings.applicationName && strlen( settings.applicationName ) > 1 )
	{
		size_t strLen = strlen( settings.applicationName ) + 1;
		char *appName = new char[ strLen ];
		strcpy_s( appName, strLen, settings.applicationName );
		s_UserDefinedSettings.applicationName = appName;
	}

	bInitialized = true;
}

void UserProjectSettings::Initialize()
{
	if ( !bInitialized )
	{
		std::string projectDirectoryPath = GetProjectDirectoryPath( true );
		std::string settingsPath = projectDirectoryPath + kStreamingAssetsFilePath;

		bool settingsFileExists = FileExists( settingsPath );
		std::ifstream infile;

		if ( settingsFileExists )
		{
#ifndef __linux__
			wstring wSettingsPath = UTF8to16( settingsPath );
			infile.open( wSettingsPath );
#else
			infile.open( settingsPath );
#endif

		}
		else
		{
			XR_TRACE( "[OpenVR] [ERROR] Not initialized by Unity and could not find settings file. Searched paths: \n\t'%s'\n\t'%s'\n",
				( projectDirectoryPath + kStreamingAssetsFilePath ).c_str(), settingsPath.c_str() );
		}

		UserDefinedSettings settings;

		std::string line;
		if ( infile.is_open() )
		{
			while ( getline( infile, line ) )
			{
				std::string lineValue;

				if ( FindSettingAndGetValue( line, kStereoRenderingMode, lineValue ) )
				{
					settings.stereoRenderingMode = ( unsigned short )std::stoi( lineValue );
				}
				else if ( FindSettingAndGetValue( line, kInitializationType, lineValue ) )
				{
					settings.initializationType = ( unsigned short )std::stoi( lineValue );
				}
				else if ( FindSettingAndGetValue( line, kMirrorViewMode, lineValue ) )
				{
					settings.mirrorViewMode = ( unsigned short )std::stoi( lineValue );
				}
				/*else if ( FindSettingAndGetValue( line, kEditorAppKey, lineValue ) ) //only for in editor
				{
					Trim( lineValue );
					char editorAppKey[MAX_PATH];
					strcpy(editorAppKey, lineValue.c_str());
					settings.editorAppKey = editorAppKey;
				}*/
				else if ( FindSettingAndGetValue( line, kActionManifestFilePath, lineValue ) )
				{
					Trim( lineValue );
					lineValue = projectDirectoryPath + lineValue;

					char actionManifestPath[MAX_PATH];
					strcpy_s( actionManifestPath, lineValue.c_str() );
					settings.actionManifestPath = actionManifestPath;
				}
			}
			infile.close();
		}

		SetUserDefinedSettings( settings );
	}
}