#pragma once

#include "ProviderInterface/IUnityXRInput.h"
#include "OpenVRSystem.h"
#include "OpenVRProviderContext.h"
#include "Singleton.h"
#include "CommonTypes.h"

#include <string>
#include <optional>
#include <variant>
#include <vector>


bool RegisterInputLifecycleProvider( OpenVRProviderContext *pOpenProviderContext );

class OpenVRInputProvider : public Singleton<OpenVRInputProvider>
{
public:
	UnitySubsystemErrorCode UNITY_INTERFACE_API Tick( UnitySubsystemHandle handle, UnityXRInputUpdateType updateType );
	UnitySubsystemErrorCode UNITY_INTERFACE_API FillDeviceDefinition( UnitySubsystemHandle handle, UnityXRInternalInputDeviceId deviceId, UnityXRInputDeviceDefinition *deviceDefinition );
	UnitySubsystemErrorCode UNITY_INTERFACE_API UpdateDeviceState( UnitySubsystemHandle handle, UnityXRInternalInputDeviceId deviceId, UnityXRInputUpdateType updateType, UnityXRInputDeviceState *deviceState );
	UnitySubsystemErrorCode UNITY_INTERFACE_API HandleEvent( UnitySubsystemHandle handle, unsigned int eventType, UnityXRInternalInputDeviceId deviceId, void *buffer, unsigned int size );

	UnitySubsystemErrorCode UNITY_INTERFACE_API HandleRecenter( UnitySubsystemHandle handle );
	UnitySubsystemErrorCode UNITY_INTERFACE_API HandleHapticImpulse( UnitySubsystemHandle handle, UnityXRInternalInputDeviceId deviceId, int channel, float amplitude, float duration );
	UnitySubsystemErrorCode UNITY_INTERFACE_API QueryHapticCapabilities( UnitySubsystemHandle handle, UnityXRInternalInputDeviceId deviceId, UnityXRHapticCapabilities *capabilities );
	UnitySubsystemErrorCode UNITY_INTERFACE_API HandleHapticStop( UnitySubsystemHandle handle, UnityXRInternalInputDeviceId deviceId );

	UnitySubsystemErrorCode UNITY_INTERFACE_API QueryTrackingOriginMode( UnitySubsystemHandle handle, UnityXRInputTrackingOriginModeFlags *trackingOriginMode );
	UnitySubsystemErrorCode UNITY_INTERFACE_API QuerySupportedTrackingOriginModes( UnitySubsystemHandle handle, UnityXRInputTrackingOriginModeFlags *supportedTrackingOriginModes );
	UnitySubsystemErrorCode UNITY_INTERFACE_API HandleSetTrackingOriginMode( UnitySubsystemHandle handle, UnityXRInputTrackingOriginModeFlags trackingOriginMode );

	UnitySubsystemErrorCode UNITY_INTERFACE_API TryGetDeviceStateAtTime( UnitySubsystemHandle handle, UnityXRTimeStamp time, UnityXRInternalInputDeviceId deviceId, UnityXRInputDeviceState *state );

	UnitySubsystemErrorCode Start();
	void Stop( UnitySubsystemHandle handle );

	void GfxThread_UpdateDevices();

private:

	enum class EDeviceStatus
	{
		None,
		Connect,
		Disconnect
	};

	enum class EHMDEye
	{
		Left,
		Right,
		Center,
		Total
	};

	enum class HMDFeature
	{
		TrackingState,
		IsTracked,
		DevicePosition,
		DeviceRotation,
		DeviceVelocity,
		DeviceAngularVelocity,
		LeftEyePosition,
		LeftEyeRotation,
		LeftEyeVelocity,
		LeftEyeAngularVelocity,
		RightEyePosition,
		RightEyeRotation,
		RightEyeVelocity,
		RightEyeAngularVelocity,
		CenterEyePosition,
		CenterEyeRotation,
		CenterEyeVelocity,
		CenterEyeAngularVelocity,
        UserPresence,
		Total
	};

	enum class ControllerFeature
	{
		TrackingState,
		IsTracked,
		DevicePosition,
		DeviceRotation,
		DeviceVelocity,
		DeviceAngularVelocity,
		Primary2DAxis,
		Primary2DAxisClick,
		Primary2DAxisTouch,
		Secondary2DAxis,
		Secondary2DAxisTouch,
		Secondary2DAxisClick,
		Trigger,
		TriggerButton,
		TriggerTouch,
		Grip, //this is force for index. 
		GripButton,
		GripTouch,
		GripGrab,
		GripCapacitive,
		Primary,
		PrimaryButton,
		PrimaryTouch,
		Secondary,
		SecondaryButton,
		SecondaryTouch,
		MenuButton,
		MenuTouch,
		BumperButton,
		Tip,
		TipButton,
		TipTouch,
		Total
	};

	enum class TrackerFeature
	{
		TrackingState,
		IsTracked,
		DevicePosition,
		DeviceRotation,
		DeviceVelocity,
		DeviceAngularVelocity,
		Total
	};

	enum class ControllerInputProfile
	{
		Undefined,
		ViveController,
		ViveCosmosController,
		OculusTouch,
		Knuckles,
		HolographicController,
		LogitechStylus,
		ViveTracker,
		ViveTrackerHanded,
		Total
	};

	ControllerInputProfile controllerInputProfile = ControllerInputProfile::Undefined;

	static int hmdFeatureIndices[static_cast< int >( HMDFeature::Total )];
	static int controllerFeatureIndices[static_cast< int >( ControllerFeature::Total )];
	static int trackerFeatureIndices[static_cast< int >( TrackerFeature::Total )];
	static const int kUnityXRInputUpdateTypeCount = 2;
	bool m_Started = false;

	struct OpenVRDevice
	{
		UnityXRInternalInputDeviceId deviceId = kUnityInvalidXRInternalInputDeviceId;
		vr::TrackedDeviceIndex_t openVRDeviceIndex = 0;
		UnityXRInputDeviceCharacteristics characteristics = kUnityXRInputDeviceCharacteristicsNone;
		EDeviceStatus deviceStatus = EDeviceStatus::None;
		EDeviceStatus deviceChangeForNextUpdate = EDeviceStatus::None;
		vr::TrackedDevicePose_t trackingPose[kUnityXRInputUpdateTypeCount];

		OpenVRDevice( vr::TrackedDeviceIndex_t index, UnityXRInternalInputDeviceId id, UnityXRInputDeviceCharacteristics characteristics )
			: deviceId( id )
			, openVRDeviceIndex( index )
			, characteristics( characteristics )
			, deviceStatus( EDeviceStatus::None )
			, deviceChangeForNextUpdate( EDeviceStatus::Connect )
			, trackingPose()
		{
		}

		std::optional<std::string> GetDeviceName() const;
	};

	std::vector<OpenVRInputProvider::OpenVRDevice> m_TrackedDevices;

	inline std::optional<OpenVRDevice *> GetTrackedDeviceByDeviceId( UnityXRInternalInputDeviceId id )
	{
		for ( auto &trackedDevice : m_TrackedDevices )
		{
			if ( trackedDevice.deviceId == id )
				return { &trackedDevice };
		}
		return std::nullopt;
	}
	inline std::optional<OpenVRDevice *> GetTrackedDeviceByOpenVRIndex( vr::TrackedDeviceIndex_t idx )
	{
		for ( auto &trackedDevice : m_TrackedDevices )
		{
			if ( trackedDevice.openVRDeviceIndex == idx )
				return { &trackedDevice };
		}
		return std::nullopt;
	}

	UnitySubsystemErrorCode SendControllerHapticImpulse( UnityXRInternalInputDeviceId deviceId, int channel, float amplitude, float duration );
	UnitySubsystemErrorCode GetHapticCapabilities( UnityXRInternalInputDeviceId deviceId, UnityXRHapticCapabilities *capabilities );
	UnityXRInternalInputDeviceId GenerateUniqueDeviceId() const;
	void GfxThread_UpdateConnectedDevices( const vr::TrackedDevicePose_t *currentDevicePoses );
	static UnityXRMatrix4x4 GetEyeTransform( EHMDEye eye );
	UnitySubsystemErrorCode Internal_UpdateDeviceState( UnitySubsystemHandle handle, const OpenVRDevice &device,
		const vr::TrackedDevicePose_t &trackingPose, UnityXRInputDeviceState *deviceState, bool updateNonTrackingData );
	static void OpenVRToUnityTracking( const vr::TrackedDevicePose_t &openVRPose, std::optional<UnityXRMatrix4x4> postTransform,
		UnityXRVector3 &outPosition, UnityXRVector4 &outRotation, UnityXRVector3 &outVelocity, UnityXRVector3 &outAngularVelocity );
	void GfxThread_CopyPoses( const vr::TrackedDevicePose_t *currentDevicePoses, const vr::TrackedDevicePose_t *futureDevicePoses );
};
