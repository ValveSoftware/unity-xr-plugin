#pragma once

#include "OpenVRSystem.h"
#include "UnityInterfaces.h"
#include "CommonTypes.h"
#include "ProviderInterface/IUnityGraphics.h"
#include "ProviderInterface/IUnityInterface.h"
#include "UserProjectSettings.h"

#include <cassert>


OpenVRSystem::OpenVRSystem() :
	m_FrameIndex( 0 ),
	m_VRSystem( nullptr ),
	m_VRCompositor( nullptr ),
	initError( vr::VRInitError_None ),
	tickCallback( nullptr )
{
	XR_TRACE( ( "[OpenVR] static Initialize: " + UserProjectSettings::GetEditorAppKey() + "\n" ).c_str() );

	if ( UserProjectSettings::GetEditorAppKey().empty() )
	{
		//if we're not coming from an editor then we need to initialize immediately. This seems like a bug that unity should fix. (keithb 1/28/2020)
		Initialize();
	}
}

OpenVRSystem::~OpenVRSystem()
{
	//Shutdown();
}

bool OpenVRSystem::GetInitialized()
{
	return ( m_VRSystem != nullptr );
}

bool OpenVRSystem::Initialize()
{
	if ( !GetInitialized() )
	{
		XR_TRACE( "[OpenVR] Starting Initialize\n" );

		initError = vr::VRInitError_None;

		UserProjectSettings::Initialize();
		std::string startupInfo = UserProjectSettings::GetInitStartupInfo();

		vr::IVRSystem *vrSystem;

		if ( startupInfo.empty() )
		{
			XR_TRACE( "[OpenVR] startupInfo is empty\n" );
			vrSystem = vr::VR_Init( &initError, UserProjectSettings::GetInitializationType() );
		}
		else
		{
			XR_TRACE( ( "[OpenVR] startupInfo: " + startupInfo + "\n" ).c_str() );
			vrSystem = vr::VR_Init( &initError, UserProjectSettings::GetInitializationType(), startupInfo.c_str() );
		}

		if ( initError != vr::VRInitError_None )
		{
			std::string errorName = vr::VR_GetVRInitErrorAsSymbol( initError );
			XR_TRACE( ( "[OpenVR] [ERROR] VR_Init initError: " + errorName + "\n" ).c_str() );
			return false;
		}

		vr::IVRCompositor *vrCompositor = StartVRCompositor();

		if ( initError != vr::VRInitError_None )
		{
			std::string errorName = vr::VR_GetVRInitErrorAsSymbol( initError );
			XR_TRACE( ( "[OpenVR] [ERROR] VR_Init initError: " + errorName + "\n" ).c_str() );
			return false;
		}

		m_VRSystem = vrSystem;
		m_VRCompositor = vrCompositor;
	}

	XR_TRACE( "[OpenVR] is initialized\n" );
	return true;
}


bool OpenVRSystem::Shutdown()
{
	XR_TRACE( "[OpenVR] Shutdown\n" );

	if ( GetInitialized() )
	{
		vr::VR_Shutdown();
		m_VRSystem = nullptr;
		tickCallback = nullptr;
	}

	return true;
}

bool OpenVRSystem::Update()
{
	m_FrameIndex++;

	if ( tickCallback )
	{
		tickCallback( m_FrameIndex );
	}

	return true;
}

bool OpenVRSystem::GetGraphicsAdapterId( void *userData, UnityXRPreInitRenderer renderer, uint64_t rendererData, uint64_t *adapterId )
{
	if ( GetInitialized() && GetCompositor() )
	{
		if ( renderer == UnityXRPreInitRenderer::kUnityXRPreInitRendererD3D11 )
		{
            m_VRSystem->GetOutputDevice( &graphicsAdapterId, vr::TextureType_DirectX );
		}
		else if ( renderer == UnityXRPreInitRenderer::kUnityXRPreInitRendererD3D12 )
		{
			m_VRSystem->GetOutputDevice( &graphicsAdapterId, vr::TextureType_DirectX12 );
		}
		else if ( renderer == UnityXRPreInitRenderer::kUnityXRPreInitRendererGLCore )
		{
			m_VRSystem->GetOutputDevice( &graphicsAdapterId, vr::TextureType_OpenGL );
		}
		else if ( renderer == UnityXRPreInitRenderer::kUnityXRPreInitRendererGLCore )
		{
			m_VRSystem->GetOutputDevice( &graphicsAdapterId, vr::TextureType_OpenGL );
		}
		else if ( renderer == UnityXRPreInitRenderer::kUnityXRPreInitRendererMetal )
		{
			m_VRSystem->GetOutputDevice( &graphicsAdapterId, vr::TextureType_Metal );
		}
		else if ( renderer == UnityXRPreInitRenderer::kUnityXRPreInitRendererVulkan )
		{
			m_VRSystem->GetOutputDevice( &graphicsAdapterId, vr::TextureType_Vulkan, (VkInstance_T * )rendererData );
		}
		else
		{
			return false;
		}

        *adapterId = (uint64_t)(&graphicsAdapterId);
	}

	return graphicsAdapterId != 0;
}

bool OpenVRSystem::GetVulkanInstanceExtensions( void *userData, uint32_t namesCapacityIn, uint32_t *namesCountOut, char *namesString )
{
	if ( namesCapacityIn == 0 )
	{
		*namesCountOut = m_VRCompositor->GetVulkanInstanceExtensionsRequired( nullptr, 0 );
		return true;
	}

	assert( namesCapacityIn >= m_VRCompositor->GetVulkanInstanceExtensionsRequired( nullptr, 0 ) );
	assert( namesString != nullptr );

	m_VRCompositor->GetVulkanInstanceExtensionsRequired( namesString, namesCapacityIn );
	*namesCountOut = namesCapacityIn;

	return true;
}

bool OpenVRSystem::GetVulkanDeviceExtensions( void *userData, uint32_t namesCapacityIn, uint32_t *namesCountOut, char *namesString )
{
	if ( namesCapacityIn == 0 )
	{
		*namesCountOut = m_VRCompositor->GetVulkanDeviceExtensionsRequired( nullptr, nullptr, 0 );
		return true;
	}

	assert( namesCapacityIn >= m_VRCompositor->GetVulkanDeviceExtensionsRequired( nullptr, nullptr, 0 ) );
	assert( namesString != nullptr );

	m_VRCompositor->GetVulkanDeviceExtensionsRequired( nullptr, namesString, namesCapacityIn );
	*namesCountOut = namesCapacityIn;

	return true;
}

vr::EVRInitError OpenVRSystem::GetInitializationResult()
{
	return initError;
}

vr::IVRCompositor *OpenVRSystem::StartVRCompositor()
{
	vr::IVRCompositor *vrCompositor = ( vr::IVRCompositor * )vr::VR_GetGenericInterface( vr::IVRCompositor_Version, &initError );
	return vrCompositor;
}

extern "C" uint32_t UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
GetInitializationResult()
{
	return OpenVRSystem::Get().GetInitializationResult();
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
RegisterTickCallback( TickCallback newTickCallback )
{
	OpenVRSystem::Get().SetTickCallback( newTickCallback );
}