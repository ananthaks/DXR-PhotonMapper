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

uint3 Cell1DToPhotonID(uint id)
{
    uint width, height, depth;
    GPhotonPos.GetDimensions(width, height, depth);
    uint3 temp;
    temp.x = id % width;
    temp.y = int(id / width) % height;
    temp.z = min(id / (width * height), depth - 1);
    return temp;
}


// Sort Pass
[numthreads(blocksize, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	float4 pos = GPhotonPos[DTid];
	float4 col = GPhotonColor[DTid];

	if (pos.w < 0.0f) {
		return;
	}

	uint cellIdX = floor(POS_TO_CELL_X(pos.x));
	uint cellIdY = floor(POS_TO_CELL_Y(pos.y));
	uint cellIdZ = floor(POS_TO_CELL_Z(pos.z));

	// Increment (with synchronization) the photon counter for that particular cell
	uint3 cellID = uint3(cellIdX, cellIdY, cellIdZ);

	// Place photon in new buffer
	uint newVal = 1;
	uint oldVal;
	InterlockedAdd(GPhotonTempIndex[cellID], newVal, oldVal);

	GPhotonSortedPos[Cell1DToPhotonID(oldVal)] = pos;
	GPhotonSortedCol[Cell1DToPhotonID(oldVal)] = col;
}

#endif // COMPUTE_PASS_3