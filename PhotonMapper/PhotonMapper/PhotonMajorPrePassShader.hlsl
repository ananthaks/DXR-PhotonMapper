#ifndef PHOTON_MAJOR_PRE_PASS
#define PHOTON_MAJOR_PRE_PASS

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
ByteAddressBuffer Indices : register(t1, space0);
StructuredBuffer<Vertex> Vertices : register(t2, space0);

// Constant Buffer views
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<CubeConstantBuffer> g_cubeCB : register(b1);

typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct RayPayload
{
    float4 color;
    float4 hitPosition;
    float4 extraInfo; // [0] is shadowHit, [1] is bounce depth (starts at 0 -> MaxBounces)
    float3 throughput;
    float3 direction;
};

[shader("raygeneration")]
void MyRaygenShader()
{
    uint2 index = DispatchRaysIndex().xy;
    
    StagedRenderTarget_R[index] = 0;
    StagedRenderTarget_G[index] = 0;
    StagedRenderTarget_B[index] = 0;
    StagedRenderTarget_A[index] = 0;
}


[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    
}

#endif // PHOTON_MAJOR_PRE_PASS