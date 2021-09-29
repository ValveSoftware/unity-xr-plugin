#pragma once

#include "Input.h"
#include "UnityInterfaces.h"
#include "ProviderInterface/XRMath.h"

#include "UserProjectSettings.h"

#include <cassert>
#include <array>
#include <algorithm>
#include <sstream>
#include <chrono>

static OpenVRProviderContext *s_pProviderContext;
static IUnityXRInputInterface *s_Input = nullptr;
int OpenVRInputProvider::hmdFeatureIndices[static_cast< int >( HMDFeature::Total )] = {};
int OpenVRInputProvider::controllerFeatureIndices[static_cast< int >( ControllerFeature::Total )] = {};
int OpenVRInputProvider::trackerFeatureIndices[static_cast< int >( TrackerFeature::Total )] = {};

// Valve's IVRSystem::TriggerHapticPulse method will not accept a value
// greater than this, as determined by trial and error.
const static unsigned short kMaxHapticPulseDurationInMicroseconds = 3999;
const static unsigned int kHapticsNumChannels = 1;

static UnitySubsystemErrorCode UNITY_INTERFACE_API Tick( UnitySubsystemHandle handle, void *userData, UnityXRInputUpdateType updateType )
{
	OpenVRInputProvider *input = (OpenVRInputProvider * )userData;

	return input->Tick( handle, updateType );
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API FillDeviceDefinition( UnitySubsystemHandle handle, void *userData, UnityXRInternalInputDeviceId deviceId, UnityXRInputDeviceDefinition *deviceDefinition )
{
	OpenVRInputProvider *input = (OpenVRInputProvider * )userData;

	return input->FillDeviceDefinition( handle, deviceId, deviceDefinition );
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API UpdateDeviceState( UnitySubsystemHandle handle, void *userData, UnityXRInternalInputDeviceId deviceId, UnityXRInputUpdateType updateType, UnityXRInputDeviceState *deviceState )
{
	if ( !userData )
		return kUnitySubsystemErrorCodeInvalidArguments;

	OpenVRInputProvider *input = (OpenVRInputProvider * )userData;

	if ( !input )
		return kUnitySubsystemErrorCodeInvalidArguments;

	return input->UpdateDeviceState( handle, deviceId, updateType, deviceState );
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API HandleEvent( UnitySubsystemHandle handle, void *userData, unsigned int eventType, UnityXRInternalInputDeviceId deviceId, void *buffer, unsigned int size )
{
	if ( !userData )
		return kUnitySubsystemErrorCodeInvalidArguments;

	OpenVRInputProvider *input = (OpenVRInputProvider * )userData;

	if ( !input )
		return kUnitySubsystemErrorCodeInvalidArguments;

	return input->HandleEvent( handle, eventType, deviceId, buffer, size );
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API HandleRecenter( UnitySubsystemHandle handle, void *userData )
{
	if ( !userData )
		return kUnitySubsystemErrorCodeInvalidArguments;

	OpenVRInputProvider *input = (OpenVRInputProvider * )userData;

	if ( !input )
		return kUnitySubsystemErrorCodeInvalidArguments;

	return input->HandleRecenter( handle );
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API HandleHapticImpulse( UnitySubsystemHandle handle, void *userData, UnityXRInternalInputDeviceId deviceId, int channel, float amplitude, float duration )
{
	if ( !userData )
		return kUnitySubsystemErrorCodeInvalidArguments;

	OpenVRInputProvider *input = (OpenVRInputProvider * )userData;

	if ( !input )
		return kUnitySubsystemErrorCodeInvalidArguments;

	return input->HandleHapticImpulse( handle, deviceId, channel, amplitude, duration );
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API HandleHapticBuffer( UnitySubsystemHandle handle, void *userData, UnityXRInternalInputDeviceId deviceId, int channel, unsigned int bufferSize, const unsigned char *const buffer )
{
	// Buffers are unsupported
	return kUnitySubsystemErrorCodeFailure;
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API QueryHapticCapabilities( UnitySubsystemHandle handle, void *userData, UnityXRInternalInputDeviceId deviceId, UnityXRHapticCapabilities *capabilities )
{
	if ( !userData )
		return kUnitySubsystemErrorCodeInvalidArguments;

	OpenVRInputProvider *input = (OpenVRInputProvider * )userData;

	if ( !input )
		return kUnitySubsystemErrorCodeInvalidArguments;

	return input->QueryHapticCapabilities( handle, deviceId, capabilities );
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API HandleHapticStop( UnitySubsystemHandle handle, void *userData, UnityXRInternalInputDeviceId deviceId )
{
	if ( !userData )
		return kUnitySubsystemErrorCodeInvalidArguments;

	OpenVRInputProvider *input = (OpenVRInputProvider * )userData;

	if ( !input )
		return kUnitySubsystemErrorCodeInvalidArguments;

	return input->HandleHapticStop( handle, deviceId );
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API QueryTrackingOriginMode( UnitySubsystemHandle handle, void *userData, UnityXRInputTrackingOriginModeFlags *trackingOriginMode )
{
	if ( !userData )
		return kUnitySubsystemErrorCodeInvalidArguments;

	OpenVRInputProvider *input = (OpenVRInputProvider * )userData;

	if ( !input )
		return kUnitySubsystemErrorCodeInvalidArguments;

	return input->QueryTrackingOriginMode( handle, trackingOriginMode );
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API QuerySupportedTrackingOriginModes( UnitySubsystemHandle handle, void *userData, UnityXRInputTrackingOriginModeFlags *supportedTrackingOriginModes )
{
	if ( !userData )
		return kUnitySubsystemErrorCodeInvalidArguments;

	OpenVRInputProvider *input = (OpenVRInputProvider * )userData;

	if ( !input )
		return kUnitySubsystemErrorCodeInvalidArguments;

	return input->QuerySupportedTrackingOriginModes( handle, supportedTrackingOriginModes );
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API HandleSetTrackingOriginMode( UnitySubsystemHandle handle, void *userData, UnityXRInputTrackingOriginModeFlags trackingOriginMode )
{
	if ( !userData )
		return kUnitySubsystemErrorCodeInvalidArguments;

	OpenVRInputProvider *input = (OpenVRInputProvider * )userData;

	if ( !input )
		return kUnitySubsystemErrorCodeInvalidArguments;

	return input->HandleSetTrackingOriginMode( handle, trackingOriginMode );
}

UnitySubsystemErrorCode UNITY_INTERFACE_API TryGetDeviceStateAtTime( UnitySubsystemHandle handle, void *userData, UnityXRTimeStamp time, UnityXRInternalInputDeviceId deviceId, UnityXRInputDeviceState *state )
{
	if ( !userData )
		return kUnitySubsystemErrorCodeInvalidArguments;

	OpenVRInputProvider *input = (OpenVRInputProvider * )userData;

	if ( !input )
		return kUnitySubsystemErrorCodeInvalidArguments;

	return input->TryGetDeviceStateAtTime( handle, time, deviceId, state );
}

UnitySubsystemErrorCode UNITY_INTERFACE_API OpenVRInputProvider::Tick( UnitySubsystemHandle handle, UnityXRInputUpdateType updateType )
{
	OpenVRSystem::Get().Update();

	if ( updateType == kUnityXRInputUpdateTypeBeforeRender )
		return kUnitySubsystemErrorCodeSuccess;

	// Connect/Disconnect devices marked for change
	for ( auto deviceIter = m_TrackedDevices.begin(); deviceIter != m_TrackedDevices.end(); )
	{
		if ( deviceIter->deviceStatus == EDeviceStatus::None &&
			deviceIter->deviceChangeForNextUpdate == EDeviceStatus::Connect )
		{
			s_Input->InputSubsystem_DeviceConnected( handle, deviceIter->deviceId );
			deviceIter->deviceChangeForNextUpdate = EDeviceStatus::None;
			deviceIter->deviceStatus = EDeviceStatus::Connect;
			XR_TRACE( "[OpenVR] Device connected (status change). Handle: %d. OpenVRIndex: %d. UnityID: %d\n", handle, deviceIter->openVRDeviceIndex, deviceIter->deviceId );
		}

		if ( deviceIter->deviceChangeForNextUpdate == EDeviceStatus::Disconnect )
		{
			if ( deviceIter->deviceStatus == EDeviceStatus::Connect )
			{
				s_Input->InputSubsystem_DeviceDisconnected( handle, deviceIter->deviceId );
				XR_TRACE( "[OpenVR] Device disconnected (status change). Handle: %d. OpenVRIndex: %d. UnityID: %d\n", handle, deviceIter->openVRDeviceIndex, deviceIter->deviceId );
			}
			deviceIter = m_TrackedDevices.erase( deviceIter );
		}
		else
			++deviceIter;
	}

	return kUnitySubsystemErrorCodeSuccess;
}

std::optional<std::string> OpenVRInputProvider::OpenVRDevice::GetDeviceName() const
{
	if ( ( characteristics & kUnityXRInputDeviceCharacteristicsHeadMounted ) == kUnityXRInputDeviceCharacteristicsHeadMounted )
	{
		char deviceModelName[kUnityXRStringSize] = { 0 };
		if ( OpenVRSystem::Get().GetSystem()->GetStringTrackedDeviceProperty(
			vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String, deviceModelName, sizeof( deviceModelName ) ) == 0 )
			return std::nullopt;

		std::ostringstream deviceNameStream;
		deviceNameStream << "OpenVR Headset(" << deviceModelName << ")";

		return { deviceNameStream.str() };
	}
	else if ( ( characteristics & kUnityXRInputDeviceCharacteristicsHeldInHand ) == kUnityXRInputDeviceCharacteristicsHeldInHand ||
		( characteristics & kUnityXRInputDeviceCharacteristicsController ) == kUnityXRInputDeviceCharacteristicsController )
	{
		char modelNumber[kUnityXRStringSize];
		if ( OpenVRSystem::Get().GetSystem()->GetStringTrackedDeviceProperty( openVRDeviceIndex,
			vr::Prop_ModelNumber_String, modelNumber, sizeof( modelNumber ) ) == 0 )
			return std::nullopt;

		std::ostringstream deviceNameStream;

		if ( ( characteristics & kUnityXRInputDeviceCharacteristicsLeft ) == kUnityXRInputDeviceCharacteristicsLeft )
		{
			deviceNameStream << "OpenVR Controller(" << modelNumber << ") - Left";
		}
		else if ( ( characteristics & kUnityXRInputDeviceCharacteristicsRight ) == kUnityXRInputDeviceCharacteristicsRight )
		{
			deviceNameStream << "OpenVR Controller(" << modelNumber << ") - Right";
		}
		else
		{
			deviceNameStream << "OpenVR Controller(" << modelNumber << ")";
		}

		return { deviceNameStream.str() };
	}
	else if ( ( characteristics & kUnityXRInputDeviceCharacteristicsTrackingReference ) == kUnityXRInputDeviceCharacteristicsTrackingReference )
	{
		char modelNumber[kUnityXRStringSize];
		if ( OpenVRSystem::Get().GetSystem()->GetStringTrackedDeviceProperty( openVRDeviceIndex,
			vr::Prop_ModelNumber_String, modelNumber, sizeof( modelNumber ) ) == 0 )
			return std::nullopt;

		std::ostringstream deviceNameStream;
		deviceNameStream << "OpenVR Tracking Reference(" << modelNumber << ")";
		return { deviceNameStream.str() };
	}
	else if ( ( characteristics & kUnityXRInputDeviceCharacteristicsTrackedDevice ) == kUnityXRInputDeviceCharacteristicsTrackedDevice )
	{
		char modelNumber[kUnityXRStringSize];
		if ( OpenVRSystem::Get().GetSystem()->GetStringTrackedDeviceProperty( openVRDeviceIndex,
			vr::Prop_ModelNumber_String, modelNumber, sizeof( modelNumber ) ) == 0 )
			return std::nullopt;

		std::ostringstream deviceNameStream;
		deviceNameStream << "OpenVR Tracked Device(" << modelNumber << ")";
		return { deviceNameStream.str() };
	}
	else
	{
		std::ostringstream retVal;

		char modelNumber[kUnityXRStringSize];
		char serialNumber[kUnityXRStringSize];

		if ( OpenVRSystem::Get().GetSystem()->GetStringTrackedDeviceProperty( openVRDeviceIndex,
			vr::Prop_ModelNumber_String, modelNumber, sizeof( modelNumber ) ) == 0 )
			return std::nullopt;
		if ( OpenVRSystem::Get().GetSystem()->GetStringTrackedDeviceProperty( openVRDeviceIndex,
			vr::Prop_SerialNumber_String, serialNumber, sizeof( serialNumber ) ) == 0 )
			return std::nullopt;

		retVal << modelNumber;
		retVal << " S/N ";
		retVal << serialNumber;

		return retVal.str();
	}
}

UnitySubsystemErrorCode UNITY_INTERFACE_API OpenVRInputProvider::FillDeviceDefinition( UnitySubsystemHandle handle, UnityXRInternalInputDeviceId deviceId, UnityXRInputDeviceDefinition *deviceDefinition )
{
	auto device = GetTrackedDeviceByDeviceId( deviceId );
	if ( !device )
		return kUnitySubsystemErrorCodeFailure;

	char serialNumber[kUnityXRStringSize];
	uint32_t serialNumberSize = OpenVRSystem::Get().GetSystem()->GetStringTrackedDeviceProperty( ( *device )->openVRDeviceIndex,
		vr::Prop_SerialNumber_String, serialNumber, sizeof( serialNumber ) );
	if ( serialNumberSize == 0 )
		return kUnitySubsystemErrorCodeFailure;
	s_Input->DeviceDefinition_SetSerialNumber( deviceDefinition, serialNumber );
	std::string serialNumberString = std::string( serialNumber );

	// logitech reports two devices. this is the one we want to use. bit of hacky work here.
	bool isLogitechVirtual = serialNumberString == "LOGITECH_STYLUS_VIRTUAL";

	if ( isLogitechVirtual )
	{
		s_Input->DeviceDefinition_SetManufacturer( deviceDefinition, "Logitech" );
	}
	else
	{
		char manufacturerName[kUnityXRStringSize];
		uint32_t manufacturerNameSize = OpenVRSystem::Get().GetSystem()->GetStringTrackedDeviceProperty( ( *device )->openVRDeviceIndex,
			vr::Prop_ManufacturerName_String, manufacturerName, sizeof( manufacturerName ) );
		if ( manufacturerNameSize == 0 && !isLogitechVirtual )
			return kUnitySubsystemErrorCodeFailure;
		s_Input->DeviceDefinition_SetManufacturer( deviceDefinition, manufacturerName );
		std::string manufacturerNameString = std::string( manufacturerName );

		if ( manufacturerNameString == "Logitech" && !isLogitechVirtual )
			return kUnitySubsystemErrorCodeFailure;
	}


	auto deviceName = ( *device )->GetDeviceName();
	if ( !deviceName )
		deviceName = serialNumberString;

	char inputProfileBuffer[kUnityXRStringSize];
	uint32_t inputProfileSize = OpenVRSystem::Get().GetSystem()->GetStringTrackedDeviceProperty( ( *device )->openVRDeviceIndex,
		vr::Prop_ControllerType_String, inputProfileBuffer, sizeof( inputProfileBuffer ) );
	if ( inputProfileSize == 0 )
		return kUnitySubsystemErrorCodeFailure;
	std::string inputProfileName = std::string( inputProfileBuffer );

	XR_TRACE( "[OpenVR] Found device OpenVRIndex:(%d) UnityIndex:(%d) with input profile:(%s) and name: (%s)\n", ( *device )->openVRDeviceIndex, deviceId, inputProfileName.c_str(), deviceName.value().c_str() );

	s_Input->DeviceDefinition_SetName( deviceDefinition, deviceName->c_str() );
	s_Input->DeviceDefinition_SetCharacteristics( deviceDefinition, ( *device )->characteristics );
	s_Input->DeviceDefinition_SetCanQueryForDeviceStateAtTime( deviceDefinition, true );

	if ( ( ( *device )->characteristics & kUnityXRInputDeviceCharacteristicsHeadMounted ) == kUnityXRInputDeviceCharacteristicsHeadMounted )
	{
		hmdFeatureIndices[static_cast< int >( HMDFeature::TrackingState )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "TrackingState", kUnityXRInputFeatureTypeDiscreteStates, kUnityXRInputFeatureUsageTrackingState );
		hmdFeatureIndices[static_cast< int >( HMDFeature::IsTracked )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "IsTracked", kUnityXRInputFeatureTypeBinary, kUnityXRInputFeatureUsageIsTracked );
		hmdFeatureIndices[static_cast< int >( HMDFeature::UserPresence )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "UserPresence", kUnityXRInputFeatureTypeBinary, kUnityXRInputFeatureUsageUserPresence );
		hmdFeatureIndices[static_cast< int >( HMDFeature::DevicePosition )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - Position", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageDevicePosition );
		hmdFeatureIndices[static_cast< int >( HMDFeature::DeviceRotation )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - Rotation", kUnityXRInputFeatureTypeRotation, kUnityXRInputFeatureUsageDeviceRotation );
		hmdFeatureIndices[static_cast< int >( HMDFeature::DeviceVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - Velocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageDeviceVelocity );
		hmdFeatureIndices[static_cast< int >( HMDFeature::DeviceAngularVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - AngularVelocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageDeviceAngularVelocity );
		hmdFeatureIndices[static_cast< int >( HMDFeature::LeftEyePosition )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "LeftEye - Position", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageLeftEyePosition );
		hmdFeatureIndices[static_cast< int >( HMDFeature::LeftEyeRotation )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "LeftEye - Rotation", kUnityXRInputFeatureTypeRotation, kUnityXRInputFeatureUsageLeftEyeRotation );
		hmdFeatureIndices[static_cast< int >( HMDFeature::LeftEyeVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "LeftEye - Velocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageLeftEyeVelocity );
		hmdFeatureIndices[static_cast< int >( HMDFeature::LeftEyeAngularVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "LeftEye - AngularVelocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageLeftEyeAngularVelocity );
		hmdFeatureIndices[static_cast< int >( HMDFeature::RightEyePosition )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "RightEye - Position", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageRightEyePosition );
		hmdFeatureIndices[static_cast< int >( HMDFeature::RightEyeRotation )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "RightEye - Rotation", kUnityXRInputFeatureTypeRotation, kUnityXRInputFeatureUsageRightEyeRotation );
		hmdFeatureIndices[static_cast< int >( HMDFeature::RightEyeVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "RightEye - Velocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageRightEyeVelocity );
		hmdFeatureIndices[static_cast< int >( HMDFeature::RightEyeAngularVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "RightEye - AngularVelocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageRightEyeAngularVelocity );
		hmdFeatureIndices[static_cast< int >( HMDFeature::CenterEyePosition )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "CenterEye - Position", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageCenterEyePosition );
		hmdFeatureIndices[static_cast< int >( HMDFeature::CenterEyeRotation )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "CenterEye - Rotation", kUnityXRInputFeatureTypeRotation, kUnityXRInputFeatureUsageCenterEyeRotation );
		hmdFeatureIndices[static_cast< int >( HMDFeature::CenterEyeVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "CenterEye - Velocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageCenterEyeVelocity );
		hmdFeatureIndices[static_cast< int >( HMDFeature::CenterEyeAngularVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "CenterEye - AngularVelocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageCenterEyeAngularVelocity );
	}
	else if ( ( ( *device )->characteristics & kUnityXRInputDeviceCharacteristicsHeldInHand ) == kUnityXRInputDeviceCharacteristicsHeldInHand )
	{
		controllerFeatureIndices[static_cast< int >( ControllerFeature::DevicePosition )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - Position", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageDevicePosition );
		controllerFeatureIndices[static_cast< int >( ControllerFeature::DeviceRotation )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - Rotation", kUnityXRInputFeatureTypeRotation, kUnityXRInputFeatureUsageDeviceRotation );
		controllerFeatureIndices[static_cast< int >( ControllerFeature::DeviceVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - Velocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageDeviceVelocity );
		controllerFeatureIndices[static_cast< int >( ControllerFeature::DeviceAngularVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - AngularVelocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageDeviceAngularVelocity );
		controllerFeatureIndices[static_cast< int >( ControllerFeature::TrackingState )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "TrackingState", kUnityXRInputFeatureTypeDiscreteStates, kUnityXRInputFeatureUsageTrackingState );
		controllerFeatureIndices[static_cast< int >( ControllerFeature::IsTracked )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "IsTracked", kUnityXRInputFeatureTypeBinary, kUnityXRInputFeatureUsageIsTracked );
	}
	else if ( ( ( *device )->characteristics & kUnityXRInputDeviceCharacteristicsTrackingReference ) == kUnityXRInputDeviceCharacteristicsTrackingReference )
	{
		trackerFeatureIndices[static_cast< int >( TrackerFeature::DevicePosition )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - Position", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageDevicePosition );
		trackerFeatureIndices[static_cast< int >( TrackerFeature::DeviceRotation )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - Rotation", kUnityXRInputFeatureTypeRotation, kUnityXRInputFeatureUsageDeviceRotation );
		trackerFeatureIndices[static_cast< int >( TrackerFeature::DeviceVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - Velocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageDeviceVelocity );
		trackerFeatureIndices[static_cast< int >( TrackerFeature::DeviceAngularVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - AngularVelocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageDeviceAngularVelocity );
		trackerFeatureIndices[static_cast< int >( TrackerFeature::TrackingState )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "TrackingState", kUnityXRInputFeatureTypeDiscreteStates, kUnityXRInputFeatureUsageTrackingState );
		trackerFeatureIndices[static_cast< int >( TrackerFeature::IsTracked )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "IsTracked", kUnityXRInputFeatureTypeBinary, kUnityXRInputFeatureUsageIsTracked );
	}
	else if ( ( ( *device )->characteristics & kUnityXRInputDeviceCharacteristicsTrackedDevice ) == kUnityXRInputDeviceCharacteristicsTrackedDevice )
	{
		trackerFeatureIndices[static_cast< int >( TrackerFeature::DevicePosition )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - Position", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageDevicePosition );
		trackerFeatureIndices[static_cast< int >( TrackerFeature::DeviceRotation )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - Rotation", kUnityXRInputFeatureTypeRotation, kUnityXRInputFeatureUsageDeviceRotation );
		trackerFeatureIndices[static_cast< int >( TrackerFeature::DeviceVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - Velocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageDeviceVelocity );
		trackerFeatureIndices[static_cast< int >( TrackerFeature::DeviceAngularVelocity )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "Device - AngularVelocity", kUnityXRInputFeatureTypeAxis3D, kUnityXRInputFeatureUsageDeviceAngularVelocity );
		trackerFeatureIndices[static_cast< int >( TrackerFeature::TrackingState )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "TrackingState", kUnityXRInputFeatureTypeDiscreteStates, kUnityXRInputFeatureUsageTrackingState );
		trackerFeatureIndices[static_cast< int >( TrackerFeature::IsTracked )] =
			s_Input->DeviceDefinition_AddFeatureWithUsage( deviceDefinition, "IsTracked", kUnityXRInputFeatureTypeBinary, kUnityXRInputFeatureUsageIsTracked );
	}

	return kUnitySubsystemErrorCodeSuccess;
}

static void OpenVRMatrix3x4ToUnity( const vr::HmdMatrix34_t &ovrm, UnityXRMatrix4x4 &out )
{
	float tmpMatrix[] =
	{
		ovrm.m[0][0], ovrm.m[1][0], ovrm.m[2][0], 0.0f,
		ovrm.m[0][1], ovrm.m[1][1], ovrm.m[2][1], 0.0f,
		ovrm.m[0][2], ovrm.m[1][2], ovrm.m[2][2], 0.0f,
		ovrm.m[0][3], ovrm.m[1][3], ovrm.m[2][3], 1.0f
	};

	out.columns[0].x = tmpMatrix[( 0 * 4 ) + 0];
	out.columns[1].x = tmpMatrix[( 1 * 4 ) + 0];
	out.columns[2].x = -tmpMatrix[( 2 * 4 ) + 0];
	out.columns[3].x = tmpMatrix[( 3 * 4 ) + 0];

	out.columns[0].y = tmpMatrix[( 0 * 4 ) + 1];
	out.columns[1].y = tmpMatrix[( 1 * 4 ) + 1];
	out.columns[2].y = -tmpMatrix[( 2 * 4 ) + 1];
	out.columns[3].y = tmpMatrix[( 3 * 4 ) + 1];

	out.columns[0].z = -tmpMatrix[( 0 * 4 ) + 2];
	out.columns[1].z = -tmpMatrix[( 1 * 4 ) + 2];
	out.columns[2].z = tmpMatrix[( 2 * 4 ) + 2];
	out.columns[3].z = -tmpMatrix[( 3 * 4 ) + 2];

	out.columns[0].w = tmpMatrix[( 0 * 4 ) + 3];
	out.columns[1].w = tmpMatrix[( 1 * 4 ) + 3];
	out.columns[2].w = -tmpMatrix[( 2 * 4 ) + 3];
	out.columns[3].w = tmpMatrix[( 3 * 4 ) + 3];
}

void OpenVRInputProvider::OpenVRToUnityTracking( const vr::TrackedDevicePose_t &openVRPose, std::optional<UnityXRMatrix4x4> postTransform,
	UnityXRVector3 &outPosition, UnityXRVector4 &outRotation, UnityXRVector3 &outVelocity, UnityXRVector3 &outAngularVelocity )
{
	UnityXRMatrix4x4 trackingToReference;
	OpenVRMatrix3x4ToUnity( openVRPose.mDeviceToAbsoluteTracking, trackingToReference );

	if ( postTransform )
	{
		XRMatrix4x4 &xrTrackingTransformRef = reinterpret_cast< XRMatrix4x4 & >( trackingToReference );
		xrTrackingTransformRef = reinterpret_cast< XRMatrix4x4 & >( *postTransform ) * xrTrackingTransformRef;
	}

	outPosition.x = trackingToReference.columns[3].x;
	outPosition.y = trackingToReference.columns[3].y;
	outPosition.z = trackingToReference.columns[3].z;

	XRMatrix3x3 rotationMatrix = {
		trackingToReference.columns[0].x, trackingToReference.columns[1].x, trackingToReference.columns[2].x,
		trackingToReference.columns[0].y, trackingToReference.columns[1].y, trackingToReference.columns[2].y,
		trackingToReference.columns[0].z, trackingToReference.columns[1].z, trackingToReference.columns[2].z
	};
	XRQuaternion xrQuaternion;
	MatrixToQuaternion( rotationMatrix, xrQuaternion );
	outRotation.x = xrQuaternion.x;
	outRotation.y = xrQuaternion.y;
	outRotation.z = xrQuaternion.z;
	outRotation.w = -xrQuaternion.w;

	outVelocity.x = openVRPose.vVelocity.v[0];
	outVelocity.y = openVRPose.vVelocity.v[1];
	outVelocity.z = -openVRPose.vVelocity.v[2];

	outAngularVelocity.x = openVRPose.vAngularVelocity.v[0];
	outAngularVelocity.y = openVRPose.vAngularVelocity.v[1];
	outAngularVelocity.z = -openVRPose.vAngularVelocity.v[2];
}

UnityXRMatrix4x4 OpenVRInputProvider::GetEyeTransform( EHMDEye eye )
{
	assert( eye != EHMDEye::Center );
	if ( eye == EHMDEye::Center )
		return XRMatrix4x4::identity;

	const vr::EVREye openVREye = ( eye == EHMDEye::Left ) ? vr::Eye_Left : vr::Eye_Right;
	vr::HmdMatrix34_t eyeOpenVRTransform = OpenVRSystem::Get().GetSystem()->GetEyeToHeadTransform( openVREye );
	UnityXRMatrix4x4 eyeTransform;
	OpenVRMatrix3x4ToUnity( eyeOpenVRTransform, eyeTransform );
	return eyeTransform;
}

static UnityXRTimeStamp GetCurrentUnityTimestamp()
{
	auto now = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast< std::chrono::milliseconds >( now.time_since_epoch() ).count();
	return static_cast< UnityXRTimeStamp >( ms );
}

UnitySubsystemErrorCode OpenVRInputProvider::Internal_UpdateDeviceState(
	UnitySubsystemHandle handle, const OpenVRDevice &device,
	const vr::TrackedDevicePose_t &trackingPose, UnityXRInputDeviceState *deviceState, bool updateNonTrackingData )
{
	if ( OpenVRSystem::Get().GetSystem()->IsInputAvailable() == false )
		updateNonTrackingData = false;

	updateNonTrackingData = false;

	int trackingState = kUnityXRInputTrackingStatePosition | kUnityXRInputTrackingStateRotation | kUnityXRInputTrackingStateVelocity | kUnityXRInputTrackingStateAngularVelocity;

	if ( !trackingPose.bPoseIsValid )
		trackingState = kUnityXRInputTrackingStateNone;

	if ( ( device.characteristics & kUnityXRInputDeviceCharacteristicsHeadMounted ) == kUnityXRInputDeviceCharacteristicsHeadMounted )
	{
		s_Input->DeviceState_SetDiscreteStateValue( deviceState,
			hmdFeatureIndices[static_cast< int >( HMDFeature::TrackingState )], trackingState );
		s_Input->DeviceState_SetBinaryValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::IsTracked )], trackingPose.bPoseIsValid );

		vr::EDeviceActivityLevel activityLevel = OpenVRSystem::Get().GetSystem()->GetTrackedDeviceActivityLevel(vr::k_unTrackedDeviceIndex_Hmd);
		bool present = activityLevel == vr::k_EDeviceActivityLevel_UserInteraction;
		s_Input->DeviceState_SetBinaryValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::UserPresence )], present ); 

		UnityXRVector3 devicePosition, deviceVelocity, deviceAngularVelocity;
		UnityXRVector4 deviceRotation;
		OpenVRToUnityTracking( trackingPose, std::nullopt, devicePosition, deviceRotation, deviceVelocity, deviceAngularVelocity );
		s_Input->DeviceState_SetAxis3DValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::DevicePosition )], devicePosition );
		s_Input->DeviceState_SetRotationValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::DeviceRotation )], deviceRotation );
		s_Input->DeviceState_SetAxis3DValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::DeviceVelocity )], deviceVelocity );
		s_Input->DeviceState_SetAxis3DValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::DeviceAngularVelocity )], deviceAngularVelocity );

		UnityXRVector3 leftEyePosition, leftEyeVelocity, leftEyeAngularVelocity;
		UnityXRVector4 leftEyeRotation;
		UnityXRMatrix4x4 leftEyeTransform = GetEyeTransform( EHMDEye::Left );
		OpenVRToUnityTracking( trackingPose, { leftEyeTransform }, leftEyePosition, leftEyeRotation, leftEyeVelocity, leftEyeAngularVelocity );
		s_Input->DeviceState_SetAxis3DValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::LeftEyePosition )], leftEyePosition );
		s_Input->DeviceState_SetRotationValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::LeftEyeRotation )], leftEyeRotation );
		s_Input->DeviceState_SetAxis3DValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::LeftEyeVelocity )], leftEyeVelocity );
		s_Input->DeviceState_SetAxis3DValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::LeftEyeAngularVelocity )], leftEyeAngularVelocity );

		UnityXRVector3 rightEyePosition, rightEyeVelocity, rightEyeAngularVelocity;
		UnityXRVector4 rightEyeRotation;
		UnityXRMatrix4x4 rightEyeTransform = GetEyeTransform( EHMDEye::Right );
		OpenVRToUnityTracking( trackingPose, { rightEyeTransform }, rightEyePosition, rightEyeRotation, rightEyeVelocity, rightEyeAngularVelocity );
		s_Input->DeviceState_SetAxis3DValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::RightEyePosition )], rightEyePosition );
		s_Input->DeviceState_SetRotationValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::RightEyeRotation )], rightEyeRotation );
		s_Input->DeviceState_SetAxis3DValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::RightEyeVelocity )], rightEyeVelocity );
		s_Input->DeviceState_SetAxis3DValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::RightEyeAngularVelocity )], rightEyeAngularVelocity );

		s_Input->DeviceState_SetAxis3DValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::CenterEyePosition )], ( leftEyePosition + rightEyePosition ) * 0.5f );
		s_Input->DeviceState_SetRotationValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::CenterEyeRotation )],
			XRQuaternion::Lerp( reinterpret_cast< XRQuaternion & >( leftEyeRotation ), reinterpret_cast< XRQuaternion & >( rightEyeRotation ), 0.5f ) );
		s_Input->DeviceState_SetAxis3DValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::CenterEyeVelocity )], ( rightEyeVelocity + rightEyeVelocity ) * 0.5f );
		s_Input->DeviceState_SetAxis3DValue( deviceState, hmdFeatureIndices[static_cast< int >( HMDFeature::CenterEyeAngularVelocity )], ( leftEyeAngularVelocity + rightEyeAngularVelocity ) * 0.5f );
	}
	else if ( ( device.characteristics & kUnityXRInputDeviceCharacteristicsHeldInHand ) == kUnityXRInputDeviceCharacteristicsHeldInHand )
	{
		s_Input->DeviceState_SetDiscreteStateValue( deviceState,
			controllerFeatureIndices[static_cast< int >( ControllerFeature::TrackingState )], trackingState );
		s_Input->DeviceState_SetBinaryValue( deviceState, controllerFeatureIndices[static_cast< int >( ControllerFeature::IsTracked )], trackingPose.bPoseIsValid );

		UnityXRVector3 devicePosition, deviceVelocity, deviceAngularVelocity;
		UnityXRVector4 deviceRotation;
		OpenVRToUnityTracking( trackingPose, std::nullopt, devicePosition, deviceRotation, deviceVelocity, deviceAngularVelocity );
		s_Input->DeviceState_SetAxis3DValue( deviceState, controllerFeatureIndices[static_cast< int >( ControllerFeature::DevicePosition )], devicePosition );
		s_Input->DeviceState_SetRotationValue( deviceState, controllerFeatureIndices[static_cast< int >( ControllerFeature::DeviceRotation )], deviceRotation );
		s_Input->DeviceState_SetAxis3DValue( deviceState, controllerFeatureIndices[static_cast< int >( ControllerFeature::DeviceVelocity )], deviceVelocity );
		s_Input->DeviceState_SetAxis3DValue( deviceState, controllerFeatureIndices[static_cast< int >( ControllerFeature::DeviceAngularVelocity )], deviceAngularVelocity );
	}
	else if ( ( device.characteristics & kUnityXRInputDeviceCharacteristicsTrackingReference ) == kUnityXRInputDeviceCharacteristicsTrackingReference )
	{
		s_Input->DeviceState_SetDiscreteStateValue( deviceState,
			trackerFeatureIndices[static_cast< int >( TrackerFeature::TrackingState )], trackingState );
		s_Input->DeviceState_SetBinaryValue( deviceState, trackerFeatureIndices[static_cast< int >( TrackerFeature::IsTracked )], trackingPose.bPoseIsValid );

		UnityXRVector3 devicePosition, deviceVelocity, deviceAngularVelocity;
		UnityXRVector4 deviceRotation;
		OpenVRToUnityTracking( trackingPose, std::nullopt, devicePosition, deviceRotation, deviceVelocity, deviceAngularVelocity );
		s_Input->DeviceState_SetAxis3DValue( deviceState, trackerFeatureIndices[static_cast< int >( TrackerFeature::DevicePosition )], devicePosition );
		s_Input->DeviceState_SetRotationValue( deviceState, trackerFeatureIndices[static_cast< int >( TrackerFeature::DeviceRotation )], deviceRotation );
		s_Input->DeviceState_SetAxis3DValue( deviceState, trackerFeatureIndices[static_cast< int >( TrackerFeature::DeviceVelocity )], deviceVelocity );
		s_Input->DeviceState_SetAxis3DValue( deviceState, trackerFeatureIndices[static_cast< int >( TrackerFeature::DeviceAngularVelocity )], deviceAngularVelocity );
	}
	else if ( ( device.characteristics & kUnityXRInputDeviceCharacteristicsTrackedDevice ) == kUnityXRInputDeviceCharacteristicsTrackedDevice )
	{
		s_Input->DeviceState_SetDiscreteStateValue( deviceState,
			trackerFeatureIndices[static_cast< int >( TrackerFeature::TrackingState )], trackingState );
		s_Input->DeviceState_SetBinaryValue( deviceState, trackerFeatureIndices[static_cast< int >( TrackerFeature::IsTracked )], trackingPose.bPoseIsValid );

		UnityXRVector3 devicePosition, deviceVelocity, deviceAngularVelocity;
		UnityXRVector4 deviceRotation;
		OpenVRToUnityTracking( trackingPose, std::nullopt, devicePosition, deviceRotation, deviceVelocity, deviceAngularVelocity );
		s_Input->DeviceState_SetAxis3DValue( deviceState, trackerFeatureIndices[static_cast< int >( TrackerFeature::DevicePosition )], devicePosition );
		s_Input->DeviceState_SetRotationValue( deviceState, trackerFeatureIndices[static_cast< int >( TrackerFeature::DeviceRotation )], deviceRotation );
		s_Input->DeviceState_SetAxis3DValue( deviceState, trackerFeatureIndices[static_cast< int >( TrackerFeature::DeviceVelocity )], deviceVelocity );
		s_Input->DeviceState_SetAxis3DValue( deviceState, trackerFeatureIndices[static_cast< int >( TrackerFeature::DeviceAngularVelocity )], deviceAngularVelocity );
	}

	return kUnitySubsystemErrorCodeSuccess;
}

UnitySubsystemErrorCode UNITY_INTERFACE_API OpenVRInputProvider::UpdateDeviceState( UnitySubsystemHandle handle, UnityXRInternalInputDeviceId deviceId, UnityXRInputUpdateType updateType, UnityXRInputDeviceState *deviceState )
{
	auto device = GetTrackedDeviceByDeviceId( deviceId );
	if ( !device )
		return kUnitySubsystemErrorCodeFailure;

	UnitySubsystemErrorCode errorCode =
		Internal_UpdateDeviceState( handle, **device, ( *device )->trackingPose[updateType], deviceState, true );
	if ( errorCode != kUnitySubsystemErrorCodeSuccess )
		return errorCode;

	s_Input->DeviceState_SetDeviceTime( deviceState, GetCurrentUnityTimestamp() );
	return kUnitySubsystemErrorCodeSuccess;
}

static UnitySubsystemErrorCode RecenterTrackingOrigin()
{
	vr::ETrackingUniverseOrigin trackingSpace = OpenVRSystem::Get().GetCompositor()->GetTrackingSpace();
	switch ( trackingSpace )
	{
	case vr::TrackingUniverseSeated:
	case vr::TrackingUniverseStanding:
		vr::VRChaperone()->ResetZeroPose( trackingSpace );
		return kUnitySubsystemErrorCodeSuccess;
	default:
		break;
	}
	return kUnitySubsystemErrorCodeFailure;
}

template <class T>
inline static T clamp( const T &t, const T &t0, const T &t1 )
{
	return ( t < t0 ) ? t0 : ( ( t > t1 ) ? t1 : t );
}

UnitySubsystemErrorCode OpenVRInputProvider::SendControllerHapticImpulse( UnityXRInternalInputDeviceId deviceId, int channel, float amplitude, float duration )
{
	if ( OpenVRSystem::Get().GetSystem() == NULL )
		return kUnitySubsystemErrorCodeFailure;

	auto device = GetTrackedDeviceByDeviceId( deviceId );
	if ( !device ||
		!( ( ( *device )->characteristics & kUnityXRInputDeviceCharacteristicsHeldInHand ) == kUnityXRInputDeviceCharacteristicsHeldInHand ) )
		return kUnitySubsystemErrorCodeInvalidArguments;

	// OpenVR only takes duration and it is a max of 5µs so we map amplitude to duration to fake amplitude working.
	float amplitudeClamped = std::clamp( amplitude, 0.f, 1.f );
	const unsigned short usDurationMicroSec =
		std::min<unsigned short>(
			static_cast< unsigned short >( amplitudeClamped * kMaxHapticPulseDurationInMicroseconds ),
			kMaxHapticPulseDurationInMicroseconds );


	// A crash here might be due to shutdown order
	// This is generally called from the VrHaptics thread
	OpenVRSystem::Get().GetSystem()->TriggerHapticPulse(
		( *device )->openVRDeviceIndex,
		channel,
		usDurationMicroSec );

	return kUnitySubsystemErrorCodeSuccess;
}

UnitySubsystemErrorCode OpenVRInputProvider::GetHapticCapabilities( UnityXRInternalInputDeviceId deviceId, UnityXRHapticCapabilities *capabilities )
{
	auto device = GetTrackedDeviceByDeviceId( deviceId );
	if ( !device )
		return kUnitySubsystemErrorCodeInvalidArguments;

	bool supportsHaptics = ( ( ( *device )->characteristics & kUnityXRInputDeviceCharacteristicsHeldInHand ) == kUnityXRInputDeviceCharacteristicsHeldInHand );

	capabilities->numChannels = supportsHaptics ? kHapticsNumChannels : 0;
	capabilities->supportsImpulse = supportsHaptics;
	capabilities->supportsBuffer = false;
	capabilities->bufferFrequencyHz = 0;
	capabilities->bufferMaxSize = 0;
	capabilities->bufferOptimalSize = 0;
	return kUnitySubsystemErrorCodeSuccess;
}

UnitySubsystemErrorCode UNITY_INTERFACE_API OpenVRInputProvider::HandleEvent( UnitySubsystemHandle handle, unsigned int eventType, UnityXRInternalInputDeviceId deviceId, void *buffer, unsigned int size )
{
	// No Custom events supported at this time.
	return kUnitySubsystemErrorCodeFailure;
}

UnitySubsystemErrorCode UNITY_INTERFACE_API OpenVRInputProvider::HandleRecenter( UnitySubsystemHandle handle )
{
	return RecenterTrackingOrigin();
}

UnitySubsystemErrorCode UNITY_INTERFACE_API OpenVRInputProvider::HandleHapticImpulse( UnitySubsystemHandle handle, UnityXRInternalInputDeviceId deviceId, int channel, float amplitude, float duration )
{
	return SendControllerHapticImpulse( deviceId, channel, amplitude, duration );
}

UnitySubsystemErrorCode UNITY_INTERFACE_API OpenVRInputProvider::QueryHapticCapabilities( UnitySubsystemHandle handle, UnityXRInternalInputDeviceId deviceId, UnityXRHapticCapabilities *capabilities )
{
	return GetHapticCapabilities( deviceId, capabilities );
}

UnitySubsystemErrorCode UNITY_INTERFACE_API OpenVRInputProvider::HandleHapticStop( UnitySubsystemHandle handle, UnityXRInternalInputDeviceId deviceId )
{
	// not supported by a variety of controllers
	return kUnitySubsystemErrorCodeFailure;
}


UnitySubsystemErrorCode UNITY_INTERFACE_API OpenVRInputProvider::QueryTrackingOriginMode( UnitySubsystemHandle handle, UnityXRInputTrackingOriginModeFlags *trackingOriginMode )
{
	if ( OpenVRSystem::Get().GetCompositor() == nullptr )
	{
		*trackingOriginMode = kUnityXRInputTrackingOriginModeUnknown;
		return kUnitySubsystemErrorCodeFailure;
	}

	vr::ETrackingUniverseOrigin originType = OpenVRSystem::Get().GetCompositor()->GetTrackingSpace();
	switch ( originType )
	{
	case vr::TrackingUniverseSeated:
		*trackingOriginMode = kUnityXRInputTrackingOriginModeDevice;
		break;

	case vr::TrackingUniverseStanding:
		*trackingOriginMode = kUnityXRInputTrackingOriginModeFloor;
		break;

	case vr::TrackingUniverseRawAndUncalibrated:
	default:
		*trackingOriginMode = kUnityXRInputTrackingOriginModeUnknown;
		break;
	}

	return kUnitySubsystemErrorCodeSuccess;
}

UnitySubsystemErrorCode UNITY_INTERFACE_API OpenVRInputProvider::QuerySupportedTrackingOriginModes( UnitySubsystemHandle handle, UnityXRInputTrackingOriginModeFlags *supportedTrackingOriginModes )
{
	*supportedTrackingOriginModes = (UnityXRInputTrackingOriginModeFlags )( kUnityXRInputTrackingOriginModeDevice | kUnityXRInputTrackingOriginModeFloor );
	return kUnitySubsystemErrorCodeFailure;
}

UnitySubsystemErrorCode UNITY_INTERFACE_API OpenVRInputProvider::HandleSetTrackingOriginMode( UnitySubsystemHandle handle, UnityXRInputTrackingOriginModeFlags trackingOriginMode )
{
	UnityXRInputTrackingOriginModeFlags previousTrackingOriginMode;
	if ( QueryTrackingOriginMode( handle, &previousTrackingOriginMode ) == kUnitySubsystemErrorCodeFailure )
		return kUnitySubsystemErrorCodeFailure;

	vr::ETrackingUniverseOrigin originType;
	switch ( trackingOriginMode )
	{
	case kUnityXRInputTrackingOriginModeDevice:
		originType = vr::TrackingUniverseSeated;
		break;

	case kUnityXRInputTrackingOriginModeFloor:
		originType = vr::TrackingUniverseStanding;
		break;

	default:
		return kUnitySubsystemErrorCodeFailure;
		break;
	}
	OpenVRSystem::Get().GetCompositor()->SetTrackingSpace( originType );

	if ( previousTrackingOriginMode != trackingOriginMode )
		s_Input->InputSubsystem_TrackingOriginUpdated( handle );

	return kUnitySubsystemErrorCodeSuccess;
}

UnitySubsystemErrorCode UNITY_INTERFACE_API OpenVRInputProvider::TryGetDeviceStateAtTime( UnitySubsystemHandle handle, UnityXRTimeStamp time, UnityXRInternalInputDeviceId deviceId, UnityXRInputDeviceState *state )
{
	UnityXRTimeStamp currentTimestamp = GetCurrentUnityTimestamp();
	UnityXRTimeStamp timestampDelta = time - currentTimestamp;
	constexpr float kMillisecondsInSecond = 1000.0f;
	float deltaTimeInSeconds = static_cast< float >( timestampDelta ) / kMillisecondsInSecond;

	vr::TrackedDevicePose_t trackedDevicesAtTimestamp[vr::k_unMaxTrackedDeviceCount];
	vr::ETrackingUniverseOrigin trackingSpace = OpenVRSystem::Get().GetCompositor()->GetTrackingSpace();
	OpenVRSystem::Get().GetSystem()->GetDeviceToAbsoluteTrackingPose( trackingSpace, deltaTimeInSeconds, trackedDevicesAtTimestamp, vr::k_unMaxTrackedDeviceCount );

	auto device = GetTrackedDeviceByDeviceId( deviceId );
	if ( !device )
		return kUnitySubsystemErrorCodeFailure;

	assert( ( *device )->openVRDeviceIndex < vr::k_unMaxTrackedDeviceCount );
	if ( ( *device )->openVRDeviceIndex >= vr::k_unMaxTrackedDeviceCount )
		return kUnitySubsystemErrorCodeFailure;

	UnitySubsystemErrorCode errorCode =
		Internal_UpdateDeviceState( handle, **device, trackedDevicesAtTimestamp[( *device )->openVRDeviceIndex], state, false );
	if ( errorCode != kUnitySubsystemErrorCodeSuccess )
		return errorCode;

	s_Input->DeviceState_SetDeviceTime( state, time );
	return kUnitySubsystemErrorCodeSuccess;
}

UnityXRInputDeviceCharacteristics GetCharacteristicsForDeviceIndex( vr::TrackedDeviceIndex_t deviceIndex )
{
	const vr::ETrackedDeviceClass trackedDeviceClass = OpenVRSystem::Get().GetSystem()->GetTrackedDeviceClass( deviceIndex );

	switch ( trackedDeviceClass )
	{
	case vr::TrackedDeviceClass_HMD:
		return (UnityXRInputDeviceCharacteristics )( kUnityXRInputDeviceCharacteristicsHeadMounted | kUnityXRInputDeviceCharacteristicsTrackedDevice );
		break;
	case vr::TrackedDeviceClass_Controller:
	{
		vr::ETrackedControllerRole openVRRole = OpenVRSystem::Get().GetSystem()->GetControllerRoleForTrackedDeviceIndex( deviceIndex );
		switch ( openVRRole )
		{
		case vr::TrackedControllerRole_LeftHand:
			return (UnityXRInputDeviceCharacteristics )( kUnityXRInputDeviceCharacteristicsHeldInHand | kUnityXRInputDeviceCharacteristicsController | kUnityXRInputDeviceCharacteristicsTrackedDevice | kUnityXRInputDeviceCharacteristicsLeft );
		case vr::TrackedControllerRole_RightHand:
			return (UnityXRInputDeviceCharacteristics )( kUnityXRInputDeviceCharacteristicsHeldInHand | kUnityXRInputDeviceCharacteristicsController | kUnityXRInputDeviceCharacteristicsTrackedDevice | kUnityXRInputDeviceCharacteristicsRight );
		case vr::TrackedControllerRole_Stylus: //logitech doesn't use this yet, but they will at some point
			return (UnityXRInputDeviceCharacteristics )( kUnityXRInputDeviceCharacteristicsHeldInHand | kUnityXRInputDeviceCharacteristicsController | kUnityXRInputDeviceCharacteristicsTrackedDevice );
		case vr::TrackedControllerRole_Treadmill:
			return (UnityXRInputDeviceCharacteristics )( kUnityXRInputDeviceCharacteristicsController | kUnityXRInputDeviceCharacteristicsTrackedDevice );
		case vr::TrackedControllerRole_Invalid: // Hardware trackers can be mapped to invalid controller
			return (UnityXRInputDeviceCharacteristics )( kUnityXRInputDeviceCharacteristicsTrackedDevice | kUnityXRInputDeviceCharacteristicsController | kUnityXRInputDeviceCharacteristicsHeldInHand );
		}
		break;
	}
	case vr::TrackedDeviceClass_TrackingReference:
		return (UnityXRInputDeviceCharacteristics )( kUnityXRInputDeviceCharacteristicsTrackingReference | kUnityXRInputDeviceCharacteristicsTrackedDevice );
		break;
	case vr::TrackedDeviceClass_GenericTracker:
	{
		vr::ETrackedControllerRole openVRRole = OpenVRSystem::Get().GetSystem()->GetControllerRoleForTrackedDeviceIndex( deviceIndex );
		switch ( openVRRole )
		{
		case vr::TrackedControllerRole_LeftHand:
			return (UnityXRInputDeviceCharacteristics )( kUnityXRInputDeviceCharacteristicsHeldInHand | kUnityXRInputDeviceCharacteristicsController | kUnityXRInputDeviceCharacteristicsTrackedDevice | kUnityXRInputDeviceCharacteristicsLeft );
		case vr::TrackedControllerRole_RightHand:
			return (UnityXRInputDeviceCharacteristics )( kUnityXRInputDeviceCharacteristicsHeldInHand | kUnityXRInputDeviceCharacteristicsController | kUnityXRInputDeviceCharacteristicsTrackedDevice | kUnityXRInputDeviceCharacteristicsRight );
		case vr::TrackedControllerRole_Stylus:
			return (UnityXRInputDeviceCharacteristics )( kUnityXRInputDeviceCharacteristicsHeldInHand | kUnityXRInputDeviceCharacteristicsController | kUnityXRInputDeviceCharacteristicsTrackedDevice );
		case vr::TrackedControllerRole_Treadmill:
			return (UnityXRInputDeviceCharacteristics )( kUnityXRInputDeviceCharacteristicsController | kUnityXRInputDeviceCharacteristicsTrackedDevice );
		case vr::TrackedControllerRole_Invalid: // Hardware trackers can be mapped to invalid controller
			return (UnityXRInputDeviceCharacteristics )( kUnityXRInputDeviceCharacteristicsTrackedDevice );
		}
		return kUnityXRInputDeviceCharacteristicsTrackedDevice;
		break;
	}
	default:
		//XR_TRACE( "Get characteristics for device with invalid class. DeviceIndex: %d. Class: %d\n", deviceIndex, trackedDeviceClass );
		break;
	}

	return kUnityXRInputDeviceCharacteristicsNone;
}

UnityXRInternalInputDeviceId OpenVRInputProvider::GenerateUniqueDeviceId() const
{
	std::vector<UnityXRInternalInputDeviceId> sortedDeviceIds;
	for ( auto device : m_TrackedDevices )
		sortedDeviceIds.emplace_back( device.deviceId );

	std::sort( sortedDeviceIds.begin(), sortedDeviceIds.end() ); //wasn't necessarily sorted. 

	UnityXRInternalInputDeviceId uniqueId = 0;
	for ( auto deviceId : sortedDeviceIds )
	{
		if ( deviceId == uniqueId )
			uniqueId++;
		else
			break;
	}

	return uniqueId;
}

void OpenVRInputProvider::GfxThread_UpdateConnectedDevices( const vr::TrackedDevicePose_t *currentDevicePoses )
{
	for ( unsigned int openVRTrackedDeviceIndex = 0; openVRTrackedDeviceIndex < vr::k_unMaxTrackedDeviceCount; ++openVRTrackedDeviceIndex )
	{
		const bool isConnected = OpenVRSystem::Get().GetSystem()->IsTrackedDeviceConnected( openVRTrackedDeviceIndex );
		auto existingDevice = GetTrackedDeviceByOpenVRIndex( openVRTrackedDeviceIndex );

		if ( !isConnected )
		{
			// Device was in list but is no longer tracked, mark for disconnect
			if ( existingDevice )
			{
				( *existingDevice )->deviceChangeForNextUpdate = EDeviceStatus::Disconnect;

				XR_TRACE( "[OpenVR] Device disconnecting (disconnection reported). OpenVRIndex: %d. UnityID: %d\n", openVRTrackedDeviceIndex, ( *existingDevice )->deviceId );
			}
		}
		else
		{
			// Device was not in list but is now tracked, add to tracked devices (is constructed marked for connect)
			UnityXRInputDeviceCharacteristics characteristics = GetCharacteristicsForDeviceIndex( openVRTrackedDeviceIndex );
			if ( !existingDevice )
			{
				if ( characteristics != kUnityXRInputDeviceCharacteristicsNone )
				{
					UnityXRInternalInputDeviceId newDeviceId = GenerateUniqueDeviceId();
					m_TrackedDevices.emplace_back( openVRTrackedDeviceIndex, newDeviceId, characteristics );
					XR_TRACE( "[OpenVR] Device connecting (status change). OpenVRIndex: %d. UnityID: %d\n", openVRTrackedDeviceIndex, newDeviceId );
				}
			}
			else if ( ( *existingDevice )->characteristics != characteristics ) // Need to check to see if characteristics changed, if so disconnect and allow reconnection next frame
			{
				XR_TRACE( "[OpenVR] Device disconnecting (characteristics change). OpenVRIndex: %d. UnityID: %d\n", openVRTrackedDeviceIndex, ( *existingDevice )->deviceId );
				( *existingDevice )->deviceChangeForNextUpdate = EDeviceStatus::Disconnect;
			}
		}
	}
}

void OpenVRInputProvider::GfxThread_CopyPoses( const vr::TrackedDevicePose_t *currentDevicePoses, const vr::TrackedDevicePose_t *futureDevicePoses )
{
	for ( auto &trackedDevice : m_TrackedDevices )
	{
		trackedDevice.trackingPose[kUnityXRInputUpdateTypeDynamic] = futureDevicePoses[trackedDevice.openVRDeviceIndex];
		trackedDevice.trackingPose[kUnityXRInputUpdateTypeBeforeRender] = currentDevicePoses[trackedDevice.openVRDeviceIndex];
	}
}

// Called from the graphics thread in post-present to get connected devices and update poses.
// The graphics thread will have a sync fence with the main loop, so thread synchronization is not further necessary.
void OpenVRInputProvider::GfxThread_UpdateDevices()
{
	if ( !m_Started )
		return;

	vr::TrackedDevicePose_t trackedDevicesCurrent[vr::k_unMaxTrackedDeviceCount];
	vr::TrackedDevicePose_t trackedDevicesFuture[vr::k_unMaxTrackedDeviceCount];

	if ( UserProjectSettings::GetInitializationType() == vr::VRApplication_Overlay )
	{
		OpenVRSystem::Get().GetSystem()->GetDeviceToAbsoluteTrackingPose( vr::TrackingUniverseStanding, 0.0f, trackedDevicesCurrent, vr::k_unMaxTrackedDeviceCount );
		OpenVRSystem::Get().GetSystem()->GetDeviceToAbsoluteTrackingPose( vr::TrackingUniverseStanding, 0.011f, trackedDevicesFuture, vr::k_unMaxTrackedDeviceCount );
	}
	else
	{
		OpenVRSystem::Get().GetCompositor()->WaitGetPoses( trackedDevicesCurrent, vr::k_unMaxTrackedDeviceCount, trackedDevicesFuture, vr::k_unMaxTrackedDeviceCount );
	}

	GfxThread_UpdateConnectedDevices( trackedDevicesCurrent );
	GfxThread_CopyPoses( trackedDevicesCurrent, trackedDevicesFuture );
}

UnitySubsystemErrorCode OpenVRInputProvider::Start()
{
	m_Started = true;

	return kUnitySubsystemErrorCodeSuccess;
}

void OpenVRInputProvider::Stop( UnitySubsystemHandle handle )
{
	m_Started = false;

	for ( auto deviceIter = m_TrackedDevices.begin(); deviceIter != m_TrackedDevices.end(); )
	{
		if ( deviceIter->deviceStatus == EDeviceStatus::Connect )
		{
			XR_TRACE( "[OpenVR] Device disconnected (stopping provider). Handle: %d. DeviceID: %d\n", handle, deviceIter->deviceId );
			s_Input->InputSubsystem_DeviceDisconnected( handle, deviceIter->deviceId );
		}
		deviceIter = m_TrackedDevices.erase( deviceIter );
	}
	m_TrackedDevices.clear();
}

UnitySubsystemErrorCode UNITY_INTERFACE_API Lifecycle_Initialize( UnitySubsystemHandle handle, void *userData )
{
	// Register to the provider context
	if ( s_pProviderContext )
	{
		s_pProviderContext->inputProvider = &OpenVRInputProvider::Get();
	}

	UnityXRInputProvider inputProvider = { 0 };

	inputProvider.userData = &OpenVRInputProvider::Get();
	inputProvider.Tick = &Tick;
	inputProvider.FillDeviceDefinition = &FillDeviceDefinition;
	inputProvider.UpdateDeviceState = &UpdateDeviceState;
	inputProvider.HandleEvent = &HandleEvent;
	inputProvider.HandleRecenter = &HandleRecenter;
	inputProvider.HandleHapticImpulse = &HandleHapticImpulse;
	inputProvider.HandleHapticBuffer = &HandleHapticBuffer;
	inputProvider.QueryHapticCapabilities = &QueryHapticCapabilities;
	inputProvider.HandleHapticStop = &HandleHapticStop;
	inputProvider.QueryTrackingOriginMode = &QueryTrackingOriginMode;
	inputProvider.QuerySupportedTrackingOriginModes = &QuerySupportedTrackingOriginModes;
	inputProvider.HandleSetTrackingOriginMode = &HandleSetTrackingOriginMode;
	inputProvider.TryGetDeviceStateAtTime = &TryGetDeviceStateAtTime;

	s_Input->RegisterInputProvider( handle, &inputProvider );

	return kUnitySubsystemErrorCodeSuccess;
}

static UnitySubsystemErrorCode UNITY_INTERFACE_API Lifecycle_Start( UnitySubsystemHandle handle, void *userData )
{
	return OpenVRInputProvider::Get().Start();
}

static void UNITY_INTERFACE_API Lifecycle_Stop( UnitySubsystemHandle handle, void *userData )
{
	OpenVRInputProvider::Get().Stop( handle );
}

static void UNITY_INTERFACE_API Lifecycle_Shutdown( UnitySubsystemHandle handle, void *userData )
{
}

bool RegisterInputLifecycleProvider( OpenVRProviderContext *pOpenProviderContext )
{
	XR_TRACE( "[OpenVR] Input lifecycle provider registered\n" );

	s_Input = UnityInterfaces::Get().GetInterface<IUnityXRInputInterface>();
	s_pProviderContext = pOpenProviderContext;

	UnityLifecycleProvider inputLifecycleHandler = { 0 };

	inputLifecycleHandler.userData = nullptr;
	inputLifecycleHandler.Initialize = &Lifecycle_Initialize;
	inputLifecycleHandler.Start = &Lifecycle_Start;
	inputLifecycleHandler.Stop = &Lifecycle_Stop;
	inputLifecycleHandler.Shutdown = &Lifecycle_Shutdown;

	s_Input->RegisterLifecycleProvider( "XRSDKOpenVR", "OpenVR Input", &inputLifecycleHandler );

	return true;
}
