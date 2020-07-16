#pragma once

#include <cassert>
#include "ProviderInterface/IUnityXRDisplay.h"

struct IUnityXRTrace;
struct IUnityXRDisplayInterface;
struct IUnityXRInputInterface;

class OpenVRDisplayProvider;
class OpenVRInputProvider;

struct OpenVRProviderContext
{
	IUnityInterfaces *interfaces;
	IUnityXRTrace *trace;

	IUnityXRDisplayInterface *display;
	OpenVRDisplayProvider *displayProvider;

	IUnityXRInputInterface *input;
	OpenVRInputProvider *inputProvider;
};

inline OpenVRProviderContext &GetProviderContext( void *data )
{
	assert( data != NULL );
	return *static_cast< OpenVRProviderContext * >( data );
}

class ProviderImpl
{
public:
	ProviderImpl( OpenVRProviderContext &providerContext, UnitySubsystemHandle subsystemHandle )
		: m_Context( providerContext )
		, m_Handle( subsystemHandle )
	{
	}
	virtual ~ProviderImpl() {}

	virtual UnitySubsystemErrorCode Initialize() = 0;
	virtual UnitySubsystemErrorCode Start() = 0;

	virtual void Stop() = 0;
	virtual void Shutdown() = 0;

protected:
	OpenVRProviderContext &m_Context;
	UnitySubsystemHandle m_Handle;
};
