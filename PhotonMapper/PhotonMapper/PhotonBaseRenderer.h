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

#pragma once

#include "DXSampleHelper.h"
#include "Win32Application.h"
#include "DeviceResources.h"
#include "PMScene.h"
#include "DirectXRaytracingHelper.h"

class PhotonBaseRenderer : public DX::IDeviceNotify
{
public:

    enum class RaytracingAPI
    {
        FallbackLayer,
        DirectXRaytracing,
    };

    PhotonBaseRenderer(const DXRPhotonMapper::PMScene& scene, UINT width, UINT height, std::wstring name);
    virtual ~PhotonBaseRenderer();

    virtual void OnInit() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnRender() = 0;
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized) = 0;
    virtual void OnDestroy() = 0;

    // Samples override the event handlers to handle specific messages.
    virtual void OnKeyDown(UINT8 /*key*/) {}
    virtual void OnKeyUp(UINT8 /*key*/) {}
    virtual void OnWindowMoved(int /*x*/, int /*y*/) {}
    virtual void OnMouseMove(UINT /*x*/, UINT /*y*/) {}
    virtual void OnLeftButtonDown(UINT /*x*/, UINT /*y*/) {}
    virtual void OnLeftButtonUp(UINT /*x*/, UINT /*y*/) {}
    virtual void OnDisplayChanged() {}

    // Overridable members.
    virtual void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

    // Accessors.
    UINT GetWidth() const {
        return m_width;
    }
    UINT GetHeight() const {
        return m_height;
    }
    const WCHAR* GetTitle() const {
        return m_title.c_str();
    }
    RECT GetWindowsBounds() const {
        return m_windowBounds;
    }
    virtual IDXGISwapChain* GetSwapchain() {
        return nullptr;
    }
    DX::DeviceResources* GetDeviceResources() const {
        return m_deviceResources.get();
    }

    void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void SetWindowBounds(int left, int top, int right, int bottom);
    std::wstring GetAssetFullPath(LPCWSTR assetName);

protected:

    DXRPhotonMapper::PMScene m_scene;

    void SetCustomWindowText(LPCWSTR text);

    // Viewport dimensions.
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;

    // Window bounds
    RECT m_windowBounds;

    // Override to be able to start without Dx11on12 UI for PIX. PIX doesn't support 11 on 12. 
    bool m_enableUI;

    // D3D device resources
    UINT m_adapterIDoverride;
    std::unique_ptr<DX::DeviceResources> m_deviceResources;

private:
    // Root assets path.
    std::wstring m_assetsPath;

    // Window title.
    std::wstring m_title;

protected:

    // App Resources
    RaytracingAPI m_raytracingAPI;
    bool m_isDxrSupported;

    // Raytracing Fallback Layer (FL) attributes
    ComPtr<ID3D12RaytracingFallbackDevice> m_fallbackDevice;
    ComPtr<ID3D12RaytracingFallbackCommandList> m_fallbackCommandList;

    // DirectX Raytracing (DXR) attributes
    ComPtr<ID3D12Device5> m_dxrDevice;
    ComPtr<ID3D12GraphicsCommandList5> m_dxrCommandList;

    // Descriptors
    ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
    UINT m_descriptorsAllocated;
    UINT m_descriptorSize;

    // D3D12 buffer holder
    struct D3DBuffer
    {
        ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
    };

    // Buffer for storing photons on the GPU
    struct GBuffer
    {
        ComPtr<ID3D12Resource> textureResource;
        D3D12_GPU_DESCRIPTOR_HANDLE uavGPUDescriptor;
        UINT uavDescriptorHeapIndex;
    };

    // Geometry buffer holder for building acceleration structure
    struct GeometryBuffer
    {
        UINT indexNumElements;
        UINT indexElementSize;

        UINT vertexNumElements;
        UINT vertexElementSize;

        D3DBuffer indexBuffer;
        D3DBuffer vertexBuffer;

        // 3 X 4 transform matrix
        XMMATRIX transformationMatrix;

        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs;

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelAccStructDesc;
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelAccStructPreBuildInfo;
        ComPtr<ID3D12Resource> bottomLevelScratchRes;
        ComPtr<ID3D12Resource> bottomLevelAccStructure;
    };

    struct SceneBufferDescHolder
    {
        SceneBufferDesc sceneBufferDesc;
        D3DBuffer sceneBufferDescRes;
    };
    std::vector<SceneBufferDescHolder> m_sceneBufferDescriptors;

    struct MaterialDescHolder
    {
        MaterialDesc materialDesc;
        D3DBuffer materialDescRes;
    };
    std::vector<MaterialDescHolder> m_materialDescriptors;

    struct LightDescHolder
    {
        LightDesc lightDesc;
        D3DBuffer lightDescRes;
    };
    std::vector<LightDescHolder> m_lightDescriptors;

    // Acceleration structure for the entire scene
    ComPtr<ID3D12Resource> m_topLevelAccelerationStructure;

    WRAPPED_GPU_POINTER m_fallBackPrimitiveTLAS;
    std::vector<GeometryBuffer> m_geometryBuffers;

    // Construction of Scene information
    void GetVerticesForPrimitiveType(DXRPhotonMapper::PrimitiveType type, std::vector<Vertex>& vertices);
    void GetIndicesForPrimitiveType(DXRPhotonMapper::PrimitiveType type, std::vector<Index>& indices);
    void BuildGeometryBuffers();
    void BuildGeometrySceneBufferDesc();
    void BuildMaterialBuffer();
    void BuildLightBuffer();
    void BuildGeometryAccelerationStructures();

    UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
    UINT CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize);
    WRAPPED_GPU_POINTER CreateFallbackWrappedPointer(ID3D12Resource* resource, UINT bufferNumElements);
    D3D12_RAYTRACING_GEOMETRY_DESC GetRayTracingGeometryDescriptor(const GeometryBuffer& geoBuffer);

};
