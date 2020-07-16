#pragma once

#include "ProviderInterface/IUnityXRTrace.h"
#include "ProviderInterface/IUnityXRStats.h"

extern IUnityXRTrace *s_pXRTrace;
//extern IUnityXRStats* sXRStats;

#define XR_TRACE_PTR s_pXRTrace
