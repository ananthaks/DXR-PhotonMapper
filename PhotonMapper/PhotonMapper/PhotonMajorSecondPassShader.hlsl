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

// Constant buffers
ConstantBuffer<SceneBufferDesc> c_bufferIndices[] : register(b0, space1);
ConstantBuffer<MaterialDesc> c_materials[] : register(b0, space2);
ConstantBuffer<LightDesc> c_lights[] : register(b0, space3);

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<CubeConstantBuffer> g_cubeCB : register(b1);


typedef BuiltInTriangleIntersectionAttributes MyAttributes;

struct RayPayload
{
    float4 extraInfo; // [0] is shadowHit, [1] is bounce depth
};


inline void GetPixelPosition(float3 rayHitPosition, float2 screenDim, out uint2 pixelIndex, out bool inRange)
{
    float4 clippingCoord = mul(float4(rayHitPosition, 1), g_sceneCB.viewProj);
    clippingCoord.xyz /= clippingCoord.w;

    if (clippingCoord.z < 0.01f || clippingCoord.z > 1 || clippingCoord.x < -1 || clippingCoord.x > 1 || clippingCoord.y < -1 || clippingCoord.y > 1) {
        inRange = false;
        return;
    }

    inRange = true;
    pixelIndex.x = ((clippingCoord.x + 1.0) * 0.5f) * screenDim.x;
    pixelIndex.y = ((1.0 - clippingCoord.y) * 0.5f) * screenDim.y;
}


void TraceRayToCamera(float4 hitPosition, float2 screenDims, out bool didHit, out uint2 pixelHit)
{
    // Find the Screen Space Coord for the photon
    uint2 pixelPos = uint2(500, 500);
    bool inRange;
    GetPixelPosition(hitPosition, screenDims, pixelPos, inRange);

    if (!inRange)
    {
        didHit = false;
        pixelHit = uint2(0, 0);
        return;
    }

    // Shadow Ray.
    RayDesc ray;
    ray.Origin = hitPosition;
    ray.Direction = g_sceneCB.cameraPosition.xyz - hitPosition;
    ray.TMin = 0.001;
    ray.TMax = 1.001;

    RayPayload shadowPayload =
    {
        float4(-1, 1, 0, 1), // Any extra information - Payload has to be 16 byte aligned. // TODO Use -1 as flag that this is shadow ray
    };

    // Perform Main photon tracing
    TraceRay(Scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, shadowPayload);

    float4 extraInfo = shadowPayload.extraInfo;
    int shadowHit = extraInfo[0];

    didHit = (shadowHit == 0);
    pixelHit = pixelPos;
}


inline void VisualizePhoton(float4 hitPosition, float4 photonColor, float4 photonNorm, float4 photonTangent, float2 screenDims)
{
    // Base Trace
    bool didHit = false;
    uint2 centerPixelPos;
    TraceRayToCamera(hitPosition, screenDims, didHit, centerPixelPos);

    if (didHit)
    {
        // Write the raytraced color to the output texture.

        uint3 scaledColor = photonColor.xyz * 255.f;
        uint4 newColorValue = float4(scaledColor, 1);

        InterlockedAdd(StagedRenderTarget_R[centerPixelPos], newColorValue[0]);
        InterlockedAdd(StagedRenderTarget_G[centerPixelPos], newColorValue[1]);
        InterlockedAdd(StagedRenderTarget_B[centerPixelPos], newColorValue[2]);
        InterlockedAdd(StagedRenderTarget_A[centerPixelPos], newColorValue[3]);
    }

    /*
    // TODO: This is a crude sampling code. Need to see if we can improve it. It is reducing the performing by a factor of 3 though

    float4 normalizedTangent = float4(normalize(photonTangent).xyz, 1.0);

    // Sample 1 Trace
    bool didHit1 = false;
    uint2 centerPixelPos1;
    float4 newHitPos1 = hitPosition + normalizedTangent * SEARCH_RADIUS;
    TraceRayToCamera(newHitPos1, screenDims, didHit1, centerPixelPos1);

    if (didHit1 && didHit)
    {
        // Write the raytraced color to the output texture.

        uint3 scaledColor1 = photonColor.xyz * 255.f * 0.5f;
        uint4 newColorValue1 = float4(scaledColor1, 1);

        InterlockedAdd(StagedRenderTarget_R[centerPixelPos1], newColorValue1[0]);
        InterlockedAdd(StagedRenderTarget_G[centerPixelPos1], newColorValue1[1]);
        InterlockedAdd(StagedRenderTarget_B[centerPixelPos1], newColorValue1[2]);
        InterlockedAdd(StagedRenderTarget_A[centerPixelPos1], newColorValue1[3]);
    }


    // Sample 2 Trace
    bool didHit2 = false;
    uint2 centerPixelPos2;
    float4 newHitPos2 = hitPosition - normalizedTangent * SEARCH_RADIUS;
    TraceRayToCamera(newHitPos2, screenDims, didHit2, centerPixelPos2);

    if (didHit2 && didHit)
    {
        // Write the raytraced color to the output texture.

        uint3 scaledColor2 = photonColor.xyz * 255.f * 0.5f;
        uint4 newColorValue2 = float4(scaledColor2, 1);

        InterlockedAdd(StagedRenderTarget_R[centerPixelPos2], newColorValue2[0]);
        InterlockedAdd(StagedRenderTarget_G[centerPixelPos2], newColorValue2[1]);
        InterlockedAdd(StagedRenderTarget_B[centerPixelPos2], newColorValue2[2]);
        InterlockedAdd(StagedRenderTarget_A[centerPixelPos2], newColorValue2[3]);
    }

    */

}

[shader("raygeneration")]
void MyRaygenShader()
{
    for (int i = 0; i < MAX_RAY_RECURSION_DEPTH; ++i)
    {
        uint3 g_index = uint3(DispatchRaysIndex().xy, i);

        float4 photonPos = float4(GPhotonPos[g_index].xyz, 1.0);
        float4 photonCol = GPhotonColor[g_index];
        float4 photonNorm = GPhotonNorm[g_index];
        float4 photonTangent = GPhotonTangent[g_index];

        bool didPhotonIntersect = GPhotonPos[g_index].w > 0.0f;

        if (didPhotonIntersect)
        {
            uint width, height;
            RenderTarget.GetDimensions(width, height);
            float2 screenDims = float2(width, height);

            VisualizePhoton(photonPos, photonCol, photonNorm, photonTangent, screenDims);
        }
    }
}


[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    payload.extraInfo = float4(1.0f, 0.0f, 0, 0);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    int isShadow = payload.extraInfo.x;
    if (isShadow == -1)
    {
        payload.extraInfo = float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
}

#endif // PHOTON_MAPPER_HLSL