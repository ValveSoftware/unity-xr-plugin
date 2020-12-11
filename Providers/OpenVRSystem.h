#pragma once

#include "OpenVR/openvr.h"
#include "Singleton.h"
#include "ProviderInterface/IUnityXRPreInit.h"

extern "C" typedef void( *TickCallback )( int );

class OpenVRSystem : public Singleton<OpenVRSystem>
{
public:
	OpenVRSystem();
	~OpenVRSystem();

	bool Initialize();
	bool Shutdown();

	bool GetInitialized();

	bool GetGraphicsAdapterId( void *userData, UnityXRPreInitRenderer renderer, uint64_t rendererData, uint64_t *adapterId );
	bool GetVulkanInstanceExtensions( void *userData, uint32_t namesCapacityIn, uint32_t *namesCountOut, char *namesString );
	bool GetVulkanDeviceExtensions( void *userData, uint32_t namesCapacityIn, uint32_t *namesCountOut, char *namesString );

	int GetFrameIndex() { return m_FrameIndex; }
	void SetFrameIndex( int frameIndex ) { m_FrameIndex = frameIndex; }
	bool Update();

	vr::EVRInitError GetInitializationResult();

	vr::IVRSystem *GetSystem() { return m_VRSystem; }
	vr::IVRCompositor *GetCompositor() { return m_VRCompositor; }

	void SetTickCallback( TickCallback newTickCallback ) { tickCallback = newTickCallback; }

private:
    uint64_t graphicsAdapterId;
	int m_FrameIndex;

	// OpenVR interfaces
	vr::IVRSystem *m_VRSystem;
	vr::IVRCompositor *m_VRCompositor;

	vr::EVRInitError initError;

	vr::IVRCompositor *StartVRCompositor();

	TickCallback tickCallback;
};

