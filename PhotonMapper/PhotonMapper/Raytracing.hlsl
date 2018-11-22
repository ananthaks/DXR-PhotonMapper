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
    float4 extraInfo; // [0] is shadowHit, [1] is bounce depth
    float3 throughput;
    float3 direction;
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

// Sampling functions
static const float PI = 3.14159265f;
static const float INV_PI = 0.318309886f;
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

inline float SquareToSphereUniformPDF(in float3 sample)
{
    // Return 1 / (Surface area of sphere)
    return INV_PI * 0.25f; //(1.f / 4.f);
}

inline float3 SquareToDiskConcentric(in float2 sample)
{
    // Used Peter Shirley's concentric disk warp
    float radius;
    float angle;
    float a = (2 * sample[0]) - 1;
    float b = (2 * sample[1]) - 1;

    if (a > -b) {
        if (a > b) {
            radius = a;
            angle = (PI / 4.f) * (b / a);
        }
        else {
            radius = b;
            angle = (PI / 4.f) * (2 - (a / b));
        }
    }
    else {
        if (a < b) {
            radius = -a;
            angle = (PI / 4.f) * (4 + (b / a));
        }
        else {
            radius = -b;
            if (b != 0) {
                angle = (PI / 4.f) * (6 - (a / b));
            }
            else {
                angle = 0;
            }
        }
    }
    return float3(radius * cos(angle), radius * sin(angle), 0);
}

inline float3 SquareToHemisphereCosine(in float2 sample)
{
    // Used Peter Shirley's cosine hemisphere
    float3 disk = SquareToDiskConcentric(sample);
    return float3(disk[0], disk[1], sqrt(1.f - pow(length(disk), 2.f)));
}

inline float SquareToHemisphereCosinePDF(in float3 sample)
{
    // Return 1 / (PI * cos(theta))
    // theta is angle between this point and north pole
    // Thus, theta is the z value of sample because z is how
    // far down the point is along the unit sphere
    return INV_PI * sample[2];
}

inline bool SameHemisphere(in float3 w, in float3 wp) {
    return w.z * wp.z > 0;
}

inline float AbsCosTheta(in float3 w) { return abs(w.z); }

inline float AbsDot(in float3 a, in float3 b)
{
    return abs(dot(a, b));
}

inline float maxValue(in float3 w) {
    return max(w.x, max(w.y, w.z));
}



// Functions for PRNG
// http://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11/
static uint rng_state;
static const float prng_01_convert = (1.0f / 4294967296.0f); // Convert numbers generated by rand_xorshift to be between 0, 1
float rand_xorshift()
{
    // Xorshift algorithm from George Marsaglia's paper
    rng_state ^= uint(rng_state << 13);
    rng_state ^= uint(rng_state >> 17);
    rng_state ^= uint(rng_state << 5);
    return rng_state * prng_01_convert;
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
    float2 randomSample = float2(rand_xorshift(), rand_xorshift());
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
        float4(0, 1, 0, 1), // Any extra information - Payload has to be 16 byte aligned
        float3(0, 0, 0), // Throughput
        float3(0, 0, 0), // Direction
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

    // TODO max depth is 5. Do we want to change that?
    // Initialize the payload
    RayPayload payload = 
    { 
        //float4(0, 0, 0, 0), // Hit Color
        g_sceneCB.lightDiffuseColor, // Photon's starting color
        float4(0, 0, 0, 0), // Hit Location
        float4(0, 5, 0, 0), // Any extra information - Payload has to be 16 byte aligned
        float3(1, 1, 1), // Throughput
        float3(rayDir), // Direction
    };

    // Perform Main photon tracing
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

    // Render the photons on the screen
    VisualizePhoton(payload, screenDims);
}

inline float3 Lambert_Sample_f(in float3 wo, inout float3 wi, in float2 sample, inout float pdf, in float3 albedo) {
    wi = SquareToHemisphereCosine(sample);
    if (wo.z < 0) wi.z *= -1;
    wi = normalize(wi);
    pdf = SameHemisphere(wo, wi) ? INV_PI * AbsCosTheta(wi) : 0;

    return INV_PI * albedo;
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    int depth = payload.extraInfo.y; // Using [1] crashes...
    if (depth == 0) {
        return;
    }
    depth--;


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


    // TODO We don't have UVs
    // Calculate tangent and bitangent of triangle using its points
    float3 tangent = normalize(Vertices[indices[0]].position - hitPosition);
    float3 bitangent = normalize(cross(triangleNormal, tangent));

    // TODO make sure these are columns
    float3x3 tangentToWorld = float3x3(tangent, bitangent, triangleNormal);
    float3x3 worldToTangent = transpose(tangentToWorld);

    // BSDF. Assume everything is only Lambert
    // TODO Add more material types
    float2 randomSample = float2(rand_xorshift(), rand_xorshift());
    float3 woW = -payload.direction;
    float3 wo = normalize(mul(worldToTangent, woW));
    float3 wi;
    float pdf;

    float3 f = Lambert_Sample_f(wo, wi, randomSample, pdf, triangleColor);

    float3 wiW = normalize(mul(tangentToWorld, wi));

    // TODO Store photon
    //VisualizePhoton(payload, DispatchRaysDimensions().xy); // Doesn't work....

    float3 curr_throughput = f * AbsDot(triangleNormal, wiW);

    payload.throughput = payload.throughput * curr_throughput;
    payload.color = float4(payload.color * curr_throughput, 1);
    payload.hitPosition = float4(hitPosition, 0.0);
    payload.extraInfo = float4(1.0f, depth, 0.0f, 0.0f);

    // Russian Roulette 
    // TODO
    float throughput_max = maxValue(throughput);
    if (rand_xorshift() < (1.f - throughput_max)) {
        return;
    }


    // TODO
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
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