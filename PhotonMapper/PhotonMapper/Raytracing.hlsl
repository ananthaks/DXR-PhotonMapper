//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "RaytracingHlslCompat.h"

// Unordered Access view
RWTexture2D<float4> RenderTarget : register(u0);

RaytracingAccelerationStructure Scene : register(t0, space0);
ByteAddressBuffer Indices : register(t1, space0);
StructuredBuffer<Vertex> Vertices : register(t2, space0);

// Constant Buffer views
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<CubeConstantBuffer> g_cubeCB : register(b1);

// Load three 16 bit indices from a byte addressed buffer.
uint3 Load3x16BitIndices(uint offsetBytes)
{
    uint3 indices;

    // ByteAdressBuffer loads must be aligned at a 4 byte boundary.
    // Since we need to read three 16 bit indices: { 0, 1, 2 } 
    // aligned at a 4 byte boundary as: { 0 1 } { 2 0 } { 1 2 } { 0 1 } ...
    // we will load 8 bytes (~ 4 indices { a b | c d }) to handle two possible index triplet layouts,
    // based on first index's offsetBytes being aligned at the 4 byte boundary or not:
    //  Aligned:     { 0 1 | 2 - }
    //  Not aligned: { - 0 | 1 2 }
    const uint dwordAlignedOffset = offsetBytes & ~3;    
    const uint2 four16BitIndices = Indices.Load2(dwordAlignedOffset);
 
    // Aligned: { 0 1 | 2 - } => retrieve first three 16bit indices
    if (dwordAlignedOffset == offsetBytes)
    {
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = (four16BitIndices.x >> 16) & 0xffff;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else // Not aligned: { - 0 | 1 2 } => retrieve last three 16bit indices
    {
        indices.x = (four16BitIndices.x >> 16) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = (four16BitIndices.y >> 16) & 0xffff;
    }

    return indices;
}

typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct RayPayload
{
    float4 color;
    float4 hitPosition;
    float4 extraInfo;
};

// Retrieve hit world position.
float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

// Retrieve attribute at a hit position interpolated from vertex attributes using the hit's barycentrics.
float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    float4 world = mul(float4(screenPos, 0, 1), g_sceneCB.projectionToWorld);

    world.xyz /= world.w;
    origin = g_sceneCB.cameraPosition.xyz;
    direction = normalize(world.xyz - origin);
}



inline void GetPixelPosition(float3 rayHitPosition, float2 screenDim, out uint2 pixelIndex)
{
    float4 clippingCoord = mul(float4(rayHitPosition, 1), g_sceneCB.viewProj);
    clippingCoord.xyz /= clippingCoord.z;

    pixelIndex.x = ((clippingCoord.x + 1.0) / 2.0) * screenDim.x;
    pixelIndex.y = ((1.0 - clippingCoord.y) / 2.0) * screenDim.y;
}


// Diffuse lighting calculation.
float4 CalculateDiffuseLighting(float3 hitPosition, float3 normal, float3 color)
{
    float3 pixelToLight = normalize(g_sceneCB.lightPosition.xyz - hitPosition);

    // Diffuse contribution.
    float fNDotL = max(0.0f, dot(pixelToLight, normal));

    return float4(color, 1.0) * g_sceneCB.lightDiffuseColor * fNDotL;
}

static const float PI = 3.14159265f;
inline float3 SquareToSphereUniform(float2 samplePoint)
{
    float radius = 1.f;

    float phi = samplePoint.y * PI;
    float theta = samplePoint.x * 2.f * PI;

    float3 result;
    result.x = radius * cos(theta) * sin(phi);
    result.y = radius * cos(phi);
    result.z = radius * sin(theta) * sin(phi);
    return result;
}

// Functions for PRNG
// http://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
static uint rng_state;
static const float png_01_convert = (1.0f / 4294967296.0f);
uint rand_xorshift()
{
    // Xorshift algorithm from George Marsaglia's paper
    rng_state ^= uint(rng_state << 13);
    rng_state ^= uint(rng_state >> 17);
    rng_state ^= uint(rng_state << 5);
    return rng_state;
}

// Wang hash
uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

// Generate a photon direction for a given sample point
inline void GeneratePhoton(float2 samplePoint, float2 sampleSpace, out float3 origin, out float3 rayDir)
{
    float2 normSample = samplePoint / sampleSpace;
    rayDir = SquareToSphereUniform(normSample);
    origin = g_sceneCB.lightPosition.xyz;

    // Use PRNG to generate ray direction
    rng_state = uint(wang_hash(samplePoint.x + sampleSpace.x * samplePoint.y));
    float2 randomSample = float2(rand_xorshift() * png_01_convert, rand_xorshift() * png_01_convert);
    rayDir = SquareToSphereUniform(randomSample);
}

inline void VisualizePhoton(RayPayload payload, float2 screenDims)
{
    // Get the Hit location of the photon
    float3 hitPosition = payload.hitPosition.xyz;

    // Find the Screen Space Coord for the photon
    uint2 pixelPos;
    GetPixelPosition(hitPosition, screenDims, pixelPos);

    // Shadow Ray.
    RayDesc ray;
    ray.Origin = hitPosition;
    ray.Direction = normalize(g_sceneCB.cameraPosition.xyz - hitPosition);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    RayPayload shadowPayload = 
    { 
        float4(0, 0, 0, 0), // Hit Color
        float4(0, 0, 0, 0), // Hit Location
        float4(0, 0, 0, 1), // Any extra information - Payload has to be 16 byte aligned
    };

    // Perform Main photon tracing
    TraceRay(Scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, shadowPayload);

    float4 extraInfo = shadowPayload.extraInfo;

    int shadowHit = extraInfo.x;

    if (shadowHit == 0)
    {
        // Write the raytraced color to the output texture.
        float2 tempPixel = pixelPos;
        tempPixel /= screenDims;

        RenderTarget[pixelPos] = payload.color;
    }
    
}

[shader("raygeneration")]
void MyRaygenShader()
{
    float2 samplePoint = DispatchRaysIndex().xy;
    float2 screenDims = DispatchRaysDimensions().xy;
    
    // Photon Generation
    float3 rayDir;
    float3 origin;
    GeneratePhoton(samplePoint, screenDims, origin, rayDir);

    // Trace the ray.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    // Initialize the payload
    RayPayload payload = 
    { 
        float4(0, 0, 0, 0), // Hit Color
        float4(0, 0, 0, 0), // Hit Location
        float4(0, 0, 0, 0), // Any extra information - Payload has to be 16 byte aligned
    };

    // Perform Main photon tracing
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

    // Render the photons on the screen
    VisualizePhoton(payload, screenDims);
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 hitPosition = HitWorldPosition();

    // Get the base index of the triangle's first 16 bit index.
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;

    // Load up 3 16 bit indices for the triangle.
    const uint3 indices = Load3x16BitIndices(baseIndex);

    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 vertexNormals[3] = { 
        Vertices[indices[0]].normal, 
        Vertices[indices[1]].normal, 
        Vertices[indices[2]].normal 
    };

    // Compute the triangle's normal.
    // This is redundant and done for illustration purposes 
    // as all the per-vertex normals are the same and match triangle's normal in this sample. 
    float3 triangleNormal = HitAttribute(vertexNormals, attr);


    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 vertexColors[3] = { 
        Vertices[indices[0]].color, 
        Vertices[indices[1]].color, 
        Vertices[indices[2]].color 
    };

    // Compute the triangle's normal.
    // This is redundant and done for illustration purposes 
    // as all the per-vertex normals are the same and match triangle's normal in this sample. 
    float3 triangleColor = HitAttribute(vertexColors, attr);

    float4 diffuseColor = CalculateDiffuseLighting(hitPosition, triangleNormal, triangleColor);
    float4 color = g_sceneCB.lightAmbientColor + diffuseColor;

    payload.color = color;
    payload.hitPosition = float4(hitPosition, 0.0);
    payload.extraInfo = float4(1.0f, 0.0f, 0.0f, 0.0f);
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    float4 background = float4(0.0f, 0.2f, 0.4f, 1.0f);
    payload.color = background;
    payload.hitPosition = float4(0.0, 0.0, 0.0, 0.0);
    payload.extraInfo = float4(0.0f, 0.0f, 0.0f, 0.0f);
}

#endif // RAYTRACING_HLSL