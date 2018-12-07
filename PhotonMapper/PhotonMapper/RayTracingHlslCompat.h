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

#define MAX_RAY_RECURSION_DEPTH 2

// Assumption : The scene bounds are considered as [-MAX_SCENE_SIZE_HALF, +MAX_SCENE_SIZE_HALF]
#define MAX_SCENE_SIZE_HALF 8.0f
#define MAX_SCENE_SIZE 2.0f * MAX_SCENE_SIZE_HALF

#define CELL_SIZE 4.0f

#define PIXEL_MAJOR_PHOTON_CLOSENESS 0.5f
#define PIXEL_MAJOR_SEARCH_RADIUS 1

#define NUM_CELLS_IN_X int(MAX_SCENE_SIZE / CELL_SIZE)
#define NUM_CELLS_IN_Y int(MAX_SCENE_SIZE / CELL_SIZE)
#define NUM_CELLS_IN_Z int(MAX_SCENE_SIZE / CELL_SIZE)

#define POS_TO_CELL_X(pos_x) NUM_CELLS_IN_X * (MAX_SCENE_SIZE_HALF + pos_x) / (MAX_SCENE_SIZE)
#define POS_TO_CELL_Y(pos_y) NUM_CELLS_IN_Y * (MAX_SCENE_SIZE_HALF - pos_y) / (MAX_SCENE_SIZE)
#define POS_TO_CELL_Z(pos_z) NUM_CELLS_IN_Z * (MAX_SCENE_SIZE_HALF + pos_z) / (MAX_SCENE_SIZE)

// This Assumes the implementation is in row-major
// i.e. the cells are placed row-wise first, the column and then depth
#define CELL_3D_TO_1D(cellx, celly, cellz) (cellx + celly * NUM_CELLS_IN_X + cellz * NUM_CELLS_IN_X * NUM_CELLS_IN_Y)

#define CELL_1D_TO_3D_X(cellId) (cellId % NUM_CELLS_IN_X)
#define CELL_1D_TO_3D_Y(cellId) ((cellId / NUM_CELLS_IN_X) % NUM_CELLS_IN_Y)
#define CELL_1D_TO_3D_Z(cellId) (cellId / (NUM_CELLS_IN_X * NUM_CELLS_IN_Y))

// Photon Major
#define NUM_SAMPLES 5
#define SEARCH_RADIUS 0.01f

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

// From Stream Compaction hw
inline int ilog2(int x) {
	int lg = 0;
	while (x >>= 1) {
		++lg;
	}
	return lg;
}

inline int ilog2ceil(int x) {
	return x == 1 ? 0 : ilog2(x - 1) + 1;
}

#endif // RAYTRACINGHLSLCOMPAT_H
