cmake_minimum_required(VERSION 3.10)
set(CMAKE_SUPPRESS_REGENERATION true)
project(unity_xr_openvr_plugin)

# C++ 17 - note cmake minver is 3.10 supported only on MS C++ 2015 Update 3 and above
message(STATUS "Project language set to C++17")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(BINARY_RUNTIME_PATH "${CMAKE_SOURCE_DIR}/com.valve.openvr/Runtime")

set(PACKAGE "com.valve.openvr")
# Determine platform architecture
message(STATUS "Checking platform architecture...")

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

set(DEST "${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE}/Runtime/${PLATFORMX}")
set(BINARY_PATH "${CMAKE_SOURCE_DIR}/com.valve.openvr/Runtime/${PLATFORMX}")
message(STATUS "Platform binary path set to: ${BINARY_PATH}")

# Set Platform
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DLINUX -DPOSIX)
  set(OPENVR_API_NAME "libopenvr_api")
  set(COMPILED_LIB_TYPE "so")
  set(IMPORT_LIB_TYPE "so.dbg")
  set(ARCH_TARGET linux64)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath -Wl,'$ORIGIN'")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

  if(${PLATFORM} MATCHES 32)
    message(WARNING "OpenVR x86 binaries not provided on x86 GNU/Linux.")
  endif()

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if(${MSVC_VERSION} VERSION_LESS 1919)
		set(PDBFILE "vc141.pdb")
	else()
		set(PDBFILE "vc142.pdb")
	endif()

	add_definitions(-D_WIN32)
	add_compile_options("/Zi")
	set(OPENVR_API_NAME "openvr_api")
	set(COMPILED_LIB_TYPE "dll")
	set(IMPORT_LIB_TYPE "lib")
	set(ARCH_TARGET win${PLATFORM})
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS True)
endif()

if(MSVC)
    message(WARNING "Adding static link option.")
    add_compile_options(
        $<$<CONFIG:>:/MT> #---------|
        $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
        $<$<CONFIG:Release>:/MT> #--|
    )
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

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Post-Build - Copy OpenVR api
	add_custom_command(TARGET XRSDKOpenVR POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
					${CMAKE_SOURCE_DIR}/lib/${PLATFORMX}/${OPENVR_API_NAME}.${COMPILED_LIB_TYPE}
					${BINARY_PATH}/${OPENVR_API_NAME}.${COMPILED_LIB_TYPE}

			COMMAND ${CMAKE_COMMAND} -E copy
					${CMAKE_SOURCE_DIR}/lib/${PLATFORMX}/${OPENVR_API_NAME}.${COMPILED_LIB_TYPE}.sig
					${BINARY_PATH}/${OPENVR_API_NAME}.${COMPILED_LIB_TYPE}.sig
			
			COMMAND ${CMAKE_COMMAND} -E copy
					${CMAKE_SOURCE_DIR}/lib/${PLATFORMX}/${OPENVR_API_NAME}.${IMPORT_LIB_TYPE}
					${BINARY_PATH}/${OPENVR_API_NAME}.${IMPORT_LIB_TYPE}

			COMMAND ${CMAKE_COMMAND} -E copy
            	    ${CMAKE_SOURCE_DIR}/XRSDKOPENVR.dir/Release/${PDBFILE}
                	${BINARY_PATH}/${PDBFILE}
			)
else()
	# Post-Build - Copy OpenVR api
	add_custom_command(TARGET XRSDKOpenVR POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
					${CMAKE_SOURCE_DIR}/lib/${PLATFORMX}/${OPENVR_API_NAME}.${COMPILED_LIB_TYPE}
					${BINARY_PATH}/lib/${PLATFORMX}/${OPENVR_API_NAME}.${COMPILED_LIB_TYPE}
			
			COMMAND ${CMAKE_COMMAND} -E copy
					${CMAKE_SOURCE_DIR}/lib/${PLATFORMX}/${OPENVR_API_NAME}.${IMPORT_LIB_TYPE}
					${BINARY_PATH}/lib/${PLATFORMX}/${OPENVR_API_NAME}.${IMPORT_LIB_TYPE}
			)
endif()
