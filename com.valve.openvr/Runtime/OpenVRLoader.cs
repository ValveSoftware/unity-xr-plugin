#if UNITY_XR_MANAGEMENT
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.XR;
using UnityEngine.Experimental.XR;
using UnityEngine.XR.Management;
using System.IO;
using Valve.VR;

#if UNITY_INPUT_SYSTEM
using UnityEngine.InputSystem;
using UnityEngine.InputSystem.Layouts;
using UnityEngine.InputSystem.XR;
#endif

#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Unity.XR.OpenVR
{
#if UNITY_INPUT_SYSTEM
#if UNITY_EDITOR
    [InitializeOnLoad]
#endif
    static class InputLayoutLoader
    {
        static InputLayoutLoader()
        {
            RegisterInputLayouts();
        }

        public static void RegisterInputLayouts()
        {
            InputSystem.RegisterLayout<Unity.XR.OpenVR.OpenVRHMD>("OpenVRHMD",
                matches: new InputDeviceMatcher()
                    .WithInterface(XRUtilities.InterfaceMatchAnyVersion)
                    .WithProduct(@"^(OpenVR Headset)|^(Vive Pro)")
            );

            InputSystem.RegisterLayout<Unity.XR.OpenVR.OpenVRControllerWMR>("OpenVRControllerWMR",
                matches: new InputDeviceMatcher()
                    .WithInterface(XRUtilities.InterfaceMatchAnyVersion)
                    .WithProduct(@"^(OpenVR Controller\(WindowsMR)")
            );

            InputSystem.RegisterLayout<Unity.XR.OpenVR.ViveWand>("ViveWand",
                matches: new InputDeviceMatcher()
                    .WithInterface(XRUtilities.InterfaceMatchAnyVersion)
                    .WithManufacturer("HTC")
                    .WithProduct(@"^(OpenVR Controller\(((Vive Controller)|(VIVE Controller)))")
            );

            InputSystem.RegisterLayout<Unity.XR.OpenVR.OpenVRViveCosmosController>("OpenVRViveCosmosController",
                matches: new InputDeviceMatcher()
                    .WithInterface(XRUtilities.InterfaceMatchAnyVersion)
                    .WithManufacturer("HTC")
                    .WithProduct(@"^(OpenVR Controller\(((VIVE Cosmos Controller)|(Vive Cosmos Controller)|(vive_cosmos_controller)))")
            );

            InputSystem.RegisterLayout<Unity.XR.OpenVR.OpenVRControllerIndex>("OpenVRControllerIndex",
                matches: new InputDeviceMatcher()
                    .WithInterface(XRUtilities.InterfaceMatchAnyVersion)
                    .WithManufacturer("Valve")
                    .WithProduct(@"^(OpenVR Controller\(Knuckles)")
            );

            InputSystem.RegisterLayout<Unity.XR.OpenVR.OpenVROculusTouchController>("OpenVROculusTouchController",
                matches: new InputDeviceMatcher()
                    .WithInterface(XRUtilities.InterfaceMatchAnyVersion)
                    .WithManufacturer("Oculus")
                    .WithProduct(@"^(OpenVR Controller\(Oculus)")
            );

            InputSystem.RegisterLayout<Unity.XR.OpenVR.HandedViveTracker>("HandedViveTracker",
                matches: new InputDeviceMatcher()
                    .WithInterface(XRUtilities.InterfaceMatchAnyVersion)
                    .WithManufacturer("HTC")
                    .WithProduct(@"^(OpenVR Controller\(((Vive Tracker)|(VIVE Tracker)).+ - ((Left)|(Right)))")
            );

            InputSystem.RegisterLayout<Unity.XR.OpenVR.ViveTracker>("ViveTracker",
                matches: new InputDeviceMatcher()
                    .WithInterface(XRUtilities.InterfaceMatchAnyVersion)
                    .WithManufacturer("HTC")
                    .WithProduct(@"^(OpenVR Controller\(((Vive Tracker)|(VIVE Tracker)).+\)(?! - Left| - Right))")
            );

            InputSystem.RegisterLayout<Unity.XR.OpenVR.ViveTracker>("ViveTracker",
                matches: new InputDeviceMatcher()
                    .WithInterface(XRUtilities.InterfaceMatchAnyVersion)
                    .WithManufacturer("HTC")
                    .WithProduct(@"^(OpenVR Tracked Device\(((Vive Tracker)|(VIVE Tracker)).+\)(?! - Left| - Right))")
            );

            InputSystem.RegisterLayout<Unity.XR.OpenVR.LogitechStylus>("LogitechStylus",
                matches: new InputDeviceMatcher()
                    .WithInterface(XRUtilities.InterfaceMatchAnyVersion)
                    .WithManufacturer("Logitech")
                    .WithProduct(@"(OpenVR Controller\(.+stylus)")
            );

            InputSystem.RegisterLayout<Unity.XR.OpenVR.ViveLighthouse>("ViveLighthouse",
                matches: new InputDeviceMatcher()
                    .WithInterface(XRUtilities.InterfaceMatchAnyVersion)
                    .WithManufacturer("HTC")
                    .WithProduct(@"^(OpenVR Tracking Reference\()")
            );

            InputSystem.RegisterLayout<Unity.XR.OpenVR.ValveLighthouse>("ValveLighthouse",
                matches: new InputDeviceMatcher()
                    .WithInterface(XRUtilities.InterfaceMatchAnyVersion)
                    .WithManufacturer("Valve Corporation")
                    .WithProduct(@"^(OpenVR Tracking Reference\()")
            );
        }
    }
#endif

    public class OpenVRLoader : XRLoaderHelper
#if UNITY_EDITOR
    , IXRLoaderPreInit
#endif
    {
        private static List<XRDisplaySubsystemDescriptor> s_DisplaySubsystemDescriptors = new List<XRDisplaySubsystemDescriptor>();
        private static List<XRInputSubsystemDescriptor> s_InputSubsystemDescriptors = new List<XRInputSubsystemDescriptor>();
        

        public XRDisplaySubsystem displaySubsystem
        {
            get
            {
                return GetLoadedSubsystem<XRDisplaySubsystem>();
            }
        }

        public XRInputSubsystem inputSubsystem
        {
            get
            {
                return GetLoadedSubsystem<XRInputSubsystem>();
            }
        }

        public override bool Initialize()
        {
#if UNITY_INPUT_SYSTEM
            //InputLayoutLoader.RegisterInputLayouts();
#endif


//this only works at the right time in editor. In builds we use a different method (reading the asset manually)
#if UNITY_EDITOR
            OpenVRSettings settings = OpenVRSettings.GetSettings();
            if (settings != null)
            {
                if (string.IsNullOrEmpty(settings.EditorAppKey))
                {
                    settings.EditorAppKey = settings.GenerateEditorAppKey();
                }

                UserDefinedSettings userDefinedSettings;
                userDefinedSettings.stereoRenderingMode = (ushort)settings.GetStereoRenderingMode();
                userDefinedSettings.initializationType = (ushort)settings.GetInitializationType();
                userDefinedSettings.applicationName = null;
                userDefinedSettings.editorAppKey = null;
                userDefinedSettings.mirrorViewMode = (ushort)settings.GetMirrorViewMode();

                userDefinedSettings.editorAppKey = settings.EditorAppKey; //only set the key if we're in the editor. Otherwise let steamvr set the key.

                if (OpenVRHelpers.IsUsingSteamVRInput())
                {
                    userDefinedSettings.editorAppKey = OpenVRHelpers.GetEditorAppKeyFromPlugin();
                }

                userDefinedSettings.applicationName = string.Format("[Testing] {0}", GetEscapedApplicationName());
                settings.InitializeActionManifestFileRelativeFilePath();

                userDefinedSettings.actionManifestPath = settings.ActionManifestFileRelativeFilePath;

                SetUserDefinedSettings(userDefinedSettings); 
            }
#endif
            
            CreateSubsystem<XRDisplaySubsystemDescriptor, XRDisplaySubsystem>(s_DisplaySubsystemDescriptors, "OpenVR Display");
            CreateSubsystem<XRInputSubsystemDescriptor, XRInputSubsystem>(s_InputSubsystemDescriptors, "OpenVR Input");

            OpenVREvents.Initialize();
            TickCallbackDelegate callback = TickCallback;
            RegisterTickCallback(callback);
            callback(0);

            return displaySubsystem != null && inputSubsystem != null;
        }

        private string GetEscapedApplicationName()
        {
            if (string.IsNullOrEmpty(Application.productName))
                return "";

            return Application.productName.Replace("\\", "\\\\").Replace("\"", "\\\""); //replace \ with \\ and replace " with \"  for json escaping
        }

        private void WatchForReload()
        {
#if UNITY_EDITOR
            UnityEditor.AssemblyReloadEvents.beforeAssemblyReload += DisableTickOnReload;
#endif
        }
        private void CleanupReloadWatcher()
        {
#if UNITY_EDITOR
            UnityEditor.AssemblyReloadEvents.beforeAssemblyReload -= DisableTickOnReload;
#endif
        }

        public override bool Start()
        {
            WatchForReload();

            StartSubsystem<XRDisplaySubsystem>();
            StartSubsystem<XRInputSubsystem>();

            EVRInitError result = GetInitializationResult();

            if (result != EVRInitError.None)
            {
                Debug.LogError("<b>[OpenVR]</b> Could not initialize OpenVR. Error code: " + result.ToString());
                return false;
            }

#if !UNITY_METRO
            try
            {
                settings = OpenVRSettings.GetSettings();

                // Listen for changes in the mirror mode file
                if (watcher == null)
                {
                    var fi = new System.IO.FileInfo(mirrorViewPath);
                    watcher = new System.IO.FileSystemWatcher(fi.DirectoryName, fi.Name);
                    watcher.NotifyFilter = System.IO.NotifyFilters.LastWrite;
                    watcher.Created += OnChanged;
                    watcher.Changed += OnChanged;
                    watcher.EnableRaisingEvents = true;
                    if (fi.Exists)
                        OnChanged(null, null);
                }
            } catch {}
#endif

            return true;
        }

        private System.IO.FileSystemWatcher watcher;
        private const string mirrorViewPath = "openvr_mirrorview.cfg";
        private OpenVRSettings settings;

        private void OnChanged(object source, System.IO.FileSystemEventArgs e)
        {
            ReadMirrorModeConfig();
        }

        /// This allows end users to switch mirror view modes at runtime with a file.
        /// To use place a file called openvr_mirrorview.cfg in the same directory as the executable (or root of project).
        /// The file should be one line with the following key/value:
        /// MirrorViewMode=openvr
        /// Acceptable values are left, right, none, and openvr. OpenVR mode is in beta but will show overlays and chaperone bounds.
        private void ReadMirrorModeConfig()
        {
            try
            {
                var lines = System.IO.File.ReadAllLines(mirrorViewPath);
                foreach (var line in lines)
                {
                    var split = line.Split('=');
                    if (split.Length == 2)
                    {
                        var key = split[0];
                        if (key == "MirrorViewMode")
                        {
                            string stringMode = split[1];
                            OpenVRSettings.MirrorViewModes mode = OpenVRSettings.MirrorViewModes.None;
                            if (stringMode.Equals("left", System.StringComparison.CurrentCultureIgnoreCase))
                                mode = OpenVRSettings.MirrorViewModes.Left;
                            else if (stringMode.Equals("right", System.StringComparison.CurrentCultureIgnoreCase))
                                mode = OpenVRSettings.MirrorViewModes.Right;
                            else if (stringMode.Equals("openvr", System.StringComparison.CurrentCultureIgnoreCase))
                                mode = OpenVRSettings.MirrorViewModes.OpenVR;
                            else if (stringMode.Equals("none", System.StringComparison.CurrentCultureIgnoreCase))
                                mode = OpenVRSettings.MirrorViewModes.None;
                            else
                            {
                                Debug.LogError("<b>[OpenVR]</b> Invalid mode specified in openvr_mirrorview.cfg. Options are: Left, Right, None, and OpenVR.");
                            }

                            Debug.Log("<b>[OpenVR]</b> Mirror View Mode changed via file to: " + mode.ToString());
                            OpenVRSettings.SetMirrorViewMode((ushort)mode); //bypass the local set.
                            
                        }
                    }
                }
            }
            catch 
            { }
        }

        private UnityEngine.Events.UnityEvent[] events;

        private void DestroyMirrorModeWatcher()
        {
            if (watcher != null)
            {
                watcher.Created -= OnChanged;
                watcher.Changed -= OnChanged;
                watcher.EnableRaisingEvents = false;
                watcher.Dispose();
                watcher = null;
            }
        }

        public override bool Stop()
        {
            CleanupTick();
            CleanupReloadWatcher();
            DestroyMirrorModeWatcher();            

            StopSubsystem<XRInputSubsystem>();
            StopSubsystem<XRDisplaySubsystem>(); //display actually does vrshutdown

            return true;
        }

        public override bool Deinitialize()
        {
            CleanupTick();
            CleanupReloadWatcher();
            DestroyMirrorModeWatcher();

            DestroySubsystem<XRInputSubsystem>();
            DestroySubsystem<XRDisplaySubsystem>();

            return true;
        }

        private static void CleanupTick()
        {
            RegisterTickCallback(null);
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto )]
        struct UserDefinedSettings
        {
            public ushort stereoRenderingMode;
            public ushort initializationType;
            public ushort mirrorViewMode;
            [MarshalAs(UnmanagedType.LPStr)] public string editorAppKey;
            [MarshalAs(UnmanagedType.LPStr)] public string actionManifestPath;
            [MarshalAs(UnmanagedType.LPStr)] public string applicationName;
        }

        [DllImport("XRSDKOpenVR", CharSet = CharSet.Auto)]
        private static extern void SetUserDefinedSettings(UserDefinedSettings settings);

        [DllImport("XRSDKOpenVR", CharSet = CharSet.Auto)]
        static extern EVRInitError GetInitializationResult();

        [DllImport("XRSDKOpenVR", CharSet = CharSet.Auto)]
        static extern void RegisterTickCallback([MarshalAs(UnmanagedType.FunctionPtr)] TickCallbackDelegate callbackPointer);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        delegate void TickCallbackDelegate(int value);

        [AOT.MonoPInvokeCallback(typeof(TickCallbackDelegate))]
        public static void TickCallback(int value)
        {
            OpenVREvents.Update();
        }

#if UNITY_EDITOR
        public string GetPreInitLibraryName(BuildTarget buildTarget, BuildTargetGroup buildTargetGroup)
        {
            return "XRSDKOpenVR";
        }

        private static void DisableTickOnReload()
        {
            CleanupTick();
        }
#endif
    }
}
#endif