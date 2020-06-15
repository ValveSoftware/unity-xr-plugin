//======= Copyright (c) Valve Corporation, All rights reserved. ===============
//
// Purpose: Notify developers when a new version of the plugin is available.
//
//=============================================================================

using UnityEngine;
using UnityEditor;
using System.IO;
using System.Text.RegularExpressions;

namespace Unity.XR.OpenVR
{
    public class OpenVRUpdaterDialog : EditorWindow
    {
        public static OpenVRUpdaterDialog window;

        private static string currentVersion;
        private static string newVersion;
        private static string notes;

        public static void Display(string currentVersionString, string newVersionString, string notesString)
        {
            currentVersion = currentVersionString;
            newVersion = newVersionString;
            notes = notesString;

            window = GetWindow<OpenVRUpdaterDialog>(true);
            window.minSize = new Vector2(320, 440);
        }

        public static void CloseWindow()
        {
            if (window != null)
                window.Close();
        }

        Vector2 scrollPosition;
        bool doNotPromptState;

        string GetResourcePath()
        {
            var ms = MonoScript.FromScriptableObject(this);
            var path = AssetDatabase.GetAssetPath(ms);
            path = Path.GetDirectoryName(path);
            return path;
        }

        public void OnGUI()
        {
            EditorGUILayout.HelpBox("A new version of the OpenVR Unity XR plugin is available!", MessageType.Warning);

            var resourcePath = GetResourcePath();
            var logo = AssetDatabase.LoadAssetAtPath<Texture2D>(resourcePath + "openvr.jpg");
            var rect = GUILayoutUtility.GetRect(position.width, 150, GUI.skin.box);
            if (logo)
                GUI.DrawTexture(rect, logo, ScaleMode.ScaleToFit);


            GUILayout.Label("Current version: " + currentVersion);
            GUILayout.Label("New version: " + newVersion);

            scrollPosition = GUILayout.BeginScrollView(scrollPosition);

            if (notes != "")
            {
                GUILayout.Label("Release notes:");
                EditorGUILayout.HelpBox(notes, MessageType.Info);
            }

            GUILayout.EndScrollView();

            GUILayout.FlexibleSpace();

            EditorGUI.BeginChangeCheck();

            var doNotPrompt = GUILayout.Toggle(doNotPromptState, "Do not prompt for this version again.");
            if (EditorGUI.EndChangeCheck())
            {
                doNotPromptState = doNotPrompt;

                OpenVRUpdater.DoNotPromptForVersion();
            }

            if (GUILayout.Button("Get Latest Version"))
            {
                OpenVRUpdater.UpdateVersion();
            }
        }
    }
}