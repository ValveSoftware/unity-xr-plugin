#pragma once

#include <vector>
#include <limits>

#include "ProviderInterface/IUnityXRDisplay.h"
#include "OpenVRSystem.h"
#include "OpenVRProviderContext.h"
#include "ProviderInterface/IUnityGraphics.h"
#include "ProviderInterface/IUnityGraphicsVulkan.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WINAPI_FAMILY)
#include "d3d11.h"
#include "ProviderInterface/IUnityGraphicsD3D11.h"
#define XR_WIN 1
#else
#define XR_WIN 0
#endif
#include <CommonTypes.h>


#define kPI 3.14159265358979323846264338327950288419716939937510F
#define RAD2DEG (180.0f / kPI)
#define DEG2RAD (2.0F * kPI / 360.0F)

bool RegisterDisplayLifecycleProvider( OpenVRProviderContext * pOpenProviderContext );

// Default mirror subrect extents
static const float k_flDefaultSubRectX = 0.15f;
static const float k_flDefaultSubRectY = 0.15f;
static const float k_flDefaultSubRectWidth = 0.65f;
static const float k_flDefaultSubRectHeight = 0.65f;
static const float k_flDockedViewAspecRatio = 1.97f;

// Default near/far eye projection values
static const float k_flNear = 0.0000001f;
static const float k_flFar = 1000.0f;

// Default UnityXR constants not covered by the Unity interfaces
static const UnityXROcclusionMeshId k_nInvalidUnityXROcclusionMeshId = 0;
static const vr::VROverlayHandle_t k_ulInvalidOverlayHandle = 0;

///The max number of attempts to make to attain a preview window
static const uint32_t k_nOpenVRMirrorAttemptsMax = 5;

class OpenVRDisplayProvider
{
public:
	OpenVRDisplayProvider();
	~OpenVRDisplayProvider();

	// --- IUnityInterface implementations
	UnitySubsystemErrorCode Lifecycle_Initialize( UnitySubsystemHandle handle, void *userData );
	UnitySubsystemErrorCode Lifecycle_Start( UnitySubsystemHandle handle );

	void Lifecycle_Stop( UnitySubsystemHandle handle );
	void Lifecycle_Shutdown( UnitySubsystemHandle handle );
	// --- End of IUnityInterface implementations

	// --- IUnityXRDisplay interface implementation
	UnitySubsystemErrorCode GfxThread_Start( UnityXRRenderingCapabilities *renderingCaps );
	UnitySubsystemErrorCode GfxThread_PopulateNextFrameDesc( const UnityXRFrameSetupHints *frameHints, UnityXRNextFrameDesc *nextFrame );
	UnitySubsystemErrorCode GfxThread_SubmitCurrentFrame();
	UnitySubsystemErrorCode GfxThread_BlitToMirrorViewRenderTarget( const UnityXRMirrorViewBlitInfo *mirrorBlitInfo );
	UnitySubsystemErrorCode GfxThread_Stop();

	UnitySubsystemErrorCode MainThread_UpdateDisplayState( UnityXRDisplayState *state );
	UnitySubsystemErrorCode MainThread_QueryMirrorViewBlitDesc( const UnityXRMirrorViewBlitInfo *pMirrorBlitInfo, UnityXRMirrorViewBlitDesc *pBlitDescriptor, OpenVRDisplayProvider *pDisplay );
	// --- End of IUnityXRDisplay interface implementation

	/// Get the currently active mirror mode - kUnityXRMirrorBlitNone, kUnityXRMirrorBlitLeftEye, kUnityXRMirrorBlitRightEye, kUnityXRMirrorBlitDistort/SteamVR View (default)
	int GetCurrentMirrorMode() const { return m_nMirrorMode; }

	/// Set active mirror mode - kUnityXRMirrorBlitNone, kUnityXRMirrorBlitLeftEye, kUnityXRMirrorBlitRightEye, kUnityXRMirrorBlitDistort/SteamVR View (default)
	void SetMirrorMode( int val )
	{
		if ( old_m_nMirrorMode != val )
		{
			XR_TRACE( "[OpenVR] Set active mirror mode (%d)\n", val );
		}
		old_m_nMirrorMode = val;
		m_nMirrorMode = val;
	}

private:
	int old_m_nMirrorMode;

	/// Sets up the mirror view
	void SetupMirror();

	/// Tries to update the mirror mode if we haven't had a failure
	void TryUpdateMirrorMode( bool skipResolutionCheck = false );

	/// Submit a texture to the compositor
	/// @param[in] EVREye eEye - For which eye to process the texture
	/// @param[in] int32_t stage - The stage for this frame to pull the correct texture from m_NativeColorTextures
	/// @return bool - If the submit to the compositor succeeded
	bool SubmitToCompositor( vr::EVREye eEye, int nStage );

	/// Set the render pass properties and parameters that will be used in the target frame
	/// @param[in] EVREye eEye - Which eye to set this render param for
	/// @param[in] UnityXRFrameSetupHints* frameHints - Frame info
	/// @param[in] UnityXRNextFrameDesc* nextFrame - The target frame
	void SetupRenderPass( const vr::EVREye eEye, const UnityXRFrameSetupHints *pFrameHints, UnityXRNextFrameDesc *pTargetFrame );

	/// Set the occlusion mesh (hidden area mesh) for a given eye
	/// @param[in] EVREye eEye - Target eye for the occlusion mesh
	/// @return bool - If we got an occlusion mesh properly setup for the target eye
	UnityXROcclusionMeshId SetupOcclusionMesh( vr::EVREye eEye );

	/// Get the distance squared between two Vectors
	/// @param[in] const float* pVector1 - The origin vector
	/// @param[in] const float* pVector2 - The target vector
	/// @return float - distance squared between two vectors
	float GetDistanceSquared2D( const float *pVector1, const float *pVector2 );

	/// Get the recommended resolution (VRHeadsetView size) for the mirror
	/// @param[out] const UnityXRVector2 - The recommended width (x) and height (y) of the mirror view
	const UnityXRVector2 GetRecommendedMirrorResolution();

	/// Get the Eye to Head transform for the given eye
	/// @param[in] int eye - 0:Left, 1:Right
	/// @return UnityXRPose - Position and Rotation (quaternion)
	UnityXRPose GetEyePose( int eye );

	/// Helper function to calculate the projection matrix for a given eye
	/// @param[in] int eye - 0:Left, 1:Right
	/// @param[in] float flNear - the near projection value (clipping area) of the camera (can be changed during runtime)
	/// @param[in] float flNear - the far projection value (clipping area) of the camera (can be changed during runtime)
	/// @return UnityXRProjection 
	UnityXRProjection GetProjection( int eye, float flNear, float flFar );

	/// Setup the culling pass for this application
	/// @param[in][return] UnityXRNextFrameDesc::UnityXRCullingPass& cullingPass 
	void SetupCullingPass( int eye, const UnityXRFrameSetupHints *frameHints, UnityXRNextFrameDesc::UnityXRCullingPass &cullingPass );

	/// Create the eye textures that will be passed to the compositor
	/// @param[in] const UnityXRFrameSetupHints* frameHint - Details about the frame (fram number, frame in flight, singlepass, etc)
	/// @return UnitySubsystemErrorCode 
	UnitySubsystemErrorCode CreateEyeTextures( const UnityXRFrameSetupHints *frameHints );

	/// Destroy the textures Unity uses to submit to the compositor
	/// @param[in] UnitySubsystemHandle handle - The handle for this display provider
	void DestroyEyeTextures( UnitySubsystemHandle handle );

	/// Get the eye textures Unity uses to submit to the compositor
	/// @param[in] int stage - The stage of the render pass 
	/// @param[in] int eye - 0:Left, 1:Right
	/// @return The device native texture that needs to be sent to the compositor
	void *GetNativeEyeTexture( int stage, int eye );

	void ReleaseOverlayPointers();

	bool HasOverlayPointer();

	void SetupOverlayMirror();

	/// The occlusion mesh (hidden area mesh) handle for the left eye. 0 if none.
	UnityXROcclusionMeshId m_pOcclusionMeshLeftEye = 0;

	/// The occlusion mesh (hidden area mesh) handle for the right eye. 0 if none. 
	UnityXROcclusionMeshId m_pOcclusionMeshRightEye = 0;

	/// The active render device (e.g. an ID3D11Device if using DirectX)
	void *m_pRenderDevice;

	/// The active render device for the compositor
	vr::VRNativeDevice_t m_nativeDevice;

	/// Cache for the mirror textures. This will be populated by the "VR View" textures from SteamVR
	UnityXRRenderTextureId m_pMirrorTexture;

	/// Cache for the Unity equivalent shared SteamVR Texture ID
	UnityXRRenderTextureId m_pSteamVRTextureId;

	/// The handle to the SteamVR View overlay
	vr::VROverlayHandle_t m_hOverlay = k_ulInvalidOverlayHandle;

	/// The SteamVR VR view overlay view
	vr::VROverlayView_t m_overlayView;

	/// Defines the part of the provided texture that will be used in the frame buffer  
	vr::VRTextureBounds_t m_textureBounds = { 0.f, 0.f, 1.f, 1.f };

	/// OpenVR Texture type in use (coincides with m_eActiveRenderer)
	vr::ETextureType m_eActiveTextureType = vr::TextureType_DirectX;

	#ifndef __linux__
	/// DirectX Mirror Texture
	ID3D11Texture2D *m_pMirrorTextureDX;
	#endif

	/// The recommended widths and heights
	uint32_t m_nEyeWidth, m_nEyeHeight, m_nEyeMirrorWidth, m_nEyeMirrorHeight, m_nRenderMirrorWidth, m_nRenderMirrorHeight;

	/// The sub-rect of the mirror texture that we need to pass down to Unity's mirror blit
	UnityXRRectf m_mirrorRenderSubRect = { 0.0f, 0.0f, 1.0f, 1.0f };

	/// If we're using the SteamVR vr view textures for the mirror 
	bool m_bIsUsingCustomMirrorMode = false;

	/// If the app is currently applying any render viewport scale
	bool m_bIsRenderViewportScaling = false;

	/// If SteamVR VR View is available
	bool m_bIsSteamVRViewAvailable = false;

	/// If the HeadsetView has already been set
	bool m_bIsHeadsetResolutionSet = false;

	/// If an incorrect/incompatible texture was opened
	bool m_bIsIncorrectTexture = false;

	/// If an incorrect/incompatible texture was opened
	bool m_bOverlayFallback = false;

	/// If an incorrect/incompatible texture was opened
	bool m_bFallbackLogged = false;

	/// If sRGB is used
	bool m_bIsUsingRGB = false;

	/// Whether the current frame is in flight (ready, queued for gpu)
	bool m_bFrameInFlight = false;

	/// Flag to indicate whether all Unity side textures have been created (triggers CreateEyeTextures if not)
	bool m_bTexturesCreated = false;

	/// Whether the application is using single pass or multi pass (can be changed in runtime)
	bool m_bUseSinglePass = false;

	bool m_bIsOverlayApplication = false;

	/// The current frame number, will revert to 0 at UINT32MAX
	uint32_t m_nCurFrame = 0;

	///The current number of attempts to make to attain a preview window
	uint32_t m_nOpenVRMirrorAttempts = 0;

	/// Maximum number of stages per render pass
	static const int k_nMaxNumStages = 3;

	/// The default number of stage per render pass
	int m_nNumStages = 2;

	/// The currently active mirror mode 
	int m_nMirrorMode = kUnityXRMirrorBlitRightEye;

	/// The previously active mirror mode
	int m_nPrevMirrorMode = kUnityXRMirrorBlitRightEye;

	/// Holds the native color texture based on device (DX11/12)
	void *m_pNativeColorTextures[k_nMaxNumStages][2];

	/// Holds the native depth texture if any, based on device (DX11/12)
	void *m_pNativeDepthTextures[k_nMaxNumStages][2];

	/// Holds the Vulkan Image from Unity
	UnityVulkanImage m_unityVulkanImage = {};

	/// Holds the OpenVR Vulkan Texture Array data for submitting to the compositor
	vr::VRVulkanTextureArrayData_t m_vrVulkanTexture = {};

	/// Holds the Unity equivalent eye textures per stage (0:Left, 1: Right, Single Pass only uses left with texture array size of 2)
	UnityXRRenderTextureId m_UnityTextures[k_nMaxNumStages][2];

	/// Holds the Unity equivalent eye depth textures per stage (0:Left, 1: Right, Single Pass only uses left with texture array size of 2)
	UnityXRRenderTextureId m_UnityDepthTextures[k_nMaxNumStages][2];
};
