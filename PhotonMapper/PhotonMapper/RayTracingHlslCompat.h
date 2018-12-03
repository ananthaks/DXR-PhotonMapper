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

#ifndef RAYTRACINGHLSLCOMPAT_H
#define RAYTRACINGHLSLCOMPAT_H

#ifdef HLSL
#include "HlslCompat.h"
#else
using namespace DirectX;

// Shader will use byte encoding to access indices.
typedef UINT16 Index;
#endif

#define MAX_RAY_RECURSION_DEPTH 5

#define MAX_SCENE_SIZE 16
#define MAX_SCENE_SIZE3 MAX_SCENE_SIZE * MAX_SCENE_SIZE * MAX_SCENE_SIZE

#define CELL_SIZE 0.25

#define NUM_CELLS_IN_X MAX_SCENE_SIZE / CELL_SIZE
#define NUM_CELLS_IN_Y MAX_SCENE_SIZE / CELL_SIZE
#define NUM_CELLS_IN_Z MAX_SCENE_SIZE / CELL_SIZE

#define POS_TO_CELL_X(pos_x) (pos_x + MAX_SCENE_SIZE) / 2
#define POS_TO_CELL_Y(pos_y) (pos_y + MAX_SCENE_SIZE) / 2
#define POS_TO_CELL_Z(pos_z) (pos_z + MAX_SCENE_SIZE) / 2

// This Assumes the implementation is in row-major
// i.e. the cells are placed row-wise first, the column and then depth
#define CELL_3D_TO_1D(cellx, celly, cellz) (cellx + celly * NUM_CELLS_IN_X + cellz * NUM_CELLS_IN_X * NUM_CELLS_IN_Y)

#define CELL_1D_TO_3D_X(cellId) (cellId % NUM_CELLS_IN_X)
#define CELL_1D_TO_3D_Y(cellId) ((cellId / NUM_CELLS_IN_X) % NUM_CELLS_IN_Y)
#define CELL_1D_TO_3D_Z(cellId) (cellId / (NUM_CELLS_IN_X * NUM_CELLS_IN_Y))


struct SceneConstantBuffer
{
    XMMATRIX projectionToWorld;
    XMMATRIX viewProj;
    XMVECTOR cameraPosition;
    XMVECTOR lightPosition;
    XMVECTOR lightAmbientColor;
    XMVECTOR lightDiffuseColor;
};

struct PixelMajorComputeConstantBuffer
{
	UINT param1;
	UINT param2;
};

struct CubeConstantBuffer
{
    XMFLOAT4 albedo;
};

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 color;
    XMFLOAT3 normal;
};



#endif // RAYTRACINGHLSLCOMPAT_H
