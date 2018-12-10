#pragma once

#include "PhotonBaseRenderer.h"
#include "StepTimer.h"
#include "RaytracingHlslCompat.h"
#include "Common.h"

// The sample supports both Raytracing Fallback Layer and DirectX Raytracing APIs. 
// This is purely for demonstration purposes to show where the API differences are. 
// Real-world applications will implement only one or the other. 
// Fallback Layer uses DirectX Raytracing if a driver and OS supports it. 
// Otherwise, it falls back to compute pipeline to emulate raytracing.
// Developers aiming for a wider HW support should target Fallback Layer.
class PhotonMajorRenderer : public PhotonBaseRenderer
{
public:
    PhotonMajorRenderer(const DXRPhotonMapper::PMScene& scene, UINT width, UINT height, std::wstring name);

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    virtual void OnInit();
    virtual void OnKeyDown(UINT8 key);
    virtual void OnKeyUp();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
    virtual void OnDestroy();
    virtual IDXGISwapChain* GetSwapchain() {
        return m_deviceResources->GetSwapChain();
    }

private:
    static const UINT FrameCount = 3;
    static const UINT NumStagingBuffers = 4;
    static const UINT NumGBuffers = 4;
    static const UINT NumRenderTargets = 1;
    static const UINT NumPhotons = 100000;

    

    // We'll allocate space for several of these and they will need to be padded for alignment.
    static_assert(sizeof(SceneConstantBuffer) < D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, "Checking the size here.");
    static_assert(sizeof(SceneBufferDesc) < D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, "Checking the size here.");

    union AlignedSceneConstantBuffer
    {
        SceneConstantBuffer constants;
        uint8_t alignmentPadding[D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT];
    };
    AlignedSceneConstantBuffer*  m_mappedConstantData;
    ComPtr<ID3D12Resource>       m_perFrameConstants;

    // Fallback Pipeline State Objects for different passes
    ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackPrePassStateObject;
    ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackFirstPassStateObject;
    ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackSecondPassStateObject;
    ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackThirdPassStateObject;

    // DXR Pipeline State Objects for different passes
    ComPtr<ID3D12StateObject> m_dxrPrePassStateObject;
    ComPtr<ID3D12StateObject> m_dxrFirstPassStateObject;
    ComPtr<ID3D12StateObject> m_dxrSecondPassStateObject;
    ComPtr<ID3D12StateObject> m_dxrThirdPassStateObject;

    // Root signatures for the pre pass
    ComPtr<ID3D12RootSignature> m_prePassGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_prePassLocalRootSignature;

    // Root signatures for the first pass
    ComPtr<ID3D12RootSignature> m_firstPassGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_firstPassLocalRootSignature;

    // Root signatures for the second pass
    ComPtr<ID3D12RootSignature> m_secondPassGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_secondPassLocalRootSignature;

    // Root signatures for the second pass
    ComPtr<ID3D12RootSignature> m_thirdPassGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_thirdPassLocalRootSignature;

    // Raytracing scene
    SceneConstantBuffer m_sceneCB[FrameCount];
    CubeConstantBuffer m_cubeCB;

    UINT m_gBufferWidth;
    UINT m_gBufferHeight;
    UINT m_gBufferDepth;

    std::vector<GBuffer> m_gBuffers;
    std::vector<GBuffer> m_stagingBuffers;
    
    // Raytracing output
    ComPtr<ID3D12Resource> m_raytracingOutput;
    D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;
    UINT m_raytracingOutputResourceUAVDescriptorHeapIndex;

    GBuffer m_stagingRenderTarget;

    // Shader tables
    static const wchar_t* c_hitGroupName;
    static const wchar_t* c_raygenShaderName;
    static const wchar_t* c_closestHitShaderName;
    static const wchar_t* c_missShaderName;

    struct ShaderTableRes
    {
        ComPtr<ID3D12Resource> m_missShaderTable;
        ComPtr<ID3D12Resource> m_hitGroupShaderTable;
        ComPtr<ID3D12Resource> m_rayGenShaderTable;
    };

    ShaderTableRes m_prePassShaderTableRes;
    ShaderTableRes m_firstPassShaderTableRes;
    ShaderTableRes m_secondPassShaderTableRes;
    ShaderTableRes m_thirdPassShaderTableRes;

    // Application state
    bool m_forceComputeFallback;
    bool m_calculatePhotonMap;
    bool m_clearStagingBuffers;
    StepTimer m_timer;
    float m_curRotationAngleRad;
    XMVECTOR m_eye;
    XMVECTOR m_at;
    XMVECTOR m_up;

    void EnableDirectXRaytracing(IDXGIAdapter1* adapter);
    void ParseCommandLineArgs(WCHAR* argv[], int argc);
    void UpdateCameraMatrices();
    void InitializeScene();
    void RecreateD3D();

    void DoPrePassPhotonMapping();
    void DoFirstPassPhotonMapping();
    void DoSecondPassPhotonMapping();
    void DoThirdPassPhotonMapping();

    void CreateConstantBuffers();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();
    void ReleaseWindowSizeDependentResources();
    void CreateRaytracingInterfaces();
    void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);

    void CreatePrePassRootSignatures();
    void CreateFirstPassRootSignatures();
    void CreateSecondPassRootSignatures();
    void CreateThirdPassRootSignatures();

    void CreateLocalRootSignatureSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline, ComPtr<ID3D12RootSignature>* rootSig);

    void CreatePrePassPhotonPipelineStateObject();
    void CreateFirstPassPhotonPipelineStateObject();
    void CreateSecondPassPhotonPipelineStateObject();
    void CreateThirdPassPhotonPipelineStateObject();

    void CreateDescriptorHeap();

    // Construction of UAVs for storage of Photons
    void CreateRaytracingOutputResource();
    void CreateStagingRenderTargetResource();
    void CreateGBuffers();

    // Construction of Shader tables
    void BuildPrePassShaderTables();
    void BuildFirstPassShaderTables();
    void BuildSecondPassShaderTables();
    void BuildThirdPassShaderTables();

    void SelectRaytracingAPI(RaytracingAPI type);
    void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void CopyRaytracingOutputToBackbuffer();
    void CopyStagingBufferToBackBuffer();
    void CopyGBufferToBackBuffer(UINT gbufferIndex);
    void CalculateFrameStats();
};
