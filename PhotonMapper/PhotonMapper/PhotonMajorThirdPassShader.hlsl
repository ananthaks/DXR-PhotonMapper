#ifndef PHOTON_MAPPER_HLSL
#define PHOTON_MAPPER_HLSL

#define HLSL
#include "RaytracingHlslCompat.h"

// Render Target for visualizing the photons - can be removed later on
RWTexture2D<float4> RenderTarget : register(u0);

RWTexture2D<uint> StagedRenderTarget_R : register(u1);
RWTexture2D<uint> StagedRenderTarget_G : register(u2);
RWTexture2D<uint> StagedRenderTarget_B : register(u3);
RWTexture2D<uint> StagedRenderTarget_A : register(u4);

// G-Buffers
RWTexture2DArray<float4> GPhotonPos : register(u5);
RWTexture2DArray<float4> GPhotonColor : register(u6);
RWTexture2DArray<float4> GPhotonNorm : register(u7);
RWTexture2DArray<float4> GPhotonTangent : register(u8);

RaytracingAccelerationStructure Scene : register(t0, space0);
ByteAddressBuffer Indices[] : register(t0, space1);
StructuredBuffer<Vertex> Vertices[] : register(t0, space2);

// Constant Buffer views
//ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
//ConstantBuffer<CubeConstantBuffer> g_cubeCB : register(b1);


ConstantBuffer<SceneBufferDesc> g_geomIndex[] : register(b0, space3);
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<CubeConstantBuffer> g_cubeCB : register(b1);


typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct RayPayload
{
    float4 color;
};

[shader("raygeneration")]
void MyRaygenShader()
{
    uint2 index = uint2(DispatchRaysIndex().xy);

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


[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{

}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{

}

#endif // PHOTON_MAPPER_HLSL