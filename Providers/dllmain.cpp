#pragma once

#include "ProviderInterface/IUnityInterface.h"
#include "UnityInterfaces.h"
#include "CommonTypes.h"
#include "ProviderInterface/IUnityXRPreInit.h"
#include "ProviderInterface/IUnityXRTrace.h"
#include "ProviderInterface/UnitySubsystemTypes.h"

#include "OpenVRSystem.h"
#include "OpenVRProviderContext.h"
#include "Display/Display.h"
#include "Input/Input.h"
#include "UserProjectSettings.h"

static OpenVRProviderContext *s_pOpenVRProviderContext {};

UnitySubsystemErrorCode Load_Display( OpenVRProviderContext & );
UnitySubsystemErrorCode Load_Input( OpenVRProviderContext & );

IUnityXRTrace *s_pXRTrace = nullptr;

static bool ReportError( const char *subsystemProviderName, UnitySubsystemErrorCode err )
{
	if ( err != kUnitySubsystemErrorCodeSuccess )
	{
		XR_TRACE_ERROR( s_pOpenVRProviderContext->trace, "[OpenVR] [Error] Error loading subsystem: %s (%d)\n", subsystemProviderName, err );
		return true;
	}
	return false;
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityPluginLoad( IUnityInterfaces *unityInterfaces )
{
	// Setup interfaces
	UnityInterfaces::Get().SetUnityInterfaces( unityInterfaces );
	s_pXRTrace = (IUnityXRTrace * )unityInterfaces->GetInterface( UNITY_GET_INTERFACE_GUID( IUnityXRTrace ) );

	// Setup provider context
	s_pOpenVRProviderContext = new OpenVRProviderContext;
	s_pOpenVRProviderContext->interfaces = unityInterfaces;
	s_pOpenVRProviderContext->trace = (IUnityXRTrace * )unityInterfaces->GetInterface( UNITY_GET_INTERFACE_GUID( IUnityXRTrace ) );

	XR_TRACE( "[OpenVR] Registering providers\n" );
	RegisterDisplayLifecycleProvider( s_pOpenVRProviderContext );
	RegisterInputLifecycleProvider( s_pOpenVRProviderContext );
}


static bool UNITY_INTERFACE_API GetPreInitFlags( void *userData, uint64_t *flags )
{
	*flags = 0;

	return true;
}


static bool UNITY_INTERFACE_API GetGraphicsAdapterId( void *userData, UnityXRPreInitRenderer renderer, uint64_t rendererData, uint64_t *adapterId )
{
	if ( OpenVRSystem::Get().GetCompositor() )
	{
		return OpenVRSystem::Get().GetGraphicsAdapterId( userData, renderer, rendererData, adapterId );
	}

	return false;
}


static bool UNITY_INTERFACE_API GetVulkanInstanceExtensions( void *userData, uint32_t namesCapacityIn, uint32_t *namesCountOut, char *namesString )
{
	return OpenVRSystem::Get().GetVulkanInstanceExtensions( userData, namesCapacityIn, namesCountOut, namesString );
}


static bool UNITY_INTERFACE_API GetVulkanDeviceExtensions( void *userData, uint32_t namesCapacityIn, uint32_t *namesCountOut, char *namesString )
{
	return OpenVRSystem::Get().GetVulkanDeviceExtensions( userData, namesCapacityIn, namesCountOut, namesString );
}


extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
XRSDKPreInit( IUnityInterfaces *interfaces )
{
	IUnityXRPreInit *preInit = (IUnityXRPreInit * )interfaces->GetInterface( GetUnityInterfaceGUID<IUnityXRPreInit>() );

	UnityXRPreInitProvider provider = { 0 };

	provider.userData = nullptr;
	provider.GetPreInitFlags = GetPreInitFlags;
	provider.GetGraphicsAdapterId = GetGraphicsAdapterId;
	provider.GetVulkanInstanceExtensions = GetVulkanInstanceExtensions;
	provider.GetVulkanDeviceExtensions = GetVulkanDeviceExtensions;

	preInit->RegisterPreInitProvider( &provider );

	XR_TRACE( "XRPreInitProvider registered\n" );
}
