using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using Unity.XR.OpenVR;
using UnityEditor.PackageManager.Requests;
using System.Data;
using System.Linq;
using System;
using UnityEngine.Networking;
using System.IO;

namespace Unity.XR.OpenVR
{
    [InitializeOnLoad]
    public class OpenVRUpdater
    {
        public const string valveOpenVRPackageString = "com.valve.openvr";
        public const string valveOpenVRGitURL = "https://github.com/ValveSoftware/steamvr_unity_plugin.git#UnityXRPlugin";
        public const string packageJSONurl = "https://raw.githubusercontent.com/ValveSoftware/steamvr_unity_plugin/UnityXRPlugin/package.json";
        public const string packageCHANGELOGurl = "https://raw.githubusercontent.com/ValveSoftware/steamvr_unity_plugin/UnityXRPlugin/CHANGELOG.md";
        private const string defaultOpenVRSettingsPath = "Assets/XR/Settings/Open VR Settings.asset";

        private static ListRequest listRequest;
        private static UpdateStates updateState;
        private static RemoveRequest removeRequest;

        private static UnityWebRequest webRequest;
        private static UnityWebRequestAsyncOperation webRequestOperation;

        private static string currentPackageVersion;
        private static string newPackageVersion;

        private static System.Diagnostics.Stopwatch addingPackageTime = new System.Diagnostics.Stopwatch();
        private const float estimatedTimeToInstall = 90; // in seconds

        static OpenVRUpdater()
        {
            OpenVRSettings settings = OpenVRSettings.GetSettings();

            if (settings.PromptToUpgradePackage)
            {
                EditorApplication.update -= Update;
                EditorApplication.update += Update;
                listRequest = UnityEditor.PackageManager.Client.List(true);
                updateState = UpdateStates.Listing;
            }
        }

        private static void Update()
        {
            switch (updateState)
            {
                case UpdateStates.Listing:
                    if (listRequest != null && listRequest.IsCompleted)
                    {
                        if (listRequest.Error != null || listRequest.Status == UnityEditor.PackageManager.StatusCode.Failure)
                        {
                            updateState = UpdateStates.Failed;
                            Debug.LogError("[OpenVR Plugin] Error checking for updated package during package listing.");
                            Stop();
                            break;
                        }

                        var package = listRequest.Result.FirstOrDefault(upackage => upackage.name == valveOpenVRPackageString);
                        currentPackageVersion = package.version;

                        webRequest = UnityWebRequest.Get(packageJSONurl);
                        webRequestOperation = webRequest.SendWebRequest();
                        updateState = UpdateStates.VersionWebRequest;
                    }
                    break;
                case UpdateStates.VersionWebRequest:
                    if (webRequestOperation != null && webRequestOperation.isDone)
                    {
                        if (string.IsNullOrEmpty(webRequest.error))
                        {
                            OpenVRSettings settings = OpenVRSettings.GetSettings();

                            string newVersion = GetVersionFromPackageJSON(webRequest.downloadHandler.text);
                            if (newVersion != settings.SkipPromptForVersion)
                            {
                                if (CompareVersions(currentPackageVersion, newVersion))
                                {
                                    webRequest = UnityWebRequest.Get(packageCHANGELOGurl);
                                    webRequestOperation = webRequest.SendWebRequest();
                                    updateState = UpdateStates.ChangelogWebRequest;
                                    newPackageVersion = newVersion;
                                }
                                else
                                {
                                    Stop();
                                }
                            }
                            else
                            {
                                Stop();
                            }
                        }
                        else
                        {
                            Stop();
                        }
                    }
                    break;
                case UpdateStates.ChangelogWebRequest:
                    if (webRequestOperation != null && webRequestOperation.isDone)
                    {
                        string notes = webRequest.downloadHandler.text;
                        notes = notes.Substring(251); //magic number of characters to take out the cruft at the top
                        OpenVRUpdaterDialog.Display(currentPackageVersion, newPackageVersion, notes);

                        updateState = UpdateStates.Idle;
                        Stop();
                    }
                    break;
            }
        }

        private const string autoUpdaterFilename = "OpenVRAutoUpdater";
        public static void UpdateVersion()
        {
            var script = MonoScript.FromScriptableObject(OpenVRUpdaterDialog.window);
            var path = AssetDatabase.GetAssetPath(script);
            FileInfo windowScript = new FileInfo(path);
            FileInfo autoUpdaterPath = new FileInfo(Path.Combine(windowScript.Directory.FullName, autoUpdaterFilename + ".auto"));
            FileInfo newAutoUpdaterPath = new FileInfo(Path.Combine(Application.dataPath, "Editor/" + autoUpdaterFilename + ".cs"));
            if (newAutoUpdaterPath.Directory.Exists == false)
                newAutoUpdaterPath.Directory.Create();
            File.Copy(autoUpdaterPath.FullName, newAutoUpdaterPath.FullName);
            OpenVRUpdaterDialog.CloseWindow();
            AssetDatabase.Refresh();
        }

        public static void DoNotPromptForVersion()
        {
            OpenVRSettings settings = OpenVRSettings.GetSettings(false);
            if (settings == null)
            {
                settings = OpenVRSettings.GetSettings(true);
                AssetDatabase.CreateAsset(settings, defaultOpenVRSettingsPath);
            }

            settings.SkipPromptForVersion = newPackageVersion;
            EditorUtility.SetDirty(settings);
            AssetDatabase.SaveAssets();
        }

        private static string GetVersionFromPackageJSON(string jsonText)
        {
            string findString = "\"version\": \"";
            int versionIndex = jsonText.IndexOf(findString);

            if (versionIndex != -1)
            {
                versionIndex += findString.Length;
                int endVersionIndex = jsonText.IndexOf("\"", versionIndex);
                if (endVersionIndex != -1)
                {
                    int length = endVersionIndex - versionIndex;
                    return jsonText.Substring(versionIndex, length);
                }
            }
            return null;
        }

        /// <summary>
        /// Returns true if the testing version is above the current version
        /// </summary>
        /// 
        // 1.0.0 > 1.0.0-preview 
        // 1.0.0-preview.2 > 1.0.0-preview.1
        private static bool CompareVersions(string current, string testing)
        {
            string[] currentSplit = current.Split(new string[] { "-preview" }, StringSplitOptions.None);
            string[] testingSplit = testing.Split(new string[] { "-preview" }, StringSplitOptions.None);

            Version currentVersion = Version.Parse(currentSplit[0]);
            Version testingVersion = Version.Parse(testingSplit[0]);

            if (currentVersion == testingVersion)
            {
                if (currentSplit.Length > 1 && testingSplit.Length > 1)
                {
                    currentSplit[1] = "0" + currentSplit[1];
                    testingSplit[1] = "0" + testingSplit[1];
                    currentVersion = Version.Parse(currentSplit[1]);
                    testingVersion = Version.Parse(testingSplit[1]);
                }
                else if (currentSplit.Length > 1 && testingSplit.Length == 1)//current is preview, testing is not
                    return true;
                else if (testingSplit.Length > 1 && currentSplit.Length == 1)//testing is preview, current is not
                    return false;
            }
            return testingVersion > currentVersion;
        }

        private static void Stop()
        {
            EditorUtility.ClearProgressBar();
            EditorApplication.update -= Update;
        }

        private enum UpdateStates
        {
            Idle,
            Failed,
            Succeeded,
            Listing,
            VersionWebRequest,
            ChangelogWebRequest,
            WaitingForRemove,
            WaitingForAdd,
        }
    }
}