cmake_minimum_required(VERSION 3.10)
set(CMAKE_SUPPRESS_REGENERATION true)
project(unity_xr_openvr_plugin)

# C++ 17 - note cmake minver is 3.10 supported only on MS C++ 2015 Update 3 and above
message(STATUS "Project language set to C++17")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(BINARY_RUNTIME_PATH "${CMAKE_SOURCE_DIR}/com.valve.openvr/Runtime")

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
	
	message(STATUS "Found MSVC version ${MSVC_VERSION} in system")
	if(${MSVC_VERSION} VERSION_LESS 1919)
		set(PDBFILE "vc141.pdb")
	elseif(${MSVC_VERSION} VERSION_LESS 1936)
		set(PDBFILE "vc142.pdb")
	else()
		set(PDBFILE "vc143.pdb")
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
		${CMAKE_SOURCE_DIR}/Providers/dllmain.cpp
		${CMAKE_SOURCE_DIR}/Providers/OpenVRProviderContext.h
		${CMAKE_SOURCE_DIR}/Providers/OpenVRSystem.h	${CMAKE_SOURCE_DIR}/Providers/OpenVRSystem.cpp
		${CMAKE_SOURCE_DIR}/Providers/UserProjectSettings.h	${CMAKE_SOURCE_DIR}/Providers/UserProjectSettings.cpp

		${CMAKE_SOURCE_DIR}/Providers/Display/Display.h	${CMAKE_SOURCE_DIR}/Providers/Display/Display.cpp
		${CMAKE_SOURCE_DIR}/Providers/Input/Input.h	${CMAKE_SOURCE_DIR}/Providers/Input/Input.cpp

		${CMAKE_SOURCE_DIR}/CommonHeaders/UnityInterfaces.h	${CMAKE_SOURCE_DIR}/CommonHeaders/UnityInterfaces.cpp

		${CMAKE_SOURCE_DIR}/CommonHeaders/Singleton.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/CommonTypes.h

		${CMAKE_SOURCE_DIR}/CommonHeaders/OpenVR/openvr.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/OpenVR/openvr_driver.h

		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityXRAudio.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityEventQueue.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityGraphics.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityGraphicsD3D11.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityGraphicsD3D12.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityGraphicsMetal.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityGraphicsVulkan.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityInterface.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityProfilerCallbacks.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityRenderingExtensions.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityShaderCompilerAccess.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityXRDisplay.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityXRDisplayShadingRateExt.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityXRInput.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityXRMeshing.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityXRPreInit.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityXRStats.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/IUnityXRTrace.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/UnitySubsystemTypes.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/UnityXRDisplayStats.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/UnityXRTypes.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/UnityTypes.h
		${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/XRMath.h	${CMAKE_SOURCE_DIR}/CommonHeaders/ProviderInterface/XRMath.cpp
		)

target_include_directories(XRSDKOpenVR PUBLIC 
				${CMAKE_SOURCE_DIR}/Providers
				${CMAKE_SOURCE_DIR}/CommonHeaders
				${CMAKE_SOURCE_DIR}/ThirdParty
				)

include(GenerateExportHeader)
generate_export_header(XRSDKOpenVR)


message(STATUS "Runtime output directory set to: ${BINARY_PATH}")
set_target_properties(XRSDKOpenVR 
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${BINARY_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${BINARY_PATH}"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${BINARY_PATH}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BINARY_PATH}"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${BINARY_PATH}"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${BINARY_PATH}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BINARY_PATH}"
)


# OpenVR
message(STATUS "Importing OpenVR API...")

add_library(openvr_api SHARED IMPORTED) 

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
set_target_properties(openvr_api PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/${PLATFORMX}/${OPENVR_API_NAME}.${COMPILED_LIB_TYPE})
set_target_properties(openvr_api PROPERTIES IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/lib/${PLATFORMX}/${OPENVR_API_NAME}.${IMPORT_LIB_TYPE})
else()
set_target_properties(openvr_api PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/${OPENVR_API_NAME}.${COMPILED_LIB_TYPE})
set_target_properties(openvr_api PROPERTIES IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/${OPENVR_API_NAME}.${IMPORT_LIB_TYPE})
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

			#COMMAND ${CMAKE_COMMAND} -E copy
            	    #${CMAKE_SOURCE_DIR}/XRSDKOPENVR.dir/Release/${PDBFILE}
                	#${BINARY_PATH}/${PDBFILE}
			)
else()
	# Post-Build - Copy OpenVR api
	add_custom_command(TARGET XRSDKOpenVR POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
					${CMAKE_SOURCE_DIR}/${OPENVR_API_NAME}.${COMPILED_LIB_TYPE}
					${BINARY_PATH}/${OPENVR_API_NAME}.${COMPILED_LIB_TYPE}
			
			COMMAND ${CMAKE_COMMAND} -E copy
					${CMAKE_SOURCE_DIR}/${OPENVR_API_NAME}.${IMPORT_LIB_TYPE}
					${BINARY_PATH}/${OPENVR_API_NAME}.${IMPORT_LIB_TYPE}
			)
endif()
