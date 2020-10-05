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

