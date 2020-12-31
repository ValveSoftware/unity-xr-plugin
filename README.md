# OpenVR Unity XR Plugin

The purpose of this package is to provide OpenVR rendering to Unity XR. This package provides the necessary sdk libraries for users to build Applications that work with the OpenVR runtime. The OpenVR XR Plugin gives you access to rendering on all major VR devices through one interface. Explicit support for: HTC Vive, HTC Vive Cosmos, Oculus Rift, Oculus Rift S, Oculus Quest (Link), Windows Mixed Reality, and Valve Index. Other SteamVR compatible devices are supported though may have inaccurate or incomplete features. See our [QuickStart guide](#QuickStart).

## License
This project is under the BSD 3 Clause license ([/LICENSE](LICENSE)) except the unity headers in the CommonHeaders/ProviderInterface folder, where you'll find the Unity Companion License ([/CommonHeaders/ProviderInterface/LICENSE.md](CommonHeaders/ProviderInterface/LICENSE.md))

## Documentation

There is some brief documentation included with this plugin at [/Documentation~/com.valvesoftware.unity.openvr.md](com.valve.openvr/Documentation~/com.valvesoftware.unity.openvr.md)

## Input

As part of [our commitment to OpenXR](https://store.steampowered.com/newshub/app/250820/view/2396425843528787269) we will begin targeting the OpenXR API in future versions of our game engine plugins.

For now, to get access to controllers and other forms of input you will need to install the beta version of the SteamVR Unity Plugin. This can be found at (https://github.com/ValveSoftware/steamvr_unity_plugin/releases/tag/2.6.0b4).

## Known Issues:
* Display Provider
  * OpenVR Mirror View Mode can cause black screens in the game view. Please send us bug reports if this happens.
  * OpenVR Mirror View Mode requires use of Linear Color Space (Project Settings > Player > Other Settings > (Rendering) Color Space)
  * In certain use cases, changing RenderScale and ViewPortScale in runtime causes some performance spikes

## Bug reports:
* For bug reports please create an issue on our github (https://github.com/ValveSoftware/unity-xr-plugin/issues) and include the following information
  * Detailed steps of what you were doing at the time
  * Your editor or build log (editor log location: %LOCALAPPDATA%\Unity\Editor\Editor.log)
  * A SteamVR System report DIRECTLY AFTER encountering the issue. (SteamVR interface -> Menu -> Create System Report -> Save to file)

## Structure
### Unity Package
There is a C# project in [/com.valve.openvr](com.valve.openvr) which is the unity package that does the high level loading and configuration for the plugin. It also manages OpenVR Events. We've included pre-built DLLs in this package for ease of access.
### Native Plugin
The brains of the plugin are in a native dll at [com.valve.openvr/Runtime/x64/XRSDKOpenVR.dll](com.valve.openvr/Runtime/x64/XRSDKOpenVR.dll). The source for this dll is in the rest of this project, but mainly in the [/Providers](Providers) directory.
### NPM Installer
For developers not building this package from source we have a separate Installer package which adds a scoped registry to your project's package manifest and then installs the OpenVR Unity XR Plugin from [npm](https://www.npmjs.com/package/com.valvesoftware.unity.openvr). This also lets you easily upgrade the plugin within the unity package manager. The code for this installer lives in the [Installer branch](https://github.com/ValveSoftware/unity-xr-plugin/tree/Installer).

## Building from source
1. Clone this GitHub repo to your local machine
2. Open a Powershell window in the root directory of your cloned repo
3. Run CMake to generate appropriate build files, such as:

    `cmake -G "Visual Studio 16 2019" .`

    `cmake -G "Visual Studio 16 2019" -A Win32 .`

    `cmake -G "Visual Studio 15 2017 Win64" .`

4. (Optional) Compile the runtime using CMake. Make sure you have MS Visual Studio 2017 or 2019 with C++ support installed:

    `cmake --build . --target ALL_BUILD --config Debug`

    `cmake --build . --target ALL_BUILD --config Release`


## QuickStart

### Installation
* Download the installer: https://github.com/ValveSoftware/unity-xr-plugin/releases/tag/installer
* Open your unity project and then open/import the unitypackage
* Open the XR Management UI (Edit Menu -> Project Settings -> XR Plugin Management)
* Click the checkbox next to OpenVR Loader - or in older versions - Under Plugin Providers hit the + icon and add “Open VR Loader”


### Standalone (no input)
* Add a cube to the scene (scale to 0.1)
* Add a Camera component to the cube
* Add TrackedPoseDriver to the cube
 *	Main Camera: Under Tracked Pose Driver:
    * For Device select: “Generic XR Device”
    * For Pose Source select: “Center Eye - HMD Reference”
* Hit play and you should see a tracked camera


### SteamVR Input System:
* Install SteamVR Unity Plugin v2.6.1+ (https://github.com/ValveSoftware/steamvr_unity_plugin/releases/)
* It should install the OpenVR XR API package automatically for 2020.1+ for 2019.3/4 you’ll need to add it with the instructions above.
* Open the SteamVR Input window (Window -> SteamVR Input)
* Accept the default json
* Click Save and Generate
* Open the Interactions_Example scene (Assets/SteamVR/InteractionSystem/Samples/Interaction_Example.unity)
* Hit play, verify that you can see your hands and teleport around


### Package installation from source
* Open the Unity Package Manager (Window Menu -> Package Manager)
* Click the plus in the upper lefthand corner and select "Add package from disk..."
* Select the package.json in the com.valve.openvr directory
* Open the XR Management UI (Edit Menu -> Project Settings -> XR Plugin Management)
* Click the checkbox next to OpenVR Loader - or in older versions - under Plugin Providers hit the + icon and add “Open VR Loader”