// Copyright (c) 2020, Valve Software
//
// SPDX-License-Identifier: BSD-3-Clause

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

