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
#include "D3D12RaytracingSimpleLighting.h"
#include "PhotonMapperRenderer.h"
#include "PhotonMajorRenderer.h"

//#define USE_PHOTON_MAJOR_RENDERER
#define USE_PIXEL_MAJOR_RENDERER

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
#ifdef USE_PIXEL_MAJOR_RENDERER
    PhotonMapperRenderer sample(1280, 720, L"DXR Pixel Major Renderer");
    return Win32Application::Run(&sample, hInstance, nCmdShow);
#endif

#ifdef USE_PHOTON_MAJOR_RENDERER
    PhotonMajorRenderer sample(1280, 720, L"DXR Photon Major Renderer");
    return Win32Application::Run(&sample, hInstance, nCmdShow);
#endif

}
