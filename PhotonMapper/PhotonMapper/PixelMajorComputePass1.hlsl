#define blocksize 128

#define HLSL
#include "RaytracingHlslCompat.h"

// Render Target for visualizing the photons - can be removed later on
RWTexture2D<float4> RenderTarget : register(u0);

// G-Buffers
RWTexture2DArray<uint> GPhotonCount : register(u1);
RWTexture2DArray<float4> GPhotonPos : register(u2);
RWTexture2DArray<float4> GPhotonColor : register(u3);
RWTexture2DArray<float4> GPhotonNorm : register(u4);

/*
cbuffer globals : register(b0) {
	uint levelPowerOne;
	uint levelPower;
};
*/

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

// Exclusive Scan, Up-sweep
[numthreads(blocksize, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	int index = DTid.x; // TODO ???? Our threads are 1D
	//int divide = index / levelPowerOne;

	//if (index - (divide * levelPowerOne) == 0) {
	//	GPhotonCount[Cell1DTo3D(index + levelPowerOne - 1)] += GPhotonCount[Cell1DTo3D(index + levelPower - 1)]; // TODO change to scan buffer
	//}
}