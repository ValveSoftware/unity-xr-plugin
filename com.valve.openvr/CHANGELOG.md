# Changelog
All notable changes to this package will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [1.0.1] - 2020-08-03
### Changed 
- Updating version of legacyinputhelpers

## [1.0.0-preview.11] - 2020-07-28
### Changed 
- Misc fixes to support linux builds. Linux is still WIP but these fixes affect the windows builds too.

## [1.0.0-preview.10] - 2020-07-22
### Changed 
- Fixing gitignore for some pdb meta files

## [1.0.0-preview.9] - 2020-07-22
### Changed 
- Statically linking vs debug runtime so people without vs can use the debug dlls

## [1.0.0-preview.8] - 2020-07-22
### Changed 
- Adding supporting debug dlls

## [1.0.0-preview.7] - 2020-07-21
### Changed 
- Requiring version 3.2.13 of the XR Management package due to critical errors in previous versions.

## [1.0.0-preview.6] - 2020-07-17
### Changed 
- Adding some subsystem failure cases to TrackingOrigin queries
- Switching to debug dlls. If you would like release DLLs either wait for v1.0 or build them from source.

## [1.0.0-preview.5] - 2020-07-15
### Changed 
- Fixed 32bit builds
- Fixed UWP builds
- Fixed IL2CPP builds
- Fixed some issues with Mirror View Mode. Now defaulting to Right Eye instead of OpenVR.
- Fixed issue where SteamVR Beta was required.

## [1.0.0-preview.4] - 2020-06-24
### Changed
- Switched from legacy input (Unity XR) to SteamVR Input via the SteamVR Unity Plugin for controller state processing
- Renamed package from com.valve.openvr to com.valvesoftware.unity.openvr
- Fixed some pathing issues with action manifests
- Fixed some rendering issues

## [1.0.0-preview.3] - 2020-06-24
### Added 
- Added NPM support
- Added some extra logging
- Added support for Unity 2019.4

### Changed
- Renamed package from com.valve.openvr to com.valvesoftware.unity.openvr
- Fixed some pathing issues with action manifests
- Fixed some rendering issues

## Removed
- Legacy Input support. To get input for now you'll need to use the SteamVR Unity Plugin.

## [1.0.0-preview.2] - 2020-04-29
### Added 
- Fixed some of the layouts, a fix for a black screen issue

## [1.0.0-preview.1] - 2020-04-29
### Added 
- Initial beta release