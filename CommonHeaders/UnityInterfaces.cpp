// Copyright (c) 2020, Valve Software
//
// SPDX-License-Identifier: BSD-3-Clause

#include "ProviderInterface/IUnityXRDisplay.h"
#include "UnityInterfaces.h"


UnityInterfaces::UnityInterfaces() : m_UnityInterfaces( nullptr )
{
}


UnityInterfaces::~UnityInterfaces()
{
	m_UnityInterfaces = nullptr;
}
