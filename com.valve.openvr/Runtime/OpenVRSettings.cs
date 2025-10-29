﻿using System;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;

#if UNITY_XR_MANAGEMENT
using UnityEngine.XR.Management;
#endif

namespace Unity.XR.OpenVR
{
#if UNITY_XR_MANAGEMENT
    [XRConfigurationData("OpenVR", "Unity.XR.OpenVR.Settings")]
#endif
    [System.Serializable]
    public class OpenVRSettings : ScriptableObject
    {
        // Convert backslashes in relative paths to forward-slashes for cross-platform compatibility
        // Windows Volume-prefixed paths and POSIX absolute paths have semantics preserved.
        // Only call on output paths constructed with System.IO.Path.* on the current platform;
        // For paths coming from other platforms, forward-slash must be valid or decoded;
        // This is not necessary here, as forward-slash is valid in all Unity targets.
        private static string SlashifyPath(string path) {
            var sep = System.IO.Path.DirectorySeparatorChar;
            var sepAlt = System.IO.Path.AltDirectorySeparatorChar;
            // Only convert when both back slashes and forward slashes are semantically valid in the
            // context of the current platform's Path handling.
            // Avoids semantic alteration of questionable paths on POSIX systems where backslash is
            // usually used as an escape character, and are not generally included within the path.
            if ((sep == '\\' || sepAlt == '\\') && (sep == '/' || sepAlt == '/'))
            {
                return path.Replace('\\', '/');
            }
            return path;
        }
    
        public enum StereoRenderingModes
        {
            MultiPass = 0,
            SinglePassInstanced
        }

        public enum InitializationTypes
        {
            Scene = 1,
            Overlay = 2,
        }

        public enum MirrorViewModes
        {
            None = 0,
            Left,
            Right,
            OpenVR,
        }

        [SerializeField, Tooltip("This will check the package version and the latest on github and prompt you to upgrade once per project load.")]
        public bool PromptToUpgradePackage = true;

        [SerializeField, Tooltip("This will check the package version and the latest on github and prompt you to upgrade once per project load.")]
        public bool PromptToUpgradePreviewPackages = true;

        [SerializeField, Tooltip("This allows developers to skip upgrade prompts for just this version.")]
        public string SkipPromptForVersion = null;

        [SerializeField, Tooltip("Set the Stereo Rendering Method")]
        public StereoRenderingModes StereoRenderingMode = StereoRenderingModes.SinglePassInstanced;

        [SerializeField, Tooltip("Most applications initialize as type Scene")]
        public InitializationTypes InitializationType = InitializationTypes.Scene;

        [SerializeField, Tooltip("A generated unique identifier for your application while in the editor")]
        public string EditorAppKey = null;

        [SerializeField, Tooltip("Internal value that tells the system what the relative path is to the manifest")]
        public string ActionManifestFileRelativeFilePath;

        [SerializeField, Tooltip("Which eye to use when rendering the headset view to the main window (none, left, right, or a composite of both + OpenVR overlays)")]
        public MirrorViewModes MirrorView = MirrorViewModes.Right;

        public const string StreamingAssetsFolderName = "SteamVR";
        public const string ActionManifestFileName = "legacy_manifest.json";
        public static string GetStreamingSteamVRPath(bool create = true)
        {
            string path = System.IO.Path.Combine(Application.streamingAssetsPath, StreamingAssetsFolderName);

            if (create)
            {
                CreateDirectory(new DirectoryInfo(path));
            }

            return path;
        }

        private static void CreateDirectory(DirectoryInfo directory)
        {
            if (directory.Parent.Exists == false)
                CreateDirectory(directory.Parent);

            if (directory.Exists == false)
                directory.Create();
        }

        [SerializeField, Tooltip("Internal value that tells the system if we have copied the default binding files yet.")]
        public bool HasCopiedDefaults = false;

        public ushort GetStereoRenderingMode()
        {
            return (ushort)StereoRenderingMode;
        }

        public ushort GetInitializationType()
        {
            return (ushort)InitializationType;
        }

        public MirrorViewModes GetMirrorViewMode()
        {
            return MirrorView;
        }

        /// <summary>
        /// Sets the mirror view mode (left, right, composite of both + openvr overlays) at runtime.
        /// </summary>
        /// <param name="newMode">left, right, composite of both + openvr overlays</param>
        public void SetMirrorViewMode(MirrorViewModes newMode)
        {
            MirrorView = newMode;
            SetMirrorViewMode((ushort)newMode);
        }

        public string GenerateEditorAppKey()
        {
            return string.Format("application.generated.unity.{0}.{1}.exe", CleanProductName(), ((int)(UnityEngine.Random.value * int.MaxValue)).ToString());
        }

        private static string CleanProductName()
        {
            string productName = Application.productName;
            if (string.IsNullOrEmpty(productName))
                productName = "unnamed_product";
            else
            {
                productName = System.Text.RegularExpressions.Regex.Replace(productName, "[^\\w\\._]", "");
                productName = productName.ToLower();
                productName = string.Concat(productName.Normalize(NormalizationForm.FormD).Where(
                    c => CharUnicodeInfo.GetUnicodeCategory(c) != UnicodeCategory.NonSpacingMark));
                byte[] bytes = Encoding.ASCII.GetBytes(productName);
                char[] chars = Encoding.ASCII.GetChars(bytes);
                productName = new String(chars).Replace("?", "");
                if (productName.Length == 0)
                {
                    productName = Mathf.Abs(Application.productName.GetHashCode()).ToString();
                }
            }

            return productName;
        }

        public static OpenVRSettings GetSettings(bool create = true)
        {
            OpenVRSettings settings = null;
#if UNITY_EDITOR
            UnityEditor.EditorBuildSettings.TryGetConfigObject<OpenVRSettings>("Unity.XR.OpenVR.Settings", out settings);
#else
            settings = OpenVRSettings.s_Settings;
#endif

            if (settings == null && create)
                settings = OpenVRSettings.CreateInstance<OpenVRSettings>();

            return settings;
        }

        [DllImport("XRSDKOpenVR", CharSet = CharSet.Auto)]
        public static extern void SetMirrorViewMode(ushort mirrorViewMode);


        public bool InitializeActionManifestFileRelativeFilePath()
        {
#if UNITY_EDITOR
            string oldPath = ActionManifestFileRelativeFilePath;
            string newPath;

            if (OpenVRHelpers.IsUsingSteamVRInput())
            {
                newPath = System.IO.Path.Combine(OpenVRSettings.GetStreamingSteamVRPath(false), OpenVRHelpers.GetActionManifestNameFromPlugin());
                
                string fullpath = System.IO.Path.GetFullPath(".");
                newPath = newPath.Remove(0, fullpath.Length + 1);

                if (newPath.StartsWith("Assets"))
                    newPath = newPath.Remove(0, "Assets".Length + 1);

                newPath = SlashifyPath(newPath);
            }
            else
            {
                newPath = null;
            }

            if (newPath != oldPath)
            {
                ActionManifestFileRelativeFilePath = newPath;
                UnityEditor.EditorUtility.SetDirty(this);
                UnityEditor.AssetDatabase.SaveAssets();
                return true;
            }
#endif
            return false;
        }

#if UNITY_EDITOR
        public void Awake()
        {
            if (string.IsNullOrEmpty(this.EditorAppKey))
            {
                this.EditorAppKey = this.GenerateEditorAppKey();
            }

            this.InitializeActionManifestFileRelativeFilePath();
        }
#else
        public static OpenVRSettings s_Settings;

        public void Awake()
        {
            s_Settings = this;
        }
#endif
    }
}
