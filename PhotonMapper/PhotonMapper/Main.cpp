//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "PhotonMapperRenderer.h"
#include "PhotonMajorRenderer.h"
#include "PMScene.h"

#define USE_PHOTON_MAJOR_RENDERER
//#define USE_PIXEL_MAJOR_RENDERER

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    

#ifdef USE_PIXEL_MAJOR_RENDERER
    PhotonMapperRenderer sample(SCREEN_WIDTH, SCREEN_HEIGHT, L"DXR Pixel Major Renderer");
    return Win32Application::Run(&sample, hInstance, nCmdShow);
#endif


#ifdef USE_PHOTON_MAJOR_RENDERER

    OutputDebugString(L"DXRPhotonMapper - Loading Scene File\n");
    const std::string filePath = "E:/Git/DXR-PhotonMapper/Scene/sample.json";

    DXRPhotonMapper::PMScene scene(SCREEN_WIDTH, SCREEN_HEIGHT);
    scene.LoadJSONScene(filePath);

    PhotonMajorRenderer sample(scene, SCREEN_WIDTH, SCREEN_HEIGHT, L"DXR Photon Major Renderer");
    return Win32Application::Run(&sample, hInstance, nCmdShow);
#endif

}
