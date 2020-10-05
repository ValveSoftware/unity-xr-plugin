// Copyright (c) 2020, Valve Software
//
// SPDX-License-Identifier: BSD-3-Clause

﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using Unity.XR.OpenVR;

namespace Unity.XR.OpenVR.Editor
{
    [CustomEditor(typeof(OpenVRSettings))]
    public class OpenVRSettingsEditor : UnityEditor.Editor
    {
        private const string kStereoRenderingMode = "StereoRenderingMode";

        static GUIContent s_StereoRenderingMode = EditorGUIUtility.TrTextContent("Stereo Rendering Mode");

        private SerializedProperty m_StereoRenderingMode;

        private const string kInitializationType = "InitializationType";

        static GUIContent s_InitializationType = EditorGUIUtility.TrTextContent("Application Type");

        private SerializedProperty m_InitializationType;

        private const string kMirrorViewModeKey = "MirrorView";

        static GUIContent s_MirrorViewMode = EditorGUIUtility.TrTextContent("Mirror View Mode");

        private SerializedProperty m_MirrorViewMode;

        private const string kPromptToUpgradePackageKey = "PromptToUpgradePackage";

        static GUIContent s_PromptToUpgradePackage = EditorGUIUtility.TrTextContent("Prompt To Upgrade Package");

        private SerializedProperty m_PromptToUpgradePackage;

        public GUIContent WindowsTab;
        private int tab = 0;

        public void OnEnable()
        { 
            WindowsTab = new GUIContent("",  EditorGUIUtility.IconContent("BuildSettings.Standalone.Small").image);
        }

        public override void OnInspectorGUI()
        {
            if (serializedObject == null || serializedObject.targetObject == null)
                return;

            if (m_StereoRenderingMode == null)
            {
                m_StereoRenderingMode = serializedObject.FindProperty(kStereoRenderingMode);
            }
            if (m_InitializationType == null)
            {
                m_InitializationType = serializedObject.FindProperty(kInitializationType);
            }
            if (m_MirrorViewMode == null)
            {
                m_MirrorViewMode = serializedObject.FindProperty(kMirrorViewModeKey);
            }
            if (m_PromptToUpgradePackage == null)
            {
                m_PromptToUpgradePackage = serializedObject.FindProperty(kPromptToUpgradePackageKey);
            }

            serializedObject.Update();

            int currentMode = m_MirrorViewMode.intValue;
            if (m_MirrorViewMode != null)

            tab = GUILayout.Toolbar(tab, new GUIContent[] {WindowsTab},EditorStyles.toolbarButton);
            EditorGUILayout.Space();

            EditorGUILayout.BeginVertical(GUILayout.ExpandWidth(true));
            if (tab == 0)
            {
                EditorGUILayout.PropertyField(m_InitializationType, s_InitializationType);
                EditorGUILayout.PropertyField(m_StereoRenderingMode, s_StereoRenderingMode);
                EditorGUILayout.PropertyField(m_MirrorViewMode, s_MirrorViewMode);
                EditorGUILayout.Space();
                EditorGUILayout.PropertyField(m_PromptToUpgradePackage, s_PromptToUpgradePackage);
            }
            EditorGUILayout.EndVertical();

            serializedObject.ApplyModifiedProperties();

            int newMode = m_MirrorViewMode.intValue;

            if (currentMode != newMode && Application.isPlaying)
            {
                OpenVRSettings.SetMirrorViewMode((ushort)newMode);
            }
        }
    }
}
