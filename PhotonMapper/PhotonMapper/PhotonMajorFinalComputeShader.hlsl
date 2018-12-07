#ifndef COMPUTE_PASS_0
#define COMPUTE_PASS_0

#define HLSL
#include "RaytracingHlslCompat.h"

#define blocksize 128

// Render Target for visualizing the photons - can be removed later on
RWTexture2D<float4> RenderTarget : register(u0);

RWTexture2D<uint> StagedRenderTarget_R : register(u1);
RWTexture2D<uint> StagedRenderTarget_G : register(u2);
RWTexture2D<uint> StagedRenderTarget_B : register(u3);
RWTexture2D<uint> StagedRenderTarget_A : register(u4);

[numthreads(blocksize, 1, 1)]
void CSMain(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
    uint2 index = uint2(DTid.xy);

    float channel_r = StagedRenderTarget_R[index];
    float channel_g = StagedRenderTarget_G[index];
    float channel_b = StagedRenderTarget_B[index];
    uint channel_a = StagedRenderTarget_A[index];

    if (channel_a > 0)
    {
        float3 avgColor = float3(channel_r, channel_g, channel_b) / (channel_a * 255.f);
        RenderTarget[index] = float4(avgColor, 1.0);
    }
    else
    {
        RenderTarget[index] = float4(0.0, 0.0, 0.0, 1.0);
    }
}

#endif // COMPUTE_PASS_0
