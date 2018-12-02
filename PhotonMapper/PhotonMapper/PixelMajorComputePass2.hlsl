#define blocksize 128

// Render Target for visualizing the photons - can be removed later on
RWTexture2D<float4> RenderTarget : register(u0);

// G-Buffers
RWTexture2DArray<float4> GPhotonPos : register(u1);
RWTexture2DArray<float4> GPhotonColor : register(u2);
RWTexture2DArray<float4> GPhotonNorm : register(u3);

[numthreads(blocksize, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	// Each thread of the CS updates one of the particles.
	//float4 pos = GPhotonPos[DTid];
	RenderTarget[DTid.xy] = float4(1.0, 0.0, 0.0, 1.0);
}