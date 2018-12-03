#ifndef COMPUTE_PASS_3
#define COMPUTE_PASS_3

#define HLSL
#include "RaytracingHlslCompat.h"

#define blocksize 128

// Render Target for visualizing the photons - can be removed later on
RWTexture2D<float4> RenderTarget : register(u0); 

// G-Buffers
RWTexture2DArray<uint> GPhotonCount : register(u1);
RWTexture2DArray<uint> GPhotonScan : register(u2);
RWTexture2DArray<uint> GPhotonTempIndex : register(u3);

RWTexture2DArray<float4> GPhotonPos : register(u4);
RWTexture2DArray<float4> GPhotonColor : register(u5);
RWTexture2DArray<float4> GPhotonSortedPos : register(u6);
RWTexture2DArray<float4> GPhotonSortedCol : register(u7);

ConstantBuffer<PixelMajorComputeConstantBuffer> CKernelParams : register(b0);

uint3 PosToCellId(float3 worldPosition)
{
	float3 correctWorldPos = (worldPosition + float3(MAX_SCENE_SIZE, MAX_SCENE_SIZE, MAX_SCENE_SIZE)) / 2.0;
	return uint3(floor(correctWorldPos / CELL_SIZE));
}

uint Cell3DTo1D(uint3 cellId)
{
	return uint(cellId.x + MAX_SCENE_SIZE * cellId.y + MAX_SCENE_SIZE * MAX_SCENE_SIZE * cellId.z); // TODO check if correct
}

uint3 Cell1DTo3D(uint id)
{
	uint3 temp;
	temp.x = id % MAX_SCENE_SIZE;
	temp.y = (id / MAX_SCENE_SIZE) % MAX_SCENE_SIZE;
	temp.z = id / (MAX_SCENE_SIZE * MAX_SCENE_SIZE);
	return temp;
}

uint3 Cell1DToPhotonID(uint id)
{
	uint3 temp;
	temp.x = id % CKernelParams.param1;
	temp.y = (id / CKernelParams.param1) % CKernelParams.param1;
	temp.z = min(id / (CKernelParams.param1 * CKernelParams.param1), CKernelParams.param2 - 1);
	return temp;
}

// Sort Pass
[numthreads(blocksize, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	float4 pos = GPhotonPos[DTid];
	float4 col = GPhotonColor[DTid];

	uint3 cellID = PosToCellId(pos.xyz);

	// Place photon in new buffer
	uint newVal = 1;
	uint oldVal;
	InterlockedAdd(GPhotonTempIndex[cellID], newVal, oldVal);

	GPhotonSortedPos[Cell1DToPhotonID(oldVal)] = pos;
	GPhotonSortedCol[Cell1DToPhotonID(oldVal)] = col;
}

#endif // COMPUTE_PASS_3