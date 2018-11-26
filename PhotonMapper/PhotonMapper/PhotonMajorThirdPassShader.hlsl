#ifndef PHOTON_MAPPER_HLSL
#define PHOTON_MAPPER_HLSL

#define HLSL
#include "RaytracingHlslCompat.h"

// Render Target for visualizing the photons - can be removed later on
RWTexture2D<float4> RenderTarget : register(u0);
RWTexture2D<float4> StagedRenderTarget : register(u1);

// G-Buffers
RWTexture2DArray<float4> GPhotonPos : register(u2);
RWTexture2DArray<float4> GPhotonColor : register(u3);

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
};

[shader("raygeneration")]
void MyRaygenShader()
{
    uint2 index = uint2(DispatchRaysIndex().xy);

    float4 totalColor = StagedRenderTarget[index];
    float numColors = totalColor.w;

    if (numColors > 0.0f)
    {
        float3 avgColor = totalColor / numColors;
        RenderTarget[index] = float4(avgColor, 1.0);
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