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

#ifndef PHOTON_MAPPER_HLSL
#define PHOTON_MAPPER_HLSL

#define HLSL
#include "RaytracingHlslCompat.h"

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

// Sampling functions
static const float PI = 3.1415926535897932384626422832795028841971f;
static const float INV_PI = 0.318309886f;
static const float TWO_PI = 6.2831853071795864769252867665590057683943f;
static const float SQRT_OF_ONE_THIRD = 0.5773502691896257645091487805019574556476f;
static const float EPSILON = 0.0001f;

inline float3 SquareToSphereUniform(float2 samplePoint)
{
	float radius = 1.f;

	float phi = samplePoint.y * PI;
	float theta = samplePoint.x * TWO_PI;

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
	//float2 normSample = samplePoint / sampleSpace;
	//rayDir = SquareToSphereUniform(normSample);
	origin = g_sceneCB.lightPosition.xyz;

	// Use PRNG to generate ray direction
	float2 randomSample = float2(rand_xorshift(), rand_xorshift());
	rayDir = SquareToSphereUniform(randomSample);
}

inline void VisualizePhoton(RayPayload payload, float2 screenDims)
{
	// Get the Hit location of the photon
	float3 hitPosition = payload.hitPosition.xyz;

	// Find the Screen Space Coord for the photon
	uint2 pixelPos;
	bool inRange;
	GetPixelPosition(hitPosition, screenDims, pixelPos, inRange);

	if (!inRange) {
		return;
	}

	// Shadow Ray.
	RayDesc ray;
	ray.Origin = hitPosition;
	//ray.Direction = normalize(g_sceneCB.cameraPosition.xyz - hitPosition);
	ray.Direction = g_sceneCB.cameraPosition.xyz - hitPosition;
	ray.TMin = 0.001;
	ray.TMax = 1.001;

	RayPayload shadowPayload =
	{
		float4(0, 0, 0, 0), // Hit Color
		float4(0, 0, 0, 0), // Hit Location
		float4(-1, 1, 0, 1), // Any extra information - Payload has to be 16 byte aligned. // TODO Use -1 as flag that this is shadow ray
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
		//GPhotonPos[pixelPos] = payload.color;
	}

}

// Diffuse lighting calculation.
inline float4 CalculateDiffuseLighting2(float3 hitPosition, float3 normal)
{
    float3 pixelToLight = normalize(g_sceneCB.lightPosition.xyz - hitPosition);

    // Diffuse contribution.
    float fNDotL = max(0.0f, dot(pixelToLight, normal));

    return float4(1, 1, 1, 1) * g_sceneCB.lightDiffuseColor * fNDotL;
}


[shader("raygeneration")]
void MyRaygenShader()
{
    float3 rayDir;
    float3 origin;

    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir);

    // Trace the ray.
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0),
    float4(0, 0, 0, 0),
    float4(0, 0, 0, 0),
    float3(0, 0, 0),
    float3(0, 0, 0)};
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex().xy] = payload.color;
}


// From hw 3
inline float3 calculateRandomDirectionInHemisphere(in float3 normal) {

	float up = sqrt(rand_xorshift()); // cos(theta)
	float over = sqrt(1 - up * up); // sin(theta)
	float around = rand_xorshift() * TWO_PI;

	// Find a direction that is not the normal based off of whether or not the
	// normal's components are all equal to sqrt(1/3) or whether or not at
	// least one component is less than sqrt(1/3). Learned this trick from
	// Peter Kutz.

	float3 directionNotNormal;
	if (abs(normal.x) < SQRT_OF_ONE_THIRD) {
		directionNotNormal = float3(1, 0, 0);
	}
	else if (abs(normal.y) < SQRT_OF_ONE_THIRD) {
		directionNotNormal = float3(0, 1, 0);
	}
	else {
		directionNotNormal = float3(0, 0, 1);
	}

	// Use not-normal direction to generate two perpendicular directions
	float3 perpendicularDirection1 =
		normalize(cross(normal, directionNotNormal));
	float3 perpendicularDirection2 =
		normalize(cross(normal, perpendicularDirection1));

	return up * normal
		+ cos(around) * over * perpendicularDirection1
		+ sin(around) * over * perpendicularDirection2;
}


inline float3 Lambert_Sample_f(in float3 wo, out float3 wi, in float2 sample, out float pdf, in float3 albedo) {
	wi = SquareToHemisphereCosine(sample);
	if (wo.z < 0) wi.z *= -1;
	wi = normalize(wi);
	pdf = SameHemisphere(wo, wi) ? INV_PI * AbsCosTheta(wi) : 0;

	return INV_PI * albedo;
}

#define PHOTON_CLOSENESS 0.5f

inline float3 ColorToWorld(float3 color)
{
	return ((color - float3(0.5f, 0.5f, 0.5f)) * (2.0f * MAX_SCENE_SIZE));
}

uint3 PosToCellId(float3 worldPosition)
{
	float3 correctWorldPos = (worldPosition + float3(MAX_SCENE_SIZE, MAX_SCENE_SIZE, MAX_SCENE_SIZE)) / 2.0;
	return uint3(floor(correctWorldPos / CELL_SIZE));
}


uint Cell3DTo1D(uint3 cellId)
{
	return uint(cellId.x + MAX_SCENE_SIZE * cellId.y + MAX_SCENE_SIZE * MAX_SCENE_SIZE * cellId.z); // TODO check if correct
}

uint3 Cell1DTo3D(uint id)
{
	uint3 temp;
	temp.x = id % MAX_SCENE_SIZE;
	temp.y = (id / MAX_SCENE_SIZE) % MAX_SCENE_SIZE;
	temp.z = id / (MAX_SCENE_SIZE * MAX_SCENE_SIZE);
	return temp;
}

inline float4 PerformNaiveSearch(float3 intersectionPoint)
{
	uint width, height, depth;
	GPhotonPos.GetDimensions(width, height, depth);
	
	float4 color = float4(0.0, 0.0, 0.0, 0.0);
	int numColors = 0;

	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			for (int k = 0; k < depth; ++k)
			{
				uint3 index = uint3(i, j, k);
				float dist = distance(intersectionPoint, GPhotonPos[index].xyz);

				if (dist < PHOTON_CLOSENESS)
				{
					color += GPhotonColor[index];
					numColors++;
				}
			}
		}
	}

	if (numColors != 0)
	{
		return color / numColors;
	}
    return float4(0.0, 0.0, 0.0, 1.0);
}


uint3 Cell1DToPhotonID(uint id)
{
	uint width, height, depth;
	GPhotonPos.GetDimensions(width, height, depth);
	uint3 temp;
	temp.x = id % width;
	temp.y = (id / width) % height;
	temp.z = min(id / (width * height), depth - 1);
	return temp;
}

/*
uint3 currCell = uint3(x, y, z);

uint photonStart = GPhotonScan[currCell];
uint photonCount = GPhotonCount[currCell];

for (int photon = photonStart; photon < photonCount; ++photon)
{
	uint3 index = Cell1DToPhotonID(photon);
	float dist = distance(intersectionPoint, ColorToWorld(GPhotonSortedPos[index].xyz));

	if (dist < PHOTON_CLOSENESS)
	{
		color += GPhotonSortedCol[index];
		numPhotons++;
	}
}*/

inline float4 PerformSorted(float3 intersectionPoint)
{
	uint3 cellId = PosToCellId(intersectionPoint);

	uint3 minCellSearch = cellId - uint3(1, 1, 1);
	uint3 maxCellSearch = clamp(cellId + uint3(1, 1, 1), uint3(0, 0, 0), uint3(MAX_SCENE_SIZE - 1, MAX_SCENE_SIZE - 1, MAX_SCENE_SIZE - 1));

	float4 color = float4(0.0, 0.0, 0.0, 0.0);
	int numPhotons = 0;

	for (int x = minCellSearch[0]; x < maxCellSearch[0]; ++x)
	{
		for (int y = minCellSearch[1]; y < maxCellSearch[1]; ++y)
		{
			for (int z = minCellSearch[2]; z < maxCellSearch[2]; ++z)
			{
				uint3 currCell = uint3(x, y, z);

				uint photonStart = GPhotonScan[currCell];
				uint photonCount = GPhotonCount[currCell];

				for (int photon = photonStart; photon < photonCount; ++photon)
				{
					uint3 index = Cell1DToPhotonID(photon);
					float dist = distance(intersectionPoint, ColorToWorld(GPhotonSortedPos[index].xyz));

					if (dist < PHOTON_CLOSENESS)
					{
						color += GPhotonSortedCol[index];
						numPhotons++;
					}
				}
			}
		}
	}

	if (numPhotons != 0)
	{
		return color / numPhotons;
	}
	return float4(1.0, 0.0, 0.0, 1.0);
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 hitPosition = HitWorldPosition();
    payload.color = PerformSorted(hitPosition);

}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
	int isShadow = payload.extraInfo.x;

	if (isShadow == -1) {
		float4 background = float4(1.0f, 1.0f, 1.0f, 1.0f);
		payload.color = background;
		payload.hitPosition = float4(0.0, 0.0, 0.0, 0.0);
		payload.extraInfo = float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

#endif // PHOTON_MAPPER_HLSL