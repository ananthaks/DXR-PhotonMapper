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

inline void VisualizePhoton(float4 hitPosition, float4 photonColor, float2 screenDims)
{
    // Find the Screen Space Coord for the photon
    uint2 pixelPos = uint2(500, 500);
    bool inRange;
    GetPixelPosition(hitPosition, screenDims, pixelPos, inRange);

    if (!inRange) 
    {
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

    if (shadowHit == 0)
    {
        // Write the raytraced color to the output texture.
        float2 tempPixel = pixelPos;
        tempPixel /= screenDims;

        float4 currentColor = StagedRenderTarget[pixelPos];
        float4 newColor = float4(currentColor.xyz + photonColor.xyz, currentColor.w + 1.0f);
        StagedRenderTarget[pixelPos] = newColor;

		// There is a Compile error for InterLockAdd 
		// Possible issue:
		// 1. You cant call for float4?
		// 2. missing header?
		/*uint4 newColorValue = float4(photonColor.xyz, 1.0f);
		uint4 newVal;
		uint4 oldVal;
		InterlockedAdd(StagedRenderTarget2[pixelPos], newVal, oldVal);*/
    }
}

[shader("raygeneration")]
void MyRaygenShader()
{
    for (int i = 0; i < MAX_RAY_RECURSION_DEPTH; ++i)
    {
        uint3 g_index = uint3(DispatchRaysIndex().xy, i);

        float4 photonPos = float4(GPhotonPos[g_index].xyz, 1.0);
        float4 photonCol = GPhotonColor[g_index];

        bool didPhotonIntersect = GPhotonPos[g_index].w > 0.0f;

        if (didPhotonIntersect)
        {
            uint width, height;
            RenderTarget.GetDimensions(width, height);
            float2 screenDims = float2(width, height);

            VisualizePhoton(photonPos, photonCol, screenDims);
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