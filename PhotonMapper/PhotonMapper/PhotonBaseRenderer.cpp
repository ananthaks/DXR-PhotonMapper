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

#include "stdafx.h"
#include "PhotonBaseRenderer.h"

using namespace Microsoft::WRL;
using namespace std;

PhotonBaseRenderer::PhotonBaseRenderer(const DXRPhotonMapper::PMScene& scene, UINT width, UINT height, std::wstring name) :
    m_scene(scene),
    m_width(width),
    m_height(height),
    m_windowBounds{ 0,0,0,0 },
    m_title(name),
    m_aspectRatio(0.0f),
    m_enableUI(true),
    m_adapterIDoverride(UINT_MAX),
    m_isDxrSupported(false)
{
    WCHAR assetsPath[512];
    GetAssetsPath(assetsPath, _countof(assetsPath));
    m_assetsPath = assetsPath;

    UpdateForSizeChange(width, height);
}

PhotonBaseRenderer::~PhotonBaseRenderer()
{
}

void PhotonBaseRenderer::UpdateForSizeChange(UINT clientWidth, UINT clientHeight)
{
    m_width = clientWidth;
    m_height = clientHeight;
    m_aspectRatio = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);
}

// Helper function for resolving the full path of assets.
std::wstring PhotonBaseRenderer::GetAssetFullPath(LPCWSTR assetName)
{
    return m_assetsPath + assetName;
}


// Helper function for setting the window's title text.
void PhotonBaseRenderer::SetCustomWindowText(LPCWSTR text)
{
    std::wstring windowText = m_title + L": " + text;
    SetWindowText(Win32Application::GetHwnd(), windowText.c_str());
}

// Helper function for parsing any supplied command line args.
_Use_decl_annotations_
void PhotonBaseRenderer::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
    for (int i = 1; i < argc; ++i)
    {
        // -disableUI
        if (_wcsnicmp(argv[i], L"-disableUI", wcslen(argv[i])) == 0 ||
            _wcsnicmp(argv[i], L"/disableUI", wcslen(argv[i])) == 0)
        {
            m_enableUI = false;
        }
        // -forceAdapter [id]
        else if (_wcsnicmp(argv[i], L"-forceAdapter", wcslen(argv[i])) == 0 ||
            _wcsnicmp(argv[i], L"/forceAdapter", wcslen(argv[i])) == 0)
        {
            ThrowIfFalse(i + 1 < argc, L"Incorrect argument format passed in.");

            m_adapterIDoverride = _wtoi(argv[i + 1]);
            i++;
        }
    }

}

void PhotonBaseRenderer::SetWindowBounds(int left, int top, int right, int bottom)
{
    m_windowBounds.left = static_cast<LONG>(left);
    m_windowBounds.top = static_cast<LONG>(top);
    m_windowBounds.right = static_cast<LONG>(right);
    m_windowBounds.bottom = static_cast<LONG>(bottom);
}


void PhotonBaseRenderer::GetVerticesForPrimitiveType(DXRPhotonMapper::PrimitiveType type, std::vector<Vertex>& vertices)
{
    switch (type)
    {
    case DXRPhotonMapper::PrimitiveType::Cube:
    {
        vertices = 
        {
            // Top Face
            { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
            { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
            { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
            { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

            // Bottom Face
            { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
            { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
            { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
            { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },

            // Left Face
            { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },

            // Right Face
            { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },

            // Back Face
            { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
            { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
            { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
            { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },

            // Front Face
            { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
            { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
            { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
            { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
        };
    }
    break;
    case DXRPhotonMapper::PrimitiveType::SquarePlane:
    {
        vertices =
        {
            { XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
            { XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
            { XMFLOAT3(0.5f, 0.5f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) },
            { XMFLOAT3(-0.5f, 0.5f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0) },
        };
    }
    break;
    case DXRPhotonMapper::PrimitiveType::Error:
    default:
        break;
    }
}

void PhotonBaseRenderer::GetIndicesForPrimitiveType(DXRPhotonMapper::PrimitiveType type, std::vector<Index>& indices)
{
    switch (type)
    {
    case DXRPhotonMapper::PrimitiveType::Cube:
    {
        indices = {
            3,1,0,
            2,1,3,

            6,4,5,
            7,4,6,

            11,9,8,
            10,9,11,

            14,12,13,
            15,12,14,

            19,17,16,
            18,17,19,

            22,20,21,
            23,20,22,
        };
    }
    break;
    case DXRPhotonMapper::PrimitiveType::SquarePlane:
    {
        indices =
        {
            3,1,0,
            2,1,3,
        };
    }
    break;
    case DXRPhotonMapper::PrimitiveType::Error:
    default:
        break;
    }
}

void PhotonBaseRenderer::BuildGeometryBuffers()
{
    auto device = m_deviceResources->GetD3DDevice();
    for (int i = 0; i < m_scene.m_primitives.size(); ++i)
    {
        DXRPhotonMapper::Primitive prim = m_scene.m_primitives[i];

        GeometryBuffer geoBuffer = {};

        std::vector<Vertex> vertices;
        GetVerticesForPrimitiveType(prim.m_primitiveType, vertices);

        std::vector<Index> indices;
        GetIndicesForPrimitiveType(prim.m_primitiveType, indices);

        AllocateUploadBuffer(device, indices.data(), indices.size() * sizeof(Index), &geoBuffer.indexBuffer.resource);
        AllocateUploadBuffer(device, vertices.data(), vertices.size() * sizeof(Vertex), &geoBuffer.vertexBuffer.resource);

        geoBuffer.indexNumElements = indices.size() * sizeof(Index) / 4;
        geoBuffer.indexElementSize = 0; // Since we are going to create a raw buffer

        geoBuffer.vertexNumElements = vertices.size();
        geoBuffer.vertexElementSize = sizeof(vertices[0]);

        const XMVECTOR translationVec = XMLoadFloat3(&prim.m_translate);
        XMMATRIX transMat = XMMatrixTranslationFromVector(translationVec);
        XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(XMConvertToRadians(prim.m_rotate.x), XMConvertToRadians(prim.m_rotate.y), XMConvertToRadians(prim.m_rotate.z));
        XMMATRIX scaleMat = XMMatrixScaling(prim.m_scale.x, prim.m_scale.y, prim.m_scale.z);

        geoBuffer.transformationMatrix = scaleMat * rotMat * transMat;
        geoBuffer.normalTransform = rotMat;


        m_geometryBuffers.push_back(geoBuffer);
    }

    // Create SRVs for Index Buffers
    for (auto& geoBuffer : m_geometryBuffers)
    {
        UINT descriptorIndexIB = CreateBufferSRV(&geoBuffer.indexBuffer, geoBuffer.indexNumElements, geoBuffer.indexElementSize);
    }

    // Create SRVs for Vertex Buffers
    for (auto& geoBuffer : m_geometryBuffers)
    {
        UINT descriptorIndexVB = CreateBufferSRV(&geoBuffer.vertexBuffer, geoBuffer.vertexNumElements, geoBuffer.vertexElementSize);
    }
}

void PhotonBaseRenderer::BuildGeometrySceneBufferDesc()
{
    auto device = m_deviceResources->GetD3DDevice();
    for (size_t i = 0; i < m_scene.m_primitives.size(); ++i)
    {
        SceneBufferDescHolder sceneBufferDescHolder = {};
        sceneBufferDescHolder.sceneBufferDesc.vbIndex = UINT(i);
        sceneBufferDescHolder.sceneBufferDesc.materialIndex = UINT(m_scene.m_primitives[i].m_materialID);
        sceneBufferDescHolder.sceneBufferDesc.normalTransformMat = m_geometryBuffers[i].normalTransform;

        const size_t bufferSize = (sizeof(SceneBufferDesc) + 255) & ~255;
        CreateUploadBuffer(device, bufferSize, &sceneBufferDescHolder.sceneBufferDescRes.resource);

        UINT descriptorIndex = AllocateDescriptor(&sceneBufferDescHolder.sceneBufferDescRes.cpuDescriptorHandle);
        sceneBufferDescHolder.sceneBufferDescRes.gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_descriptorSize);

        // Create a CBV descriptor
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = sceneBufferDescHolder.sceneBufferDescRes.resource->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = bufferSize;
        device->CreateConstantBufferView(&cbvDesc, sceneBufferDescHolder.sceneBufferDescRes.cpuDescriptorHandle);

        CD3DX12_RANGE readRange(0, 0);
        SceneBufferDesc *mappedConstantData;
        ThrowIfFailed(sceneBufferDescHolder.sceneBufferDescRes.resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedConstantData)));
        memcpy(mappedConstantData, &sceneBufferDescHolder.sceneBufferDesc, sizeof(SceneBufferDesc));
        sceneBufferDescHolder.sceneBufferDescRes.resource->Unmap(0, &readRange);

        m_sceneBufferDescriptors.push_back(sceneBufferDescHolder);
    }
}

void PhotonBaseRenderer::BuildMaterialBuffer()
{
    auto device = m_deviceResources->GetD3DDevice();
    for (size_t i = 0; i < m_scene.m_materials.size(); ++i)
    {
        MaterialDescHolder materialDescHolder = {};
        materialDescHolder.materialDesc.materialID = UINT(i);
        materialDescHolder.materialDesc.albedo = m_scene.m_materials[i].m_baseMaterials[0].m_albedo;

        const size_t bufferSize = (sizeof(MaterialDesc) + 255) & ~255;
        CreateUploadBuffer(device, bufferSize, &materialDescHolder.materialDescRes.resource);

        UINT descriptorIndex = AllocateDescriptor(&materialDescHolder.materialDescRes.cpuDescriptorHandle);
        materialDescHolder.materialDescRes.gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_descriptorSize);

        // Create a CBV descriptor
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = materialDescHolder.materialDescRes.resource->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = bufferSize;
        device->CreateConstantBufferView(&cbvDesc, materialDescHolder.materialDescRes.cpuDescriptorHandle);

        CD3DX12_RANGE readRange(0, 0);
        SceneBufferDesc *mappedConstantData;
        ThrowIfFailed(materialDescHolder.materialDescRes.resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedConstantData)));
        memcpy(mappedConstantData, &materialDescHolder.materialDesc, sizeof(MaterialDesc));
        materialDescHolder.materialDescRes.resource->Unmap(0, &readRange);

        m_materialDescriptors.push_back(materialDescHolder);
    }
}

void PhotonBaseRenderer::BuildLightBuffer()
{
    auto device = m_deviceResources->GetD3DDevice();
    for (size_t i = 0; i < m_scene.m_lights.size(); ++i)
    {
        LightDescHolder lightDescHolder = {};
        lightDescHolder.lightDesc.shapeID = static_cast<UINT>(m_scene.m_lights[i].m_lightType);
        lightDescHolder.lightDesc.lightIntensity = m_scene.m_lights[i].m_lightIntensity;
        lightDescHolder.lightDesc.lightColor = m_scene.m_lights[i].m_lightColor;

        const size_t bufferSize = (sizeof(LightDesc) + 255) & ~255;
        CreateUploadBuffer(device, bufferSize, &lightDescHolder.lightDescRes.resource);

        UINT descriptorIndex = AllocateDescriptor(&lightDescHolder.lightDescRes.cpuDescriptorHandle);
        lightDescHolder.lightDescRes.gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_descriptorSize);

        // Create a CBV descriptor
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = lightDescHolder.lightDescRes.resource->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = bufferSize;
        device->CreateConstantBufferView(&cbvDesc, lightDescHolder.lightDescRes.cpuDescriptorHandle);

        CD3DX12_RANGE readRange(0, 0);
        SceneBufferDesc *mappedConstantData;
        ThrowIfFailed(lightDescHolder.lightDescRes.resource->Map(0, &readRange, reinterpret_cast<void**>(&mappedConstantData)));
        memcpy(mappedConstantData, &lightDescHolder.lightDesc, sizeof(LightDesc));
        lightDescHolder.lightDescRes.resource->Unmap(0, &readRange);

        m_lightDescriptors.push_back(lightDescHolder);
    }
}

void PhotonBaseRenderer::BuildGeometryAccelerationStructures()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto commandList = m_deviceResources->GetCommandList();
    auto commandQueue = m_deviceResources->GetCommandQueue();
    auto commandAllocator = m_deviceResources->GetCommandAllocator();

    // Reset the command list for the acceleration structure construction.
    commandList->Reset(commandAllocator, nullptr);

    // 1. Create a unique Bottom Level Acceleration Structure for each primitive
    for (auto& geoBuffer : m_geometryBuffers)
    {
        // 1a. Get the geometry descriptors
        D3D12_RAYTRACING_GEOMETRY_DESC geoDesc = GetRayTracingGeometryDescriptor(geoBuffer);

        geoBuffer.geometryDescs.clear();
        geoBuffer.geometryDescs.push_back(geoDesc);

        geoBuffer.bottomLevelAccStructDesc = {};

        // 1b. BLAS: Create the bottom level inputs
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = geoBuffer.bottomLevelAccStructDesc.Inputs;
        bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        bottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        bottomLevelInputs.NumDescs = static_cast<UINT>(geoBuffer.geometryDescs.size());
        bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        bottomLevelInputs.pGeometryDescs = geoBuffer.geometryDescs.data();

        // 1c. BLAS: Create a descriptor for prebuild and query for scratch and result space
        geoBuffer.bottomLevelAccStructPreBuildInfo = {};

        if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
        {
            m_fallbackDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &geoBuffer.bottomLevelAccStructPreBuildInfo);
        }
        else // DirectX Raytracing
        {
            m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &geoBuffer.bottomLevelAccStructPreBuildInfo);
        }
        ThrowIfFalse(geoBuffer.bottomLevelAccStructPreBuildInfo.ResultDataMaxSizeInBytes > 0);

        // 1d. BLAS: Allocate space for the scratch space
        AllocateUAVBuffer(device, geoBuffer.bottomLevelAccStructPreBuildInfo.ScratchDataSizeInBytes, &geoBuffer.bottomLevelScratchRes, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"BotLevelScratchResource");

        // 1e. BLAS: Allocate space for the acc structure
        D3D12_RESOURCE_STATES initialResourceState;
        if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
        {
            initialResourceState = m_fallbackDevice->GetAccelerationStructureResourceState();
        }
        else // DirectX Raytracing
        {
            initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }
        AllocateUAVBuffer(device, geoBuffer.bottomLevelAccStructPreBuildInfo.ResultDataMaxSizeInBytes, &geoBuffer.bottomLevelAccStructure, initialResourceState, L"BottomLevelAccelerationStructure");
    }

    // 2a. Create a Descriptor for the top Level Inputs
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelBuildDesc.Inputs;
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    topLevelInputs.NumDescs = m_geometryBuffers.size();
    topLevelInputs.pGeometryDescs = nullptr;
    topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    // 2b. Get the Prebuild info - scratch space and result space size for top level acceleration structure
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        m_fallbackDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
    }
    else // DirectX Raytracing
    {
        m_dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
    }
    ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    // 2c. Allocate scratch space for building the top level acc structure
    ComPtr<ID3D12Resource> topLevelScratchRes;
    AllocateUAVBuffer(device, topLevelPrebuildInfo.ScratchDataSizeInBytes, &topLevelScratchRes, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"TopLevelScratchResource");

    // 2d. Allocate result space for the top level acc structure
    D3D12_RESOURCE_STATES initialResourceState;
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        initialResourceState = m_fallbackDevice->GetAccelerationStructureResourceState();
    }
    else // DirectX Raytracing
    {
        initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    }
    AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_topLevelAccelerationStructure, initialResourceState, L"TopLevelAccelerationStructure");

    // Note on Emulated GPU pointers (AKA Wrapped pointers) requirement in Fallback Layer:
    // The primary point of divergence between the DXR API and the compute-based Fallback layer is the handling of GPU pointers. 
    // DXR fundamentally requires that GPUs be able to dynamically read from arbitrary addresses in GPU memory. 
    // The existing Direct Compute API today is more rigid than DXR and requires apps to explicitly inform the GPU what blocks of memory it will access with SRVs/UAVs.
    // In order to handle the requirements of DXR, the Fallback Layer uses the concept of Emulated GPU pointers, 
    // which requires apps to create views around all memory they will access for raytracing, 
    // but retains the DXR-like flexibility of only needing to bind the top level acceleration structure at DispatchRays.
    //
    // The Fallback Layer interface uses WRAPPED_GPU_POINTER to encapsulate the underlying pointer
    // which will either be an emulated GPU pointer for the compute - based path or a GPU_VIRTUAL_ADDRESS for the DXR path.

    // 3. Create an instance desc for each of the bottom-level acceleration structure.    
    ComPtr<ID3D12Resource> instanceDescs;
    std::vector<D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC> fallbackInstanceDescriptions;
    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> dxrInstanceDescriptions;
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        UINT instanceId = 0;
        for (auto& geoBuffer : m_geometryBuffers)
        {
            D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC instanceDesc = {};
            XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instanceDesc.Transform), geoBuffer.transformationMatrix);
            instanceDesc.InstanceMask = 1;
            instanceDesc.InstanceID = instanceId++;
            UINT numBufferElements = static_cast<UINT>(geoBuffer.bottomLevelAccStructPreBuildInfo.ResultDataMaxSizeInBytes) / sizeof(UINT32);
            instanceDesc.AccelerationStructure = CreateFallbackWrappedPointer(geoBuffer.bottomLevelAccStructure.Get(), numBufferElements);
            fallbackInstanceDescriptions.push_back(instanceDesc);
        }
        AllocateUploadBuffer(device, fallbackInstanceDescriptions.data(), fallbackInstanceDescriptions.size() * sizeof(D3D12_RAYTRACING_FALLBACK_INSTANCE_DESC), &instanceDescs, L"InstanceDescs");
    }
    else // DirectX Raytracing
    {
        UINT instanceId = 0;
        for (auto& geoBuffer : m_geometryBuffers)
        {
            D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
            XMStoreFloat3x4(reinterpret_cast<XMFLOAT3X4*>(instanceDesc.Transform), geoBuffer.transformationMatrix);
            instanceDesc.InstanceMask = 1;
            instanceDesc.InstanceID = instanceId++;
            instanceDesc.AccelerationStructure = geoBuffer.bottomLevelAccStructure->GetGPUVirtualAddress();
            dxrInstanceDescriptions.push_back(instanceDesc);
        }
        AllocateUploadBuffer(device, dxrInstanceDescriptions.data(), dxrInstanceDescriptions.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), &instanceDescs, L"InstanceDescs");
    }

    // 4. Create a wrapped pointer to the acceleration structure.
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        UINT numBufferElements = static_cast<UINT>(topLevelPrebuildInfo.ResultDataMaxSizeInBytes) / sizeof(UINT32);
        m_fallBackPrimitiveTLAS = CreateFallbackWrappedPointer(m_topLevelAccelerationStructure.Get(), numBufferElements);
    }

    // 5. Assign the scratch and result space for the BLAS
    for (auto& geoBuffer : m_geometryBuffers)
    {
        geoBuffer.bottomLevelAccStructDesc.ScratchAccelerationStructureData = geoBuffer.bottomLevelScratchRes->GetGPUVirtualAddress();
        geoBuffer.bottomLevelAccStructDesc.DestAccelerationStructureData = geoBuffer.bottomLevelAccStructure->GetGPUVirtualAddress();
    }

    // 6. Assign Top Level Acceleration Structure desc
    {
        topLevelBuildDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
        topLevelBuildDesc.ScratchAccelerationStructureData = topLevelScratchRes->GetGPUVirtualAddress();
        topLevelBuildDesc.Inputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
    }

    // 7. Build acceleration structure.
    if (m_raytracingAPI == RaytracingAPI::FallbackLayer)
    {
        // Set the descriptor heaps to be used during acceleration structure build for the Fallback Layer.
        ID3D12DescriptorHeap *pDescriptorHeaps[] = { m_descriptorHeap.Get() };
        m_fallbackCommandList->SetDescriptorHeaps(ARRAYSIZE(pDescriptorHeaps), pDescriptorHeaps);
        for (auto& geoBuffer : m_geometryBuffers)
        {
            m_fallbackCommandList->BuildRaytracingAccelerationStructure(&geoBuffer.bottomLevelAccStructDesc, 0, nullptr);
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(geoBuffer.bottomLevelAccStructure.Get()));
        }
        m_fallbackCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
    }
    else // DirectX Raytracing
    {
        for (auto& geoBuffer : m_geometryBuffers)
        {
            m_dxrCommandList->BuildRaytracingAccelerationStructure(&geoBuffer.bottomLevelAccStructDesc, 0, nullptr);
            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(geoBuffer.bottomLevelAccStructure.Get()));
        }
        m_dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
    }

    // 8. Kick off acceleration structure construction.
    m_deviceResources->ExecuteCommandList();

    // 9. Wait for GPU to finish as the locally created temporary GPU resources will get released once we go out of scope.
    m_deviceResources->WaitForGpu();
}

// Allocate a descriptor and return its index. 
// If the passed descriptorIndexToUse is valid, it will be used instead of allocating a new one.
UINT PhotonBaseRenderer::AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse)
{
    auto descriptorHeapCpuBase = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    if (descriptorIndexToUse >= m_descriptorHeap->GetDesc().NumDescriptors)
    {
        descriptorIndexToUse = m_descriptorsAllocated++;
    }
    *cpuDescriptor = CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeapCpuBase, descriptorIndexToUse, m_descriptorSize);
    return descriptorIndexToUse;
}

// Create SRV for a buffer.
UINT PhotonBaseRenderer::CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize)
{
    auto device = m_deviceResources->GetD3DDevice();

    // SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = numElements;
    if (elementSize == 0)
    {
        srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
        srvDesc.Buffer.StructureByteStride = 0;
    }
    else
    {
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        srvDesc.Buffer.StructureByteStride = elementSize;
    }
    UINT descriptorIndex = AllocateDescriptor(&buffer->cpuDescriptorHandle);
    device->CreateShaderResourceView(buffer->resource.Get(), &srvDesc, buffer->cpuDescriptorHandle);
    buffer->gpuDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(), descriptorIndex, m_descriptorSize);
    return descriptorIndex;
}

// Create a wrapped pointer for the Fallback Layer path.
WRAPPED_GPU_POINTER PhotonBaseRenderer::CreateFallbackWrappedPointer(ID3D12Resource* resource, UINT bufferNumElements)
{
    auto device = m_deviceResources->GetD3DDevice();

    D3D12_UNORDERED_ACCESS_VIEW_DESC rawBufferUavDesc = {};
    rawBufferUavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    rawBufferUavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
    rawBufferUavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    rawBufferUavDesc.Buffer.NumElements = bufferNumElements;

    D3D12_CPU_DESCRIPTOR_HANDLE bottomLevelDescriptor;

    // Only compute fallback requires a valid descriptor index when creating a wrapped pointer.
    UINT descriptorHeapIndex = 0;
    if (!m_fallbackDevice->UsingRaytracingDriver())
    {
        descriptorHeapIndex = AllocateDescriptor(&bottomLevelDescriptor);
        device->CreateUnorderedAccessView(resource, nullptr, &rawBufferUavDesc, bottomLevelDescriptor);
    }
    return m_fallbackDevice->GetWrappedPointerSimple(descriptorHeapIndex, resource->GetGPUVirtualAddress());
}

D3D12_RAYTRACING_GEOMETRY_DESC PhotonBaseRenderer::GetRayTracingGeometryDescriptor(const GeometryBuffer& geoBuffer)
{
    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Triangles.IndexBuffer = geoBuffer.indexBuffer.resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.IndexCount = static_cast<UINT>(geoBuffer.indexBuffer.resource->GetDesc().Width) / sizeof(Index);
    geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
    geometryDesc.Triangles.Transform3x4 = 0;
    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    geometryDesc.Triangles.VertexCount = static_cast<UINT>(geoBuffer.vertexBuffer.resource->GetDesc().Width) / sizeof(Vertex);
    geometryDesc.Triangles.VertexBuffer.StartAddress = geoBuffer.vertexBuffer.resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    return geometryDesc;
}

UINT PhotonBaseRenderer::GetNumDescriptorsForScene()
{
    // Allocate a heap for descriptors:
    // n - vertex SRVs
    // n - index SRVs
    // n - constant buffer views for scene buffer info
    // n - constant buffer views for materials
    // n - constant buffer views for lights
    // n - bottom level acceleration structure fallback wrapped pointer UAVs
    // n - top level acceleration structure fallback wrapped pointer UAVs
    const UINT numVertexSRVDescs = UINT(m_scene.m_primitives.size());
    const UINT numIndexSRVDescs = UINT(m_scene.m_primitives.size());
    const UINT numSceneCBVDescs = UINT(m_scene.m_primitives.size());
    const UINT numMaterialCBVDescs = UINT(m_scene.m_materials.size());
    const UINT numLightCBVDescs = UINT(m_scene.m_lights.size());
    const UINT numBLASDescs = UINT(m_scene.m_primitives.size());
    const UINT numTLASDescs = UINT(m_scene.m_primitives.size());

    return (numVertexSRVDescs + numIndexSRVDescs + numSceneCBVDescs + numMaterialCBVDescs + numLightCBVDescs + numBLASDescs + numTLASDescs);
}
