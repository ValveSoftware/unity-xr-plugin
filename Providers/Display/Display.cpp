#pragma once

#include "Display.h"
#include "Input/Input.h"
#include "UnityInterfaces.h"
#include "OpenVRProviderContext.h"
#include "CommonTypes.h"
#include "ProviderInterface/UnityXRDisplayStats.h"
#include "ProviderInterface/XRMath.h"
#include "UserProjectSettings.h"

#ifdef __linux__
#include <cstring>
#endif

// Interfaces
static IUnityXRDisplayInterface *s_pXRDisplay = nullptr;
static IUnityXRStats *s_pXRStats;
static UnitySubsystemHandle s_DisplayHandle;
static OpenVRProviderContext *s_pProviderContext;

// XR Stats
static UnityXRStatId m_nNumDroppedFrames;			// number of additional times previous frame was scanned out
static UnityXRStatId m_nNumFramePresents;			// number of times this frame was presented
static UnityXRStatId m_flFrameTimeReference;		// absolute time reference for comparing frames.This aligns with the vsync that running start is relative to.
static UnityXRStatId m_flGPURenderTimeInMs;			// time between work submitted immediately after present (ideally vsync) until the end of compositor submitted work
static UnityXRStatId m_flCPURenderTimeInMs;			// time spent on cpu submitting the above work for this frame
static UnityXRStatId m_flCPUIdelTimeInMs;			// time spent waiting for running start (application could have used this much more time)
static UnityXRStatId m_flCompositorRenderTimeInMs;	// time spend performing distortion correction, rendering chaperone, overlays, etc.
static UnityXRStatId m_flRefreshRate;

static UnitySubsystemErrorCode UNITY_INTERFACE_API GfxThread_Start( UnitySubsystemHandle handle, void *userData, UnityXRRenderingCapabilities *renderingCaps )
{
	OpenVRDisplayProvider *pDisplay = (OpenVRDisplayProvider * )userData;

	return pDisplay->GfxThread_Start( renderingCaps );
}


static UnitySubsystemErrorCode UNITY_INTERFACE_API GfxThread_PopulateNextFrameDesc( UnitySubsystemHandle handle, void *userData, const UnityXRFrameSetupHints *frameHints, UnityXRNextFrameDesc *nextFrame )
{
	OpenVRDisplayProvider *pDisplay = (OpenVRDisplayProvider * )userData;

	return pDisplay->GfxThread_PopulateNextFrameDesc( frameHints, nextFrame );
}


static UnitySubsystemErrorCode UNITY_INTERFACE_API GfxThread_SubmitCurrentFrame( UnitySubsystemHandle handle, void *userData )
{
	OpenVRDisplayProvider *pDisplay = (OpenVRDisplayProvider * )userData;

	return pDisplay->GfxThread_SubmitCurrentFrame();
}


static UnitySubsystemErrorCode UNITY_INTERFACE_API GfxThread_Stop( UnitySubsystemHandle handle, void *userData )
{
	OpenVRDisplayProvider *pDisplay = (OpenVRDisplayProvider * )userData;

	return pDisplay->GfxThread_Stop();
}


// Callback executed for rendering to editor preview
static UnitySubsystemErrorCode UNITY_INTERFACE_API GfxThread_BlitToMirrorViewRenderTarget( UnitySubsystemHandle handle, void *userData, const UnityXRMirrorViewBlitInfo mirrorBlitInfo )
{
	OpenVRDisplayProvider *pDisplay = (OpenVRDisplayProvider * )userData;

	return pDisplay->GfxThread_BlitToMirrorViewRenderTarget( &mirrorBlitInfo );
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API MainThread_UpdateDisplayState( UnitySubsystemHandle handle, void *userData, UnityXRDisplayState *state )
{
	OpenVRDisplayProvider *pDisplay = (OpenVRDisplayProvider * )userData;

	return pDisplay->MainThread_UpdateDisplayState( state );
}


static UnitySubsystemErrorCode UNITY_INTERFACE_API MainThread_QueryMirrorViewBlitDesc( UnitySubsystemHandle pHandle, void *pUserData, const UnityXRMirrorViewBlitInfo pMirrorBlitInfo, UnityXRMirrorViewBlitDesc *pBlitDescriptor )
{
	OpenVRDisplayProvider *pDisplay = (OpenVRDisplayProvider * )pUserData;

	return pDisplay->MainThread_QueryMirrorViewBlitDesc( &pMirrorBlitInfo, pBlitDescriptor, pDisplay );
}


// Callback executed when a subsystem should initialize in preparation for becoming active.
static UnitySubsystemErrorCode UNITY_INTERFACE_API Lifecycle_Initialize( UnitySubsystemHandle handle, void *userData )
{
	OpenVRSystem::Get().Initialize();

	OpenVRDisplayProvider *pDisplay = (OpenVRDisplayProvider * )userData;

	return pDisplay->Lifecycle_Initialize( handle, userData );
}


// Callback executed when a subsystem should become active.
static UnitySubsystemErrorCode UNITY_INTERFACE_API Lifecycle_Start( UnitySubsystemHandle handle, void *userData )
{
	OpenVRDisplayProvider *pDisplay = (OpenVRDisplayProvider * )userData;

	return pDisplay->Lifecycle_Start( handle );
}


// Callback executed when a subsystem should become inactive.
static void UNITY_INTERFACE_API Lifecycle_Stop( UnitySubsystemHandle handle, void *userData )
{
	OpenVRDisplayProvider *pDisplay = (OpenVRDisplayProvider * )userData;

	pDisplay->Lifecycle_Stop( handle );
}


// Callback executed when a subsystem should release all resources and is about to be unloaded.
static void UNITY_INTERFACE_API Lifecycle_Shutdown( UnitySubsystemHandle handle, void *userData )
{
	OpenVRDisplayProvider *pDisplay = (OpenVRDisplayProvider * )userData;

	pDisplay->Lifecycle_Shutdown( handle );

	OpenVRSystem::Get().Shutdown();
}


OpenVRDisplayProvider::OpenVRDisplayProvider() :
	m_nCurFrame( 0 ),
	m_bFrameInFlight( false ),
	m_bTexturesCreated( false )
{
	for ( int i = 0; i < k_nMaxNumStages; ++i )
	{
		for ( int j = 0; j < 2; ++j )
		{
			m_pNativeColorTextures[i][j] = nullptr;
			m_pNativeDepthTextures[i][j] = nullptr;
			m_UnityTextures[i][j] = 0;
		}
	}
}


OpenVRDisplayProvider::~OpenVRDisplayProvider()
{
}


UnitySubsystemErrorCode OpenVRDisplayProvider::Lifecycle_Initialize( UnitySubsystemHandle handle, void *userData )
{
	// Register handles
	s_DisplayHandle = handle;
	if ( s_pProviderContext )
	{
		s_pProviderContext->displayProvider = this;
	}

	// Register for callbacks on the graphics thread.
	UnityXRDisplayGraphicsThreadProvider gfxThreadProvider = { 0 };

	gfxThreadProvider.userData = this;
	gfxThreadProvider.Start = &::GfxThread_Start;
	gfxThreadProvider.PopulateNextFrameDesc = &::GfxThread_PopulateNextFrameDesc;
	gfxThreadProvider.SubmitCurrentFrame = &::GfxThread_SubmitCurrentFrame;
	gfxThreadProvider.BlitToMirrorViewRenderTarget = &::GfxThread_BlitToMirrorViewRenderTarget;
	gfxThreadProvider.Stop = &::GfxThread_Stop;

	if ( s_pXRDisplay->RegisterProviderForGraphicsThread( handle, &gfxThreadProvider ) != kUnitySubsystemErrorCodeSuccess )
		return  kUnitySubsystemErrorCodeFailure;

	// Register for callbacks on the main thread.
	UnityXRDisplayProvider mainThreadProvider = { 0 };

	mainThreadProvider.userData = this;
	mainThreadProvider.UpdateDisplayState = &::MainThread_UpdateDisplayState;
	mainThreadProvider.QueryMirrorViewBlitDesc = &::MainThread_QueryMirrorViewBlitDesc;

	if ( s_pXRDisplay->RegisterProvider( handle, &mainThreadProvider ) )
		return  kUnitySubsystemErrorCodeFailure;

	return kUnitySubsystemErrorCodeSuccess;
}


UnitySubsystemErrorCode OpenVRDisplayProvider::Lifecycle_Start( UnitySubsystemHandle handle )
{
	XR_TRACE( "XR OpenVR Display Start\n" );
	OpenVRSystem::Get();

	// We should have OpenVR and the compositor initialized here already
	if ( !vr::VRSystem() && !vr::VRCompositor() )
		return kUnitySubsystemErrorCodeFailure;

	m_nOpenVRMirrorAttempts = 0;
	m_bOverlayFallback = false;

	SetMirrorMode( UserProjectSettings::GetUnityMirrorViewMode() );

	// Reset previous mirror mode
	m_nPrevMirrorMode = m_nMirrorMode;

	// Setup mirror subrect defaults
	SetupMirror();

	return kUnitySubsystemErrorCodeSuccess;
}


void OpenVRDisplayProvider::Lifecycle_Stop( UnitySubsystemHandle handle )
{
	XR_TRACE( "[OpenVR] XR OpenVR Display Stop\n" );

	// Unregister XR Stats
	if ( s_pXRStats )
	{
		s_pXRStats->UnregisterStatSource( handle );
	}

	m_bFrameInFlight = false;
}


void OpenVRDisplayProvider::Lifecycle_Shutdown( UnitySubsystemHandle handle )
{
	XR_TRACE( "[OpenVR] XR OpenVR Display Shutdown\n" );

	// Release overlay view
	if ( vr::VROverlayView() )
	{
		vr::VROverlayView()->ReleaseOverlayView( &m_overlayView );
		ReleaseOverlayPointers();
	}

	// Destroy all eye textures
	DestroyEyeTextures( handle );

	// Explicitly reset member vars as Unity holds on to them in-between editor runs
	m_nCurFrame = 0;
	m_hOverlay = k_ulInvalidOverlayHandle;
	m_bTexturesCreated = false;
	m_bIsUsingCustomMirrorMode = false;
	m_bIsSteamVRViewAvailable = false;
	m_bIsIncorrectTexture = false;
	m_bIsHeadsetResolutionSet = false;
	OpenVRSystem::Get().SetTickCallback( nullptr );
}


UnitySubsystemErrorCode OpenVRDisplayProvider::GfxThread_Start( UnityXRRenderingCapabilities *renderingCaps )
{
	// Set active graphics renderer
	IUnityGraphics *pUnityGraphics = UnityInterfaces::Get().GetInterface< IUnityGraphics >();

	switch ( pUnityGraphics->GetRenderer() )
	{
	case kUnityGfxRendererD3D11:
		m_eActiveTextureType = vr::TextureType_DirectX;
		break;

	case kUnityGfxRendererVulkan:
		m_eActiveTextureType = vr::TextureType_Vulkan;
		break;

	case kUnityGfxRendererOpenGLCore:
	case kUnityGfxRendererOpenGLES30:
		m_eActiveTextureType = vr::TextureType_OpenGL;
		break;

	default:
		XR_TRACE( "[OpenVR] [Error] Unsupported graphics api. Only DirectX, OpenGL and Vulkan are supported at this time." );
		return kUnitySubsystemErrorCodeFailure;
		break;
	}

	// Check if we are using single pass
	m_bUseSinglePass = UserProjectSettings::GetStereoRenderingMode() == EVRStereoRenderingModes::SinglePassInstanced;

	// Setup rendering capabilities
	renderingCaps->noSinglePassRenderingSupport = false;
	renderingCaps->invalidateRenderStateAfterEachCallback = true;

	// Get occlusion mesh/hidden area mesh for both eyes (if available)
	m_pOcclusionMeshLeftEye = SetupOcclusionMesh( vr::Eye_Left );
	m_pOcclusionMeshRightEye = SetupOcclusionMesh( vr::Eye_Right );

	m_bIsOverlayApplication = UserProjectSettings::GetInitializationType() == vr::VRApplication_Overlay;

	return kUnitySubsystemErrorCodeSuccess;
}

void OpenVRDisplayProvider::TryUpdateMirrorMode( bool skipResolutionCheck )
{
	// Disregard mirror mode changes in the first frame
	if ( m_bIsHeadsetResolutionSet || skipResolutionCheck )
	{
		int requestedMirrorMode = UserProjectSettings::GetUnityMirrorViewMode();

		if ( requestedMirrorMode == kUnityXRMirrorBlitDistort && m_bOverlayFallback )
		{
			//don't try to set it again if we've already failed
		}
		else if ( requestedMirrorMode != GetCurrentMirrorMode() )
		{
			SetMirrorMode( requestedMirrorMode );
			SetupMirror();
		}
		else if ( m_nMirrorMode == kUnityXRMirrorBlitDistort )
		{
			if ( ( !m_bIsUsingCustomMirrorMode && m_bIsHeadsetResolutionSet )	// We need to pick up the shared texture if we're on SteamVR View mode after the first frame
				|| !HasOverlayPointer() )									// If getting the shared texture failed once, retry
			{
				SetupMirror();
			}
		}
	}

	m_nPrevMirrorMode = m_nMirrorMode;
}

UnitySubsystemErrorCode OpenVRDisplayProvider::GfxThread_PopulateNextFrameDesc( const UnityXRFrameSetupHints *frameHints, UnityXRNextFrameDesc *nextFrame )
{
	UnitySubsystemErrorCode ret = kUnitySubsystemErrorCodeSuccess;
	s_pProviderContext->inputProvider->GfxThread_UpdateDevices();
	m_bIsUsingRGB = frameHints->appSetup.sRGB;

	TryUpdateMirrorMode();

	// Check if engine requested a change of the viewport
	if ( ( kUnityXRFrameSetupHintsChangedRenderViewport & frameHints->changedFlags ) != 0 )
	{
		// Set new texture bounds and mirror subrect
		m_textureBounds.uMin = frameHints->appSetup.renderViewport.x;
		m_textureBounds.vMin = frameHints->appSetup.renderViewport.y;
		m_textureBounds.uMax = frameHints->appSetup.renderViewport.width;
		m_textureBounds.vMax = frameHints->appSetup.renderViewport.height;

		// Re-init mirror
		m_bIsRenderViewportScaling = false;
		SetupMirror();

		// Set render viewport scaling status
		if ( frameHints->appSetup.renderViewport.x > 0.0f || frameHints->appSetup.renderViewport.y > 0.0f
			|| frameHints->appSetup.renderViewport.width < 1.0f || frameHints->appSetup.renderViewport.height < 1.0f )
		{
			// Set mirror subrect from render target location
			m_mirrorRenderSubRect.x *= frameHints->appSetup.renderViewport.width;
			m_mirrorRenderSubRect.y *= frameHints->appSetup.renderViewport.height;
			m_mirrorRenderSubRect.width *= frameHints->appSetup.renderViewport.width;
			m_mirrorRenderSubRect.height *= frameHints->appSetup.renderViewport.height;

			m_bIsRenderViewportScaling = true;
		}
	}

	// Check if there was a request to change the texture resolution scale
	if ( ( kUnityXRFrameSetupHintsChangedTextureResolutionScale & frameHints->changedFlags ) != 0 )
	{
		if ( m_bTexturesCreated && s_DisplayHandle )
			DestroyEyeTextures( s_DisplayHandle );

		m_bTexturesCreated = false;
	}

	if ( !m_bTexturesCreated )
	{
		ret = CreateEyeTextures( frameHints );
	}


	// Calculate culling frustum
	if ( frameHints->appSetup.singlePassRendering )
	{
		// Single pass
		SetupCullingPass( 2, frameHints, nextFrame->cullingPasses[0] );
	}
	else
	{
		// Multi-pass
		SetupCullingPass( vr::Eye_Left, frameHints, nextFrame->cullingPasses[0] );
		SetupCullingPass( vr::Eye_Right, frameHints, nextFrame->cullingPasses[1] );
	}

	SetupRenderPass( vr::Eye_Left, frameHints, nextFrame );
	SetupRenderPass( vr::Eye_Right, frameHints, nextFrame );

	m_bFrameInFlight = true;

	// Set frame stats
	if ( s_pXRStats )
	{
		vr::Compositor_FrameTiming pTiming = {};
		pTiming.m_nSize = sizeof( vr::Compositor_FrameTiming );

		if ( vr::VRCompositor() && vr::VRCompositor()->GetFrameTiming( &pTiming, 0 ) )
		{
			// Unity XRStats in this version only exposes floats
			// Since we are in the gfx thread, we do NOT need to call increment frame here for XRStats
			s_pXRStats->SetStatFloat( m_nNumDroppedFrames, (float )pTiming.m_nNumDroppedFrames );
			s_pXRStats->SetStatFloat( m_nNumFramePresents, (float )pTiming.m_nNumFramePresents );
			s_pXRStats->SetStatFloat( m_flFrameTimeReference, (float )pTiming.m_flSystemTimeInSeconds );
			s_pXRStats->SetStatFloat( m_flGPURenderTimeInMs, pTiming.m_flTotalRenderGpuMs );
			s_pXRStats->SetStatFloat( m_flCPURenderTimeInMs, pTiming.m_flCompositorRenderCpuMs );
			s_pXRStats->SetStatFloat( m_flCPUIdelTimeInMs, pTiming.m_flCompositorIdleCpuMs );
			s_pXRStats->SetStatFloat( m_flCompositorRenderTimeInMs, pTiming.m_flCompositorRenderGpuMs );
            s_pXRStats->SetStatFloat( m_flRefreshRate, vr::VRSystem()->GetFloatTrackedDeviceProperty(0, 
				vr::ETrackedDeviceProperty::Prop_DisplayFrequency_Float));
		}
	}

	return ret;
}


UnitySubsystemErrorCode OpenVRDisplayProvider::GfxThread_SubmitCurrentFrame()
{
	if ( !m_bFrameInFlight )
		return kUnitySubsystemErrorCodeSuccess;

	// Get the stage for the current frame
	int stage = m_nCurFrame % m_nNumStages;

	// Advance frame number
	m_nCurFrame = ( m_nCurFrame < UINT32_MAX ) ? m_nCurFrame + 1 : 0;

	// Send eye textures for this stage to the compositor
	SubmitToCompositor( vr::Eye_Left, stage );
	SubmitToCompositor( vr::Eye_Right, stage );

	// Tell the compositor it can start rendering immediately
	if ( vr::VRCompositor() )
	{
		vr::VRCompositor()->PostPresentHandoff();
	}

	// Set the mirror resolution - should be done only ONCE and after at least ONE FRAME has been submitted
	if ( !m_bIsHeadsetResolutionSet && !m_bOverlayFallback && GetCurrentMirrorMode() == kUnityXRMirrorBlitDistort && vr::VRHeadsetView() )
	{
		// Get recommended mirror resolution
		UnityXRVector2 mirrorResolution = GetRecommendedMirrorResolution();
		uint32_t nRecommendedMirrorWidth = (uint32_t )mirrorResolution.x;
		uint32_t nRecommendedMirrorHeight = (uint32_t )mirrorResolution.y;

		// Get current mirror resolution
		uint32_t nCurrentMirrorWidth, nCurrentMirrorHeight;
		vr::VRHeadsetView()->GetHeadsetViewSize( &nCurrentMirrorWidth, &nCurrentMirrorHeight );

		// Set headset resolution if needed
		if ( nCurrentMirrorWidth != nRecommendedMirrorWidth || nCurrentMirrorHeight != nRecommendedMirrorHeight )
		{
			vr::VRHeadsetView()->SetHeadsetViewSize( nRecommendedMirrorWidth, nRecommendedMirrorHeight );
			vr::VRHeadsetView()->SetHeadsetViewCropped( true );
			XR_TRACE( "[OpenVR] [Mirror] Setting mirror view to %ix%i\n", nRecommendedMirrorWidth, nRecommendedMirrorHeight );
		}

		m_bIsHeadsetResolutionSet = true;
		vr::VRHeadsetView()->GetHeadsetViewSize( &nCurrentMirrorWidth, &nCurrentMirrorHeight );
		XR_TRACE( "[OpenVR] [Mirror] Mirror view set to %ix%i\n", nCurrentMirrorWidth, nCurrentMirrorHeight );
	}

	// Returning anything other than success here will shutdown the display provider (Unity SDK behavior)
	return kUnitySubsystemErrorCodeSuccess;
}


UnitySubsystemErrorCode OpenVRDisplayProvider::GfxThread_BlitToMirrorViewRenderTarget( const UnityXRMirrorViewBlitInfo *mirrorBlitInfo )
{
	#ifndef __linux__
	// Set RTV
	if ( XR_WIN && m_eActiveTextureType == vr::TextureType_DirectX )
	{
		ID3D11RenderTargetView *pRTV = s_pProviderContext->interfaces->Get<IUnityGraphicsD3D11>()->RTVFromRenderBuffer( mirrorBlitInfo->mirrorRtDesc->rtNative );
		ID3D11DeviceContext *pImmediateContext;
		( (ID3D11Device * )m_pRenderDevice )->GetImmediateContext( &pImmediateContext );
		pImmediateContext->OMSetRenderTargets( 1, &pRTV, NULL );

		// Clear out RTV
		const FLOAT clrColor[4] = { 0, 0, 0, 0 };
		pImmediateContext->ClearRenderTargetView( pRTV, clrColor );
	}
	#endif

	return kUnitySubsystemErrorCodeSuccess;
}


UnitySubsystemErrorCode OpenVRDisplayProvider::GfxThread_Stop()
{
	m_nCurFrame = 0;

	// Clean-up occlusion meshes
	if ( m_pOcclusionMeshLeftEye != 0 )
		s_pXRDisplay->DestroyOcclusionMesh( s_DisplayHandle, m_pOcclusionMeshLeftEye );

	if ( m_pOcclusionMeshRightEye != 0 )
		s_pXRDisplay->DestroyOcclusionMesh( s_DisplayHandle, m_pOcclusionMeshRightEye );

	return kUnitySubsystemErrorCodeSuccess;
}


UnitySubsystemErrorCode OpenVRDisplayProvider::MainThread_UpdateDisplayState( UnityXRDisplayState *state )
{
	// Check if reprojection is active
	vr::Compositor_FrameTiming frameTiming = {};
	frameTiming.m_nSize = sizeof( vr::Compositor_FrameTiming );

	if ( vr::VRCompositor()
		&& vr::VRCompositor()->GetFrameTiming( &frameTiming, 0 )
		&& frameTiming.m_nNumFramePresents > 1 )
	{
		state->reprojectionMode = kUnityXRReprojectionModePositionAndOrientation;
	}
	else
	{
		state->reprojectionMode = kUnityXRReprojectionModeNone;
	}

	state->displayIsTransparent = false;	// Only true if AR
	state->focusLost = vr::VRSystem() && vr::VRSystem()->ShouldApplicationPause() && !vr::VRCompositor()->CanRenderScene();

	return kUnitySubsystemErrorCodeSuccess;
}


UnitySubsystemErrorCode OpenVRDisplayProvider::MainThread_QueryMirrorViewBlitDesc( const UnityXRMirrorViewBlitInfo *pMirrorBlitInfo, UnityXRMirrorViewBlitDesc *pBlitDescriptor, OpenVRDisplayProvider *pDisplay )
{
	// Ensure that we can access the Overlay interface
	if ( !vr::VROverlay() )
		return kUnitySubsystemErrorCodeFailure;

	TryUpdateMirrorMode();

	// Set mirror subrect and aspect ratio for the steamvr mirror view
	float flSourceAspect = ( m_nEyeMirrorWidth * m_mirrorRenderSubRect.width ) / ( m_nRenderMirrorHeight * m_mirrorRenderSubRect.height );
	if ( m_nMirrorMode == kUnityXRMirrorBlitDistort && m_bIsSteamVRViewAvailable && m_bIsUsingCustomMirrorMode && m_bIsHeadsetResolutionSet )
	{
		// Full subrect if there's a valid mirror view
		m_mirrorRenderSubRect = { 0.0f, 0.0f, 1.0f, 1.0f };
		flSourceAspect = static_cast< float >( m_nRenderMirrorWidth ) / static_cast< float >( m_nRenderMirrorHeight );
	}

	// Set default mirror blit
	int stage = m_nCurFrame % m_nNumStages;
	int32_t nTextureArraySlice = 0;
	m_pMirrorTexture = m_UnityTextures[stage][0];

	// Check the current mirror mode
	if ( m_bIsHeadsetResolutionSet && m_nMirrorMode == kUnityXRMirrorBlitDistort && m_bIsUsingCustomMirrorMode && HasOverlayPointer() )
	{
		// SteamVR View - Grab the Overlay view for the SteamVR VR view
		vr::EVROverlayError eOverlayError = vr::VROverlayView()->AcquireOverlayView( m_hOverlay, &m_nativeDevice, &m_overlayView, sizeof( m_overlayView ) );
		if ( eOverlayError != vr::VROverlayError_None )
		{
			XR_TRACE( "[OpenVR] [Mirror] Fatal. Unable to acquire the SteamVR Display VR View overlay this frame [%i]\n", eOverlayError );
			return kUnitySubsystemErrorCodeFailure;
		}

		// Set the mirror texture to the SteamVR mirror texture
		m_pMirrorTexture = m_pSteamVRTextureId;
	}
	else
	{
		// Right eye texture (left is default)
		if ( m_nMirrorMode == kUnityXRMirrorBlitRightEye )
		{
			m_pMirrorTexture = m_UnityTextures[stage][m_bUseSinglePass ? 0 : 1];
			nTextureArraySlice = m_bUseSinglePass ? 1 : 0;
		}
	}


	// Get destination texture size and UV from current Editor/Player Window scale
	UnityXRVector2 vSourceUV0, vSourceUV1, vDestUV0, vDestUV1;
	const UnityXRVector2 vDestTextureSize = { static_cast< float >( pMirrorBlitInfo->mirrorRtDesc->rtScaledWidth ), static_cast< float >( pMirrorBlitInfo->mirrorRtDesc->rtScaledHeight ) };
	const UnityXRRectf vDestUVRect = ( m_nMirrorMode == kUnityXRMirrorBlitNone ) ? UnityXRRectf { 0.0f } : UnityXRRectf { 0.0f, 0.0f, 1.0f, 1.0f };

	// Calculate aspect ratios
	float flDestAspect = ( vDestTextureSize.x * vDestUVRect.width ) / ( vDestTextureSize.y * vDestUVRect.height );
	float flRatio = flSourceAspect / flDestAspect;

	// Adjust sub rects to fit texture into editor preview/player window
	UnityXRVector2 vSourceUVCenter = { m_mirrorRenderSubRect.x + m_mirrorRenderSubRect.width * 0.5f, m_mirrorRenderSubRect.y + m_mirrorRenderSubRect.height * 0.5f };
	UnityXRVector2 vSourceUVSize = { m_mirrorRenderSubRect.width, m_mirrorRenderSubRect.height };
	UnityXRVector2 vDestUVCenter = { vDestUVRect.x + vDestUVRect.width * 0.5f, vDestUVRect.y + vDestUVRect.height * 0.5f };
	UnityXRVector2 vDestUVSize = { vDestUVRect.width, vDestUVRect.height };

	if ( flRatio > 1.0f )
	{
		vSourceUVSize.x /= flRatio;
	}
	else
	{
		vSourceUVSize.y *= flRatio;
	}

	vSourceUV0 = { vSourceUVCenter.x - ( vSourceUVSize.x * 0.5f ), vSourceUVCenter.y - ( vSourceUVSize.y * 0.5f ) };
	vSourceUV1 = { vSourceUV0.x + vSourceUVSize.x, vSourceUV0.y + vSourceUVSize.y };
	vDestUV0 = { vDestUVCenter.x - vDestUVSize.x * 0.5f, vDestUVCenter.y - vDestUVSize.y * 0.5f };
	vDestUV1 = { vDestUV0.x + vDestUVSize.x, vDestUV0.y + vDestUVSize.y };

	// Set blit params
	( *pBlitDescriptor ).blitParamsCount = 1;
	( *pBlitDescriptor ).blitParams[0].srcTexId = m_pMirrorTexture;
	( *pBlitDescriptor ).blitParams[0].srcTexArraySlice = nTextureArraySlice;
	( *pBlitDescriptor ).blitParams[0].srcRect = { vSourceUV0.x, vSourceUV0.y, vSourceUV1.x - vSourceUV0.x, vSourceUV1.y - vSourceUV0.y };
	( *pBlitDescriptor ).blitParams[0].destRect = { vDestUV0.x, vDestUV0.y, vDestUV1.x - vDestUV0.x, vDestUV1.y - vDestUV0.y };

	return kUnitySubsystemErrorCodeSuccess;
}


static void OpenVRMatrix3x4ToUnity( const vr::HmdMatrix34_t &ovrm, UnityXRMatrix4x4 &out )
{
	float flTmpMatrix[] =
	{
		ovrm.m[0][0], ovrm.m[1][0], ovrm.m[2][0], 0.0f,
		ovrm.m[0][1], ovrm.m[1][1], ovrm.m[2][1], 0.0f,
		ovrm.m[0][2], ovrm.m[1][2], ovrm.m[2][2], 0.0f,
		ovrm.m[0][3], ovrm.m[1][3], ovrm.m[2][3], 1.0f
	};

	out.columns[0].x = flTmpMatrix[( 0 * 4 ) + 0];
	out.columns[1].x = flTmpMatrix[( 1 * 4 ) + 0];
	out.columns[2].x = -flTmpMatrix[( 2 * 4 ) + 0];
	out.columns[3].x = flTmpMatrix[( 3 * 4 ) + 0];

	out.columns[0].y = flTmpMatrix[( 0 * 4 ) + 1];
	out.columns[1].y = flTmpMatrix[( 1 * 4 ) + 1];
	out.columns[2].y = -flTmpMatrix[( 2 * 4 ) + 1];
	out.columns[3].y = flTmpMatrix[( 3 * 4 ) + 1];

	out.columns[0].z = -flTmpMatrix[( 0 * 4 ) + 2];
	out.columns[1].z = -flTmpMatrix[( 1 * 4 ) + 2];
	out.columns[2].z = flTmpMatrix[( 2 * 4 ) + 2];
	out.columns[3].z = -flTmpMatrix[( 3 * 4 ) + 2];

	out.columns[0].w = flTmpMatrix[( 0 * 4 ) + 3];
	out.columns[1].w = flTmpMatrix[( 1 * 4 ) + 3];
	out.columns[2].w = -flTmpMatrix[( 2 * 4 ) + 3];
	out.columns[3].w = flTmpMatrix[( 3 * 4 ) + 3];
}


void OpenVRDisplayProvider::SetupMirror()
{
	// Set XR Stats
	if ( s_pXRStats )
	{
		s_pXRStats->RegisterStatSource( s_DisplayHandle );
		m_nNumDroppedFrames = s_pXRStats->RegisterStatDefinition( s_DisplayHandle, kUnityStatsDroppedFrameCount, kUnityXRStatOptionNone );
		m_nNumFramePresents = s_pXRStats->RegisterStatDefinition( s_DisplayHandle, kUnityStatsFramePresentCount, kUnityXRStatOptionNone );
		m_flFrameTimeReference = s_pXRStats->RegisterStatDefinition( s_DisplayHandle, "OpenVR.FrameTimeReferenceSecs", kUnityXRStatOptionNone );
		m_flCPURenderTimeInMs = s_pXRStats->RegisterStatDefinition( s_DisplayHandle, kUnityStatsGPUTimeApp, kUnityXRStatOptionNone );
		m_flCPUIdelTimeInMs = s_pXRStats->RegisterStatDefinition( s_DisplayHandle, "OpenVR.CPUIdleTimeMs", kUnityXRStatOptionNone );
		m_flCompositorRenderTimeInMs = s_pXRStats->RegisterStatDefinition( s_DisplayHandle, kUnityStatsGPUTimeCompositor, kUnityXRStatOptionNone );
        m_flRefreshRate = s_pXRStats->RegisterStatDefinition( s_DisplayHandle, kUnityStatsDisplayRefreshRate, kUnityXRStatOptionNone );
	}


	// Get recommended render target size based on currently active hmd
	vr::VRSystem()->GetRecommendedRenderTargetSize( &m_nEyeWidth, &m_nEyeHeight );
	m_nEyeMirrorWidth = m_nEyeWidth;
	m_nEyeMirrorHeight = m_nEyeHeight;

	// Set default mirror render subrect extents
	if ( !m_bIsRenderViewportScaling )
	{
		m_mirrorRenderSubRect.x = k_flDefaultSubRectX;
		m_mirrorRenderSubRect.y = k_flDefaultSubRectY;
		m_mirrorRenderSubRect.width = k_flDefaultSubRectWidth;
		m_mirrorRenderSubRect.height = k_flDefaultSubRectHeight;
	}

	// Set default render subrects
	if ( m_bIsUsingCustomMirrorMode && m_nMirrorMode == kUnityXRMirrorBlitDistort )
	{
		m_mirrorRenderSubRect = { 0.0f, 0.0f, 1.0f, 1.0f };
	}
	else
	{
		UnityXRVector2 mirrorResolution = GetRecommendedMirrorResolution();
		m_nRenderMirrorWidth = (uint32_t )mirrorResolution.x;
		m_nRenderMirrorHeight = (uint32_t )mirrorResolution.y;

		if ( m_nRenderMirrorWidth > 0
			&& m_nRenderMirrorHeight > 0
			&& m_nRenderMirrorWidth < m_nEyeWidth
			&& m_nRenderMirrorHeight < m_nEyeHeight
			)
		{
			float flRenderWidth = (float )m_nRenderMirrorWidth;
			float flRenderHeight = (float )m_nRenderMirrorHeight;
			float flEyeWidth = (float )m_nEyeWidth;
			float flEyeHeight = (float )m_nEyeHeight;

			float aspectRatio = flRenderWidth / flRenderHeight;
			m_mirrorRenderSubRect.x = ( flEyeWidth - flRenderWidth ) / flEyeWidth;
			m_mirrorRenderSubRect.y = 1.0f - ( ( flEyeHeight - flRenderHeight ) / flEyeHeight );
			m_mirrorRenderSubRect.width = 1.0f - m_mirrorRenderSubRect.x;
			m_mirrorRenderSubRect.height = m_mirrorRenderSubRect.width / aspectRatio;
		}
	}

	// Do not open shared texture until we've actually sent one frame to the compositor
	if ( !m_bIsHeadsetResolutionSet )
		return;

	if ( m_nMirrorMode == kUnityXRMirrorBlitDistort )
	{
		SetupOverlayMirror();
	}
}

bool OpenVRDisplayProvider::SubmitToCompositor( vr::EVREye eEye, int nStage )
{
	if ( !vr::VRCompositor() )
		return false;


	// Use the left eye texture if we're doing a single pass
	int nTexIndex = m_bUseSinglePass ? vr::Eye_Left : eEye;

	// Check for Vulkan support
	if ( m_eActiveTextureType == vr::TextureType_Vulkan )
	{
		if ( s_pProviderContext->interfaces->Get<IUnityGraphicsVulkan>()->AccessTexture( GetNativeEyeTexture( nStage, nTexIndex ),
			UnityVulkanWholeImage,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			kUnityVulkanResourceAccess_ObserveOnly,
			&m_unityVulkanImage )
			)
		{
			// Vulkan image information
			m_vrVulkanTexture.m_nImage = (uint64_t )m_unityVulkanImage.image;
			m_vrVulkanTexture.m_nWidth = m_unityVulkanImage.extent.width;
			m_vrVulkanTexture.m_nHeight = m_unityVulkanImage.extent.height;
			m_vrVulkanTexture.m_nFormat = m_unityVulkanImage.format;
			m_vrVulkanTexture.m_nSampleCount = m_unityVulkanImage.samples;
			m_vrVulkanTexture.m_pPhysicalDevice = s_pProviderContext->interfaces->Get<IUnityGraphicsVulkan>()->Instance().physicalDevice;
			m_vrVulkanTexture.m_pDevice = s_pProviderContext->interfaces->Get<IUnityGraphicsVulkan>()->Instance().device;
			m_vrVulkanTexture.m_pInstance = s_pProviderContext->interfaces->Get<IUnityGraphicsVulkan>()->Instance().instance;
			m_vrVulkanTexture.m_pQueue = s_pProviderContext->interfaces->Get<IUnityGraphicsVulkan>()->Instance().graphicsQueue;
			m_vrVulkanTexture.m_nQueueFamilyIndex = s_pProviderContext->interfaces->Get<IUnityGraphicsVulkan>()->Instance().queueFamilyIndex;

			// Array specific data
			m_vrVulkanTexture.m_unArraySize = m_bUseSinglePass ? 2 : 1;
			m_vrVulkanTexture.m_unArrayIndex = ( m_bUseSinglePass && eEye == vr::Eye_Right ) ? 1 : 0;
		}
		else
		{
			XR_TRACE( "[OpenVR] [Error] Unable to get Vulkan texture for stage %i and eye %i\n", nStage, eEye );
			return false;
		}
	}

	// Grab the correct texture for this stage
	vr::VRTextureWithDepth_t tex;
	tex.handle = m_eActiveTextureType == vr::TextureType_Vulkan ? &m_vrVulkanTexture : GetNativeEyeTexture( nStage, nTexIndex );
	tex.eType = m_eActiveTextureType;
	tex.eColorSpace = vr::ColorSpace_Auto;

	// Check if we have a valid depth buffer
	if (m_pNativeDepthTextures[nStage][nTexIndex])
	{
		tex.depth.handle = m_pNativeDepthTextures[nStage][nTexIndex];
	}

	if ( !m_bIsOverlayApplication )
	{
		// OpenVR submission flags
		vr::EVRSubmitFlags nFlags = m_eActiveTextureType == vr::TextureType_Vulkan ? vr::Submit_VulkanTextureWithArrayData : vr::Submit_Default;

		// Submit the texture to the Compositor
		vr::EVRCompositorError res = vr::VRCompositorError_None;
		res = vr::VRCompositor()->Submit( eEye, &tex, &m_textureBounds, nFlags );

		if ( res != vr::VRCompositorError_None )
		{
			XR_TRACE( "[OpenVR] [Error] Unable to submit eye texture: [%i] [%x]\n", res, tex.handle );
			return false;
		}
	}

	return true;
}


void OpenVRDisplayProvider::SetupRenderPass( const vr::EVREye eEye, const UnityXRFrameSetupHints *pFrameHints, UnityXRNextFrameDesc *pTargetFrame )
{
	// Setup vars for the render params
	uint32_t nParamsCount;
	uint32_t nRenderPasses;

	int32_t nRenderPassesCount;
	int32_t nRenderParamsCount;
	int32_t nTextureArraySlice;

	if ( m_bUseSinglePass )
	{
		// Single pass and single pass instance values
		nParamsCount = eEye;
		nRenderPasses = 0;

		nRenderPassesCount = 1;
		nRenderParamsCount = 2;
		nTextureArraySlice = eEye;
	}
	else
	{
		// Multi-pass values
		nParamsCount = 0;
		nRenderPasses = eEye;

		nRenderPassesCount = 2;
		nRenderParamsCount = 1;
		nTextureArraySlice = 0;
	}

	// Set the number of render passes for this frame
	pTargetFrame->renderPassesCount = nRenderPassesCount;

	// Setup base render pass properties
	UnityXRNextFrameDesc::UnityXRRenderPass &renderPass = pTargetFrame->renderPasses[nRenderPasses];
	renderPass.textureId = m_UnityTextures[m_nCurFrame % m_nNumStages][nRenderPasses];
	renderPass.renderParamsCount = nRenderParamsCount;
	renderPass.cullingPassIndex = nRenderPasses;

	// Setup base render pass parameters
	UnityXRNextFrameDesc::UnityXRRenderPass::UnityXRRenderParams &renderParams = renderPass.renderParams[nParamsCount];
	renderParams.deviceAnchorToEyePose = GetEyePose( eEye );
	renderParams.projection = GetProjection( eEye, pFrameHints->appSetup.zNear, pFrameHints->appSetup.zFar );
	renderParams.viewportRect = { 0.0f, 0.0f, 1.0f, 1.0f };
	renderParams.textureArraySlice = nTextureArraySlice;

	// Set an occlusion mesh (hidden area mesh) if there's a valid one
	if ( eEye == vr::Eye_Left && m_pOcclusionMeshLeftEye != 0 )
	{
		renderParams.occlusionMeshId = m_pOcclusionMeshLeftEye;
	}
	else if ( eEye == vr::Eye_Right && m_pOcclusionMeshRightEye != 0 )
	{
		renderParams.occlusionMeshId = m_pOcclusionMeshRightEye;
	}
}


UnityXROcclusionMeshId OpenVRDisplayProvider::SetupOcclusionMesh( vr::EVREye eEye )
{
	if ( !vr::VRSystem() )
		return k_nInvalidUnityXROcclusionMeshId;

	// Grab the hidden area mesh from SteamVR
	vr::HiddenAreaMesh_t vrHiddenMesh = vr::VRSystem()->GetHiddenAreaMesh( eEye, vr::k_eHiddenAreaMesh_Standard );

	if ( vrHiddenMesh.pVertexData == NULL || vrHiddenMesh.unTriangleCount == 0 )
	{
		XR_TRACE( "[OpenVR] No hidden area mesh available for eye[%i] in active hmd\n", eEye );
		return k_nInvalidUnityXROcclusionMeshId;
	}

	// Setup our vertex and index arrays
	std::vector< uint32_t > vIndices( (long long )vrHiddenMesh.unTriangleCount * (long long )3, UINT32_MAX );
	std::vector< UnityXRVector2 > vVertices;
	vVertices.reserve( vIndices.size() );

	for ( size_t v1 = 0; v1 < vIndices.size(); v1++ )										// Go through each vertex from OpenVR (may contain duplicates)
	{
		if ( vIndices[v1] != UINT32_MAX )                                                 // Skip if already assigned (i.e. was found to be a duplicate)
			continue;

		uint32_t nIndex = (uint32_t )vVertices.size();										// Record new index
		vIndices[v1] = nIndex;

		const auto &v = vrHiddenMesh.pVertexData[v1].v;									// Keep this vertex
		vVertices.push_back( { v[0], v[1] } );

		for ( size_t v2 = v1 + 1; v2 < vVertices.size(); v2++ )								// Check remaining for duplicate vertices
		{
			static const float k_flThreshold = 1e-9f;
			if ( GetDistanceSquared2D( v, vrHiddenMesh.pVertexData[v2].v ) < k_flThreshold )
			{
				vIndices[v2] = nIndex;                                                      // Remap this index to point to similar earlier vertex
			}
		}
	}

	// Create a Unity occlusion mesh
	UnityXROcclusionMeshId pOcclusionMeshId;
	UnitySubsystemErrorCode res = s_pXRDisplay->CreateOcclusionMesh( s_DisplayHandle, (uint32_t )vVertices.size(),
		(uint32_t )vIndices.size(), &pOcclusionMeshId );

	if ( res != kUnitySubsystemErrorCodeSuccess )
	{
		XR_TRACE( "[OpenVR] Error creating occlusion mesh for eye[%i]: [%i]\n", eEye, res );
		return k_nInvalidUnityXROcclusionMeshId;
	}

	// Setup the Unity occlusion mesh
	res = s_pXRDisplay->SetOcclusionMesh( s_DisplayHandle, pOcclusionMeshId, vVertices.data(), (uint32_t )vVertices.size(),
		vIndices.data(), (uint32_t )vIndices.size() );

	if ( res != kUnitySubsystemErrorCodeSuccess )
	{
		XR_TRACE( "[OpenVR] Error creating occlusion mesh for eye[%i]: [%i]\n", eEye, res );
		return k_nInvalidUnityXROcclusionMeshId;
	}

	// Finally, return the occlusion mesh id to caller
	return pOcclusionMeshId;
}


float OpenVRDisplayProvider::GetDistanceSquared2D( const float *pVector1, const float *pVector2 )
{
	float flDiffX = pVector2[0] - pVector1[0];
	float flDiffY = pVector2[1] - pVector1[1];

	return ( flDiffX * flDiffX ) + ( flDiffY * flDiffY );
}


const UnityXRVector2 OpenVRDisplayProvider::GetRecommendedMirrorResolution()
{
	float flWidth = vr::k_unHeadsetViewMaxWidth;
	float flHeight = vr::k_unHeadsetViewMaxHeight;
	//flHeight = flWidth / k_flDockedViewAspecRatio;

	return UnityXRVector2 { flWidth, flHeight };
}


UnityXRPose OpenVRDisplayProvider::GetEyePose( int eye )
{
	UnityXRPose ret = { 0 };

	if ( !vr::VRSystem() )
		return ret;

	// Set identity matrix
	vr::HmdMatrix34_t vrMat = { { { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f } } };

	if ( eye > vr::Eye_Right )
	{
		// add depth information to identity matrix that'll serve as "middle eye" pose
		float fHeadToEyeDepth = vr::VRSystem()->GetFloatTrackedDeviceProperty( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_UserHeadToEyeDepthMeters_Float, nullptr );
		vrMat.m[2][3] = fHeadToEyeDepth;
	}
	else
	{
		// Get eye specific pose
		vrMat = vr::VRSystem()->GetEyeToHeadTransform( ( eye == 0 ) ? vr::Eye_Left : vr::Eye_Right );
	}

	UnityXRMatrix4x4 mat;
	OpenVRMatrix3x4ToUnity( vrMat, mat );

	XRVector3 pos;
	XRQuaternion quat;

	MatrixToTranslationRotation( reinterpret_cast< XRMatrix4x4 & >( mat ), reinterpret_cast< XRVector3 & >( ret.position ), reinterpret_cast< XRQuaternion & >( ret.rotation ) );

	return ret;
}


UnityXRProjection OpenVRDisplayProvider::GetProjection( int eye, float flNear, float flFar )
{
	UnityXRProjection ret;
	ret.type = kUnityXRProjectionTypeMatrix;

	if ( !vr::VRSystem() )
		return ret;

	float vrL, vrR, vrT, vrB;

	if ( eye > vr::Eye_Right )
	{
		// Calculate combined left + right eye combined projection
		float leftVrL, leftVrR, leftVrT, leftVrB, rightVrL, rightVrR, rightVrT, rightVrB;

		// Get each eye's projection
		vr::VRSystem()->GetProjectionRaw( vr::Eye_Left, &leftVrL, &leftVrR, &leftVrT, &leftVrB );
		vr::VRSystem()->GetProjectionRaw( vr::Eye_Right, &rightVrL, &rightVrR, &rightVrT, &rightVrB );

		// Use the max extent's for each eye 
		vrL = leftVrL;
		vrR = rightVrR;
		vrT = rightVrT;
		vrB = leftVrB;

		//XR_TRACE( "COMBINED EYE PROJECTION: %f %f %f %f\nNear: %f Far: %f\n", vrL, vrR, vrT, vrB, flNear, flFar );
	}
	else
	{
		// Calculate eye specific projection
		vr::VRSystem()->GetProjectionRaw( ( eye == 0 ) ? vr::Eye_Left : vr::Eye_Right, &vrL, &vrR, &vrT, &vrB );

		//XR_TRACE( "%i EYE PROJECTION: %f %f %f %f\nNear: %f Far: %f\n", eye, vrL, vrR, vrT, vrB, flNear, flFar );
	}

	float x, y, a, b, c, d, e;

	float nearval = flNear < k_flNear ? k_flNear : flNear;
	float farval = flFar < k_flNear ? k_flFar : flFar;
	float left = vrL * nearval;
	float right = vrR * nearval;
	float top = vrB * nearval;
	float bottom = vrT * nearval;

	x = ( 2.0F * nearval ) / ( right - left );
	y = ( 2.0F * nearval ) / ( top - bottom );
	a = ( right + left ) / ( right - left );
	b = ( top + bottom ) / ( top - bottom );
	c = -( farval + nearval ) / ( farval - nearval );
	d = -( 2.0f * farval * nearval ) / ( farval - nearval );
	e = -1.0f;

	ret.data.matrix.columns[0].x = x;    ret.data.matrix.columns[1].x = 0.0f;  ret.data.matrix.columns[2].x = a;   ret.data.matrix.columns[3].x = 0.0f;
	ret.data.matrix.columns[0].y = 0.0f;  ret.data.matrix.columns[1].y = y;    ret.data.matrix.columns[2].y = b;   ret.data.matrix.columns[3].y = 0.0f;
	ret.data.matrix.columns[0].z = 0.0f;  ret.data.matrix.columns[1].z = 0.0f;  ret.data.matrix.columns[2].z = c;   ret.data.matrix.columns[3].z = d;
	ret.data.matrix.columns[0].w = 0.0f;  ret.data.matrix.columns[1].w = 0.0f;  ret.data.matrix.columns[2].w = e;   ret.data.matrix.columns[3].w = 0.0f;

	return ret;
}


void OpenVRDisplayProvider::SetupCullingPass( int eye, const UnityXRFrameSetupHints *frameHints, UnityXRNextFrameDesc::UnityXRCullingPass &cullingPass )
{
	if ( !vr::VRSystem() )
		return;

	float separation = 0;

	// get the actual separation; IPD in meters 
	vr::ETrackedPropertyError err;
	separation = vr::VRSystem()->GetFloatTrackedDeviceProperty( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_UserIpdMeters_Float, &err );

	if ( separation == 0 || err != vr::TrackedProp_Success )
	{
		separation = 0.0625f; // default camera separation in legacy system
	}
	cullingPass.separation = separation;

	UnityXRPose pose = GetEyePose( eye );
	UnityXRProjection projection = GetProjection( eye, frameHints->appSetup.zNear, frameHints->appSetup.zFar );

	float aspect = projection.data.matrix.columns[1].y / projection.data.matrix.columns[0].x;

	float vertFov = RAD2DEG * ( 2.0f * (float )atan( 1.0f / projection.data.matrix.columns[1].y ) );
	float eyePullback = 0.5f * separation / tanf( ( 0.5f * vertFov * aspect ) * DEG2RAD );

	pose.position.z = pose.position.z - eyePullback;
	cullingPass.deviceAnchorToCullingPose = pose;
	cullingPass.projection = projection;
}


UnitySubsystemErrorCode OpenVRDisplayProvider::CreateEyeTextures( const UnityXRFrameSetupHints *frameHints )
{
	if ( !vr::VRSystem() )
		return kUnitySubsystemErrorCodeSuccess;	// Anything other than success will shutdown the provider, we probably don't want to do that here

	// One texture per eye, per stage
	int nNumTextures = 2;

	// Grab texture size from OpenVR based on currently active HMD
	uint32_t eyeWidth, eyeHeight;
	vr::VRSystem()->GetRecommendedRenderTargetSize( &eyeWidth, &eyeHeight );

	// Apply scale
	float eyeWidthScaled = eyeWidth * frameHints->appSetup.textureResolutionScale;
	float eyeHeightScaled = eyeHeight * frameHints->appSetup.textureResolutionScale;
	eyeWidth = (uint32_t )eyeWidthScaled;
	eyeHeight = (uint32_t )eyeHeightScaled;

	// Create textures
	for ( int stage = 0; stage < m_nNumStages; ++stage )
	{
		for ( int eye = 0; eye < nNumTextures; ++eye )
		{
			UnityXRRenderTextureDesc unityDesc;
			memset( &unityDesc, 0, sizeof( UnityXRRenderTextureDesc ) );
			unityDesc.colorFormat = kUnityXRRenderTextureFormatRGBA32;
			unityDesc.depthFormat = kUnityXRDepthTextureFormat24bitOrGreater;
			unityDesc.color.nativePtr = (void * )kUnityXRRenderTextureIdDontCare;
			unityDesc.depth.nativePtr = (void * )kUnityXRRenderTextureIdDontCare;
			unityDesc.width = eyeWidth;
			unityDesc.height = eyeHeight;

			if ( m_bIsUsingRGB )
			{
				unityDesc.flags |= kUnityXRRenderTextureFlagsSRGB;
			}

			if ( m_bUseSinglePass )
			{
				unityDesc.textureArrayLength = 2;
			}

			// Create an UnityXRRenderTextureId for the native texture so we can tell unity to render to it later.
			UnityXRRenderTextureId unityTexId;
			UnitySubsystemErrorCode res = s_pXRDisplay->CreateTexture( s_DisplayHandle, &unityDesc, &unityTexId );
			if ( res != kUnitySubsystemErrorCodeSuccess )
			{
				XR_TRACE( "[OpenVR] Error creating texture: [%i]\n", res );
				return res;
			}

			m_UnityTextures[stage][eye] = unityTexId;
			m_pNativeColorTextures[stage][eye] = nullptr;    // this is just registering a creation request, we'll grab the native textures later
			m_pNativeDepthTextures[stage][eye] = nullptr;
		}
	}

	m_bTexturesCreated = true;
	return kUnitySubsystemErrorCodeSuccess;
}


void OpenVRDisplayProvider::DestroyEyeTextures( UnitySubsystemHandle handle )
{
	for ( int i = 0; i < m_nNumStages; ++i )
	{
		for ( int eye = 0; eye < 2; ++eye )
		{
			if ( m_UnityTextures[i][eye] != 0 )
			{
				s_pXRDisplay->DestroyTexture( handle, m_UnityTextures[i][eye] );
			}
		}
	}

	m_bTexturesCreated = false;
}


void *OpenVRDisplayProvider::GetNativeEyeTexture( int stage, int eye )
{
	if ( m_pNativeColorTextures[stage][eye] == nullptr )
	{
		UnityXRRenderTextureId unityTexId = m_UnityTextures[stage][eye];

		UnityXRRenderTextureDesc unityDesc;
		memset( &unityDesc, 0, sizeof( UnityXRRenderTextureDesc ) );

		UnitySubsystemErrorCode res = s_pXRDisplay->QueryTextureDesc( s_DisplayHandle, unityTexId, &unityDesc );
		if ( res != kUnitySubsystemErrorCodeSuccess )
		{
			XR_TRACE( "[OpenVR] Error querying texture: [%i]\n", res );
			return nullptr;
		}

		m_pNativeColorTextures[stage][eye] = unityDesc.color.nativePtr;
		XR_TRACE( "[OpenVR] Created Native Color: %x\n", unityDesc.color.nativePtr );

		m_pNativeDepthTextures[stage][eye] = unityDesc.depth.nativePtr;
		XR_TRACE( "[OpenVR] Created Native Depth: %x\n", unityDesc.depth.nativePtr );
	}

	return m_pNativeColorTextures[stage][eye];
}


void OpenVRDisplayProvider::ReleaseOverlayPointers()
{
	#ifndef __linux__
	m_pMirrorTextureDX = nullptr;
	#endif
}

bool OpenVRDisplayProvider::HasOverlayPointer()
{
	#ifndef __linux__
	return m_pMirrorTextureDX != nullptr;
	#else
	return false;
	#endif
}

void OpenVRDisplayProvider::SetupOverlayMirror()
{
	#ifndef __linux__
	// Acquire the SteamVR Display VR View overlay 
	vr::EVROverlayError eOverlayError;
	if ( m_nMirrorMode == kUnityXRMirrorBlitDistort && !m_bOverlayFallback && m_hOverlay == k_ulInvalidOverlayHandle && !m_bIsUsingCustomMirrorMode && vr::VROverlay() )
	{
		eOverlayError = vr::VROverlay()->FindOverlay( vr::k_pchHeadsetViewOverlayKey, &m_hOverlay );
		if ( eOverlayError != vr::VROverlayError_None )
		{
			XR_TRACE( "[OpenVR] [Mirror] Failed to find the SteamVR Display VR View overlay [%i]\n", eOverlayError );
			m_bIsUsingCustomMirrorMode = false;
		}
		else
		{
			XR_TRACE( "[OpenVR] [Mirror] Mirror overlay found [%i]\n", eOverlayError );
		}
	}

	ID3D11Device *m_pD3D11Device;
	if ( m_eActiveTextureType == vr::TextureType_DirectX )
	{
		// Grab the active render device
		m_pD3D11Device = s_pProviderContext->interfaces->Get< IUnityGraphicsD3D11 >()->GetDevice();

		// Create a native device handle for OpenVR
		m_nativeDevice.eType = vr::DeviceType_DirectX11;
		m_nativeDevice.handle = m_pD3D11Device;
	}
	else
	{
		m_nativeDevice.handle = nullptr;
		m_pD3D11Device = nullptr;
	}
	
	// Create a native device handle for OpenVR
	m_nativeDevice.eType = vr::DeviceType_DirectX11;
	m_nativeDevice.handle = m_pD3D11Device;

	// Get the active overlay view - should be our scene application now at this stage
	if ( m_nMirrorMode == kUnityXRMirrorBlitDistort && !m_bOverlayFallback && m_hOverlay != k_ulInvalidOverlayHandle && m_bIsUsingRGB && !m_bIsUsingCustomMirrorMode && vr::VRSystem() && vr::VROverlayView() )
	{
		// Grab the Overlay view for the SteamVR VR view
		eOverlayError = vr::VROverlayView()->AcquireOverlayView( m_hOverlay, &m_nativeDevice, &m_overlayView, sizeof( m_overlayView ) );
		if ( eOverlayError != vr::VROverlayError_None )
		{
			XR_TRACE( "[OpenVR] [Mirror] Unable to acquire the SteamVR Display VR View overlay [%i]\n", eOverlayError );
			m_hOverlay = k_ulInvalidOverlayHandle;
		}
		else
		{
			XR_TRACE( "[OpenVR] [Mirror] Mirror view overlay acquired [%i]\n", eOverlayError );

			if ( !m_overlayView.texture.handle )
			{
				XR_TRACE( "[OpenVR] [Mirror] No valid texture for the overlay view was found\n" );
			}
		}
	}

	if ( m_bIsUsingRGB									// Unity only supports RGB textures for the mirror mode in linear mode
		&& m_nMirrorMode == kUnityXRMirrorBlitDistort	// kUnityXRMirrorBlitDistort is the SteamVR View
		&& m_bIsHeadsetResolutionSet					// The headset resolution must be set before we attempt to open the shared texture
		&& !m_bIsUsingCustomMirrorMode					// Check if the shared texture has already been opened
		&& !m_bIsIncorrectTexture
		&& m_overlayView.texture.handle
		&& m_hOverlay != k_ulInvalidOverlayHandle
		&& m_pD3D11Device
		)
	{
		// Get current mirror resolution
		if ( vr::VRHeadsetView() )
		{
			uint32_t nCurrentMirrorWidth, nCurrentMirrorHeight;
			vr::VRHeadsetView()->GetHeadsetViewSize( &nCurrentMirrorWidth, &nCurrentMirrorHeight );
			XR_TRACE( "[OpenVR] [Mirror] Mirror view set to %ix%i\n", nCurrentMirrorWidth, nCurrentMirrorHeight );
		}

		// Attempt to open shared texture
		XR_TRACE( "[OpenVR] [Mirror] Attempting to open shared texture...\n" );
		if ( SUCCEEDED( m_pD3D11Device->OpenSharedResource( m_overlayView.texture.handle, __uuidof( ID3D11Texture2D ), (void ** )& m_pMirrorTextureDX ) ) )
		{
			// Convert the overlay texture to a Texture2D
			D3D11_TEXTURE2D_DESC mirrorTextureDesc;
			m_pMirrorTextureDX->GetDesc( &mirrorTextureDesc );

			UnityXRVector2 recommendedMirrorSize = GetRecommendedMirrorResolution();

			// Check if it's the correct size
			if ( mirrorTextureDesc.Width == recommendedMirrorSize.x )
			{
				UnityXRRenderTextureDesc pNativeTexture;

				memset( &pNativeTexture, 0, sizeof( UnityXRRenderTextureDesc ) );
				pNativeTexture.colorFormat = kUnityXRRenderTextureFormatRGBA32;
				pNativeTexture.depthFormat = kUnityXRDepthTextureFormat24bitOrGreater;
				pNativeTexture.depth.nativePtr = (void * )kUnityXRRenderTextureIdDontCare;

				pNativeTexture.width = m_nEyeMirrorWidth;
				pNativeTexture.height = m_nEyeMirrorHeight;
				pNativeTexture.color.nativePtr = m_pMirrorTextureDX;
				pNativeTexture.flags |= kUnityXRRenderTextureFlagsSRGB;

				UnityXRRenderTextureId pSourceTextureId;
				UnitySubsystemErrorCode eCreateTextureError = s_pXRDisplay->CreateTexture( s_DisplayHandle, &pNativeTexture, &pSourceTextureId );
				if ( eCreateTextureError != kUnitySubsystemErrorCodeSuccess )
				{
					XR_TRACE( "[OpenVR] [Mirror] Unable to create a native texture to display in mirror mode [%i]\n", eCreateTextureError );
					m_bIsUsingCustomMirrorMode = false;
				}

				// Use the SteamVR VR view as mirror if possible
				m_mirrorRenderSubRect = { 0.0f, 0.0f, 1.0f, 1.0f };
				m_pMirrorTexture = m_pSteamVRTextureId = pSourceTextureId;
				m_bIsIncorrectTexture = false;
				m_bIsUsingCustomMirrorMode = true;
				m_bIsSteamVRViewAvailable = true;
				XR_TRACE( "[OpenVR] [Mirror] Mirror view shared texture opened\n" );
			}
			else
			{
				XR_TRACE( "[OpenVR] [Error] [Mirror] Unexpected texture size %ix%i found\n", mirrorTextureDesc.Width, mirrorTextureDesc.Height );
				m_pMirrorTexture = m_UnityTextures[0][0];
				m_bIsIncorrectTexture = true;
			}
		}
		else
		{
			XR_TRACE( "[OpenVR] [Error] [Mirror] Unable to open shared texture\n" );

			// Release overlay view
			if ( vr::VROverlayView() && m_overlayView.texture.handle )
			{
				vr::VROverlayView()->ReleaseOverlayView( &m_overlayView );
				m_hOverlay = k_ulInvalidOverlayHandle;
			}

			m_bIsUsingCustomMirrorMode = false;
			m_bIsSteamVRViewAvailable = false;
		}
	}

	if ( GetCurrentMirrorMode() == kUnityXRMirrorBlitDistort )
	{
		if ( !m_bIsUsingRGB )
		{
			// TODO: Request Unity to have XRMirrorBlitDesc to support custom sRGB setting despite project setting
			m_bOverlayFallback = true;
			XR_TRACE( "[OpenVR] [Error] [Mirror] OpenVR View falling back to left eye texture. Project not using sRGB.\n" );
		}
		else if ( m_eActiveTextureType != vr::TextureType_DirectX )
		{
			m_bOverlayFallback = true;
			XR_TRACE( "[OpenVR] [Error] [Mirror] OpenVR View falling back to left eye texture. Project not using DirectX.\n" );
		}
		else if ( !m_bIsSteamVRViewAvailable && !m_bIsUsingCustomMirrorMode )
		{
			m_nOpenVRMirrorAttempts++;
			if ( m_nOpenVRMirrorAttempts >= k_nOpenVRMirrorAttemptsMax )
			{
				m_bOverlayFallback = true;
				XR_TRACE( "[OpenVR] [Error] [Mirror] OpenVR View falling back to left eye texture. Could not enable SteamVR View.\n" );
			}
			else
			{
				XR_TRACE( "[OpenVR] [Error] [Mirror] Could not enable SteamVR View. Will retry...\n" );
			}
		}
		else if ( m_bIsIncorrectTexture )
		{
			m_nOpenVRMirrorAttempts++;
			if ( m_nOpenVRMirrorAttempts >= k_nOpenVRMirrorAttemptsMax )
			{
				m_bOverlayFallback = true;
				XR_TRACE( "[OpenVR] [Error] [Mirror] OpenVR View falling back to left eye texture. Incorrect texture size.\n" );
			}
			else
			{
				XR_TRACE( "[OpenVR] [Error] [Mirror] Incorrect texture size. Will retry...\n" );
			}
		}

		if ( m_bOverlayFallback )
		{
			// Fallback to left eye texture
			SetMirrorMode( kUnityXRMirrorBlitLeftEye );
			m_nPrevMirrorMode = kUnityXRMirrorBlitLeftEye;
			m_pMirrorTexture = m_UnityTextures[0][0];
			m_bIsSteamVRViewAvailable = false;

			SetupMirror(); //setup with the left eye
		}
	}
	#endif
}


bool RegisterDisplayLifecycleProvider( OpenVRProviderContext *pOpenProviderContext )
{
	XR_TRACE( "[OpenVR] Display lifecyle provider registered\n" );

	s_pXRDisplay = UnityInterfaces::Get().GetInterface< IUnityXRDisplayInterface >();
	s_pProviderContext = pOpenProviderContext;
	s_pXRStats = (IUnityXRStats * )s_pProviderContext->interfaces->GetInterface( UNITY_GET_INTERFACE_GUID( IUnityXRStats ) );

	UnityLifecycleProvider displayLifecycleHandler = { 0 };

	pOpenProviderContext->displayProvider = new OpenVRDisplayProvider;
	displayLifecycleHandler.userData = pOpenProviderContext->displayProvider;
	displayLifecycleHandler.Initialize = &Lifecycle_Initialize;
	displayLifecycleHandler.Start = &Lifecycle_Start;
	displayLifecycleHandler.Stop = &Lifecycle_Stop;
	displayLifecycleHandler.Shutdown = &Lifecycle_Shutdown;

	UnitySubsystemErrorCode result = s_pXRDisplay->RegisterLifecycleProvider( "XRSDKOpenVR", "OpenVR Display", &displayLifecycleHandler );

	if ( result != kUnitySubsystemErrorCodeSuccess )
	{
		XR_TRACE( "[OpenVR] [Error] Unable to register display lifecyle provider: [%i]\n", result );
		return false;
	}

	return true;
}