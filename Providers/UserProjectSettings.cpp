#pragma once
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS TRUE

#include "UserProjectSettings.h"
#include "CommonTypes.h"
#include "UnityInterfaces.h"
#include "Display/Display.h"

#ifdef _WIN32
#include <windows.h>    //GetModuleFileNameW
#include <direct.h>
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
const std::string kStreamingAssetsFilePath = "StreamingAssets\\SteamVR\\OpenVRSettings.asset";
#endif

const std::string kVSDebugPath = "..\\..\\";


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
		std::string actionManifestPath = std::string( s_UserDefinedSettings.actionManifestPath );
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
		projectDirectoryName = RemoveFileExtension( std::string( basename( fullExePath ) ) + "_Data/" );
		return basePath + projectDirectoryName;
		#endif
	}
}

std::string decode_utf8( uint32_t cp )
{
	std::wstring_convert< std::codecvt_utf8< char >, char > conv;
	return conv.to_bytes( ( char )cp );
}

std::string string_to_utf8( std::string str )
{
	std::string::size_type startIdx = 0;
	std::string::size_type endIdx = 0;
	do
	{
		startIdx = str.find( "\\u", startIdx );
		if ( startIdx == std::string::npos )
			break;

		endIdx = str.find_first_not_of( "0123456789abcdefABCDEF", startIdx + 2 );
		if ( endIdx == std::string::npos )
			break;

		std::string tmpStr = str.substr( startIdx + 2, endIdx - ( startIdx + 2 ) );
		std::istringstream iss( tmpStr );

		uint32_t cp;
		if ( iss >> std::hex >> cp )
		{
			std::string utf8 = decode_utf8( cp );
			str.replace( startIdx, 2 + tmpStr.length(), utf8 );
			startIdx += utf8.length();
		}
		else
			startIdx += 2;
	} while ( endIdx != std::string::npos );

	return str;
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

		lineValue = string_to_utf8( lineValue.c_str() );

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
		std::wstring string_to_convert = std::wstring( temp );

		//setup converter
		using convert_type = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_type, wchar_t> converter;

		//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
		return converter.to_bytes( string_to_convert );
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
	char path[MAX_PATH] = { 0 };
	GetModuleFileNameA( NULL, path, MAX_PATH );
	return std::string( path );
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
	std::ifstream infile( fileName );
	return infile.good();
}

bool UserProjectSettings::DirectoryExists( const char *const path )
{
	struct stat info;

	int statRC = stat( path, &info );
	if ( statRC != 0 )
	{
		if ( errno == ENOENT ) { return false; } // something along the path does not exist
		if ( errno == ENOTDIR ) { return false; } // something in path prefix is not a dir
		return false;
	}

	return ( info.st_mode & S_IFDIR ) ? true : false;
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

			std::string fullPath = UserProjectSettings::GetCurrentWorkingPath() + "\\Assets\\" + settings.actionManifestPath;
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

		if ( FileExists( settingsPath ) )
		{
			UserDefinedSettings settings;

			std::string line;
			std::ifstream infile;
			infile.open( settingsPath );
			if ( infile.is_open() )
			{
				while ( getline( infile, line ) )
				{
					std::string lineValue;

					if ( FindSettingAndGetValue( line, kStereoRenderingMode, lineValue ) )
					{
						settings.stereoRenderingMode = (unsigned short )std::stoi( lineValue );
					}
					else if ( FindSettingAndGetValue( line, kInitializationType, lineValue ) )
					{
						settings.initializationType = (unsigned short )std::stoi( lineValue );
					}
					else if ( FindSettingAndGetValue( line, kMirrorViewMode, lineValue ) )
					{
						settings.mirrorViewMode = (unsigned short )std::stoi( lineValue );
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
		else
		{
			XR_TRACE( "[OpenVR] [ERROR] Not initialized by Unity and could not find settings file. Searched paths: \n\t'%s'\n\t'%s'\n",
				( projectDirectoryPath + kStreamingAssetsFilePath ).c_str(), settingsPath.c_str() );
		}
	}
}