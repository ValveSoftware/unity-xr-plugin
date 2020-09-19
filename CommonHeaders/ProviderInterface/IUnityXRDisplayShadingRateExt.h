#pragma once
#if !UNITY
#include "IUnityInterface.h"
#endif
#include "IUnityXRDisplay.h"

/// @file IUnityXRDisplayShadingRateExt.h
/// @brief XR extension interface for texture allocation with shading rate image
/// @see IUnityXRDisplayShadingRateExt

/// @brief Exposes entry points related to shading rate
UNITY_DECLARE_INTERFACE(IUnityXRDisplayShadingRateExt)
{
    /// Provides the same functionality as IUnityXRDisplayInterface::CreateTexture, but also allows for supplying a shading rate image.
    /// This only works with the Vulkan graphics API if VK_EXT_fragment_density_map is available.
    ///
    /// @param[in] handle Handle obtained from UnityLifecycleProvider callbacks.
    /// @param[in] colorDepthDesc Descriptor of the color/depth textures to be created.
    /// @param[in] shadingRateTexture Native texture pointer for a shading rate image.
    /// @param[out] outTexId Texture ID representing a unique instance of a texture.
    /// @return kUnitySubsystemErrorCodeSuccess Successfully requested creation of texture.
    /// @return kUnitySubsystemErrorCodeInvalidArguments Invalid / null parameters
    /// @return kUnitySubsystemErrorCodeFailure Error
    UnitySubsystemErrorCode(UNITY_INTERFACE_API * CreateTextureWithShadingRate)(UnitySubsystemHandle handle, const UnityXRRenderTextureDesc* colorDepthDesc, const UnityXRTextureData* shadingRateTexture, UnityXRRenderTextureId* outTexId);
};
UNITY_REGISTER_INTERFACE_GUID(0x1066FBD7A90E4A74ULL, 0xBCCBB730B26DE473ULL, IUnityXRDisplayShadingRateExt)
