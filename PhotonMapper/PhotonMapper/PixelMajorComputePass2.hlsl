#ifndef COMPUTE_PASS_2
#define COMPUTE_PASS_2

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

// Downsweep
[numthreads(blocksize, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	int index = DTid.x; // TODO ???? Our threads are 1D
	
	const int two_d = CKernelParams.param1;
	const int two_d_1 = two_d * 2;

	if (index % two_d_1 != 0)
	{
		return;
	}

	const int oldIndex = index + two_d - 1;
	const int newIndex = index + two_d_1 - 1;

	const int dataAtNewIndex = GPhotonScan[Cell1DTo3D(newIndex)];

	const int t = GPhotonScan[Cell1DTo3D(oldIndex)];

	GPhotonScan[Cell1DTo3D(oldIndex)] = dataAtNewIndex;
	GPhotonScan[Cell1DTo3D(newIndex)] = t + GPhotonScan[Cell1DTo3D(newIndex)];
}

#endif // COMPUTE_PASS_2