# Copyright (c) 2020, Valve Software
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.10)
set(CMAKE_SUPPRESS_REGENERATION true)
project(unity_xr_openvr_plugin)

# C++ 17 - note cmake minver is 3.10 supported only on MS C++ 2015 Update 3 and
# above
message(STATUS "Project language set to C++17")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PACKAGE "com.valve.openvr")

# Auto determine platform if none is provided
if(NOT PLATFORM)
	if(CMAKE_SIZEOF_VOID_P MATCHES 8)
		set(PLATFORM 64)
	else()
		set(PLATFORM 32)
	endif()
endif()

# Platform binary path
if(PLATFORM MATCHES 64)
	set(PLATFORMX "x64")
else()
	set(PLATFORMX "x86")
endif()

option(
	COPY_TO_PACKAGE
	"Should the binaries be copied to the ${PACKAGE} package as a post-build step? This will modify the source tree!"
	ON)

set(DEST "${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE}/Runtime/${PLATFORMX}")

# Set Platform
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_definitions(-DLINUX -DPOSIX)
	set(CMAKE_SHARED_LINKER_FLAGS
		"${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath -Wl,'$ORIGIN'")
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

	if(${PLATFORM} MATCHES 32)
		message(WARNING "OpenVR x86 binaries not provided on x86 GNU/Linux.")
	endif()

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	add_definitions(-D_WIN32)
	add_compile_options("/Zi")
endif()

if(MSVC)
	# Statically link the runtime libraries
	add_compile_options(
		$<$<CONFIG:>:/MT>
		$<$<CONFIG:Debug>:/MTd>
		$<$<CONFIG:Release>:/MT>
		$<$<CONFIG:RelWithDebInfo>:/MT>)
endif()

message(STATUS "Compilation set for ${CMAKE_SYSTEM_NAME} ${PLATFORM}bit")

# XRSDKOpenVR
message(STATUS "Adding XRSDKOpenVR...")

add_library(XRSDKOpenVR SHARED
		Providers/dllmain.cpp
		Providers/OpenVRProviderContext.h
		Providers/OpenVRSystem.h	Providers/OpenVRSystem.cpp
		Providers/UserProjectSettings.h	Providers/UserProjectSettings.cpp

		Providers/Display/Display.h	Providers/Display/Display.cpp
		Providers/Input/Input.h	Providers/Input/Input.cpp

		CommonHeaders/UnityInterfaces.h	CommonHeaders/UnityInterfaces.cpp

		CommonHeaders/Singleton.h
		CommonHeaders/CommonTypes.h

		CommonHeaders/OpenVR/openvr.h
		CommonHeaders/OpenVR/openvr_driver.h

		CommonHeaders/ProviderInterface/IUnityXRAudio.h
		CommonHeaders/ProviderInterface/IUnityEventQueue.h
		CommonHeaders/ProviderInterface/IUnityGraphics.h
		CommonHeaders/ProviderInterface/IUnityGraphicsD3D11.h
		CommonHeaders/ProviderInterface/IUnityGraphicsD3D12.h
		CommonHeaders/ProviderInterface/IUnityGraphicsMetal.h
		CommonHeaders/ProviderInterface/IUnityGraphicsVulkan.h
		CommonHeaders/ProviderInterface/IUnityInterface.h
		CommonHeaders/ProviderInterface/IUnityProfilerCallbacks.h
		CommonHeaders/ProviderInterface/IUnityRenderingExtensions.h
		CommonHeaders/ProviderInterface/IUnityShaderCompilerAccess.h
		CommonHeaders/ProviderInterface/IUnityXRDisplay.h
		CommonHeaders/ProviderInterface/IUnityXRDisplayShadingRateExt.h
		CommonHeaders/ProviderInterface/IUnityXRInput.h
		CommonHeaders/ProviderInterface/IUnityXRMeshing.h
		CommonHeaders/ProviderInterface/IUnityXRPreInit.h
		CommonHeaders/ProviderInterface/IUnityXRStats.h
		CommonHeaders/ProviderInterface/IUnityXRTrace.h
		CommonHeaders/ProviderInterface/UnitySubsystemTypes.h
		CommonHeaders/ProviderInterface/UnityXRDisplayStats.h
		CommonHeaders/ProviderInterface/UnityXRTypes.h
		CommonHeaders/ProviderInterface/UnityTypes.h
		CommonHeaders/ProviderInterface/XRMath.h	CommonHeaders/ProviderInterface/XRMath.cpp
		)

target_include_directories(XRSDKOpenVR PUBLIC
				Providers
				CommonHeaders
				ThirdParty
				)

# OpenVR
set(OPENVR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/${PLATFORMX})
add_library(openvr_api SHARED IMPORTED)
set_target_properties(
	openvr_api
	PROPERTIES
		IMPORTED_LOCATION
		${OPENVR_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}openvr_api${CMAKE_SHARED_LIBRARY_SUFFIX}
)
if(WIN32)
	set_target_properties(
		openvr_api
		PROPERTIES
			IMPORTED_IMPLIB
			${OPENVR_DIR}/${CMAKE_IMPORT_LIBRARY_PREFIX}openvr_api${CMAKE_IMPORT_LIBRARY_SUFFIX}
	)
endif()
set_target_properties(openvr_api PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(XRSDKOpenVR PUBLIC openvr_api)

if(COPY_TO_PACKAGE)
	message(
		STATUS
			"Will copy result binary (and OpenVR dependency) to ${DEST} as a post-build step: Turn off COPY_TO_PACKAGE to disable this behavior."
	)

	if(WIN32)
		# Post-Build - Copy OpenVR api
		add_custom_command(
			TARGET XRSDKOpenVR
			POST_BUILD
			COMMAND
				${CMAKE_COMMAND} -E copy
				${OPENVR_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}openvr_api${CMAKE_SHARED_LIBRARY_SUFFIX}
				${DEST}
			COMMAND
				${CMAKE_COMMAND} -E copy
				${OPENVR_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}openvr_api${CMAKE_SHARED_LIBRARY_SUFFIX}.sig
				${DEST}
			COMMAND ${CMAKE_COMMAND} -E rm -f ${DEST}/$<TARGET_FILE_NAME:XRSDKOpenVR>
			COMMAND ${CMAKE_COMMAND} -E rm -f ${DEST}/$<TARGET_PDB_FILE_NAME:XRSDKOpenVR>
			COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:XRSDKOpenVR>" ${DEST}
			COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_PDB_FILE:XRSDKOpenVR>" ${DEST}
			COMMENT "Copying plugin binary to package runtime directory"
			VERBATIM)
	else()
		# Post-Build - Copy OpenVR api
		add_custom_command(
			TARGET XRSDKOpenVR
			POST_BUILD
			COMMAND
				${CMAKE_COMMAND} -E copy
				${OPENVR_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}openvr_api${CMAKE_SHARED_LIBRARY_SUFFIX}
				${DEST}
			COMMAND ${CMAKE_COMMAND} -E rm -f "${DEST}/$<TARGET_FILE_NAME:XRSDKOpenVR>"
			COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:XRSDKOpenVR>" ${DEST}
			COMMENT "Copying plugin binary to package runtime directory"
			VERBATIM)
	endif()
endif()
