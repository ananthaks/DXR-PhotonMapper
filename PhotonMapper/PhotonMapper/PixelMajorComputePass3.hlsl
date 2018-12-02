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
RWTexture2DArray<float4> GPhotonNorm : register(u6);

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

[numthreads(blocksize, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	int index = DTid.x; // TODO ???? Our threads are 1D

	int divide = index / CKernelParams.param1;
	if (index - (divide * CKernelParams.param1) == 0) {
		int temp = GPhotonScan[Cell1DTo3D(index + CKernelParams.param2 - 1)];
		GPhotonScan[Cell1DTo3D(index + CKernelParams.param2 - 1)] = GPhotonScan[Cell1DTo3D(index + CKernelParams.param1 - 1)];
		GPhotonScan[Cell1DTo3D(index + CKernelParams.param1 - 1)] += temp;
	}

}

#endif // COMPUTE_PASS_3