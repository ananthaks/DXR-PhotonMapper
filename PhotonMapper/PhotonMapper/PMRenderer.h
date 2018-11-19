#pragma once

#include "DXSampleHelper.h"
#include "Win32Application.h"

class PMRenderer
{

private:

    static const UINT FrameCount = 2;

    struct Vertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    };

    // Root assets path.
    std::wstring m_assetsPath;

    // Window title.
    std::wstring m_title;

    // Viewport dimensions.
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;

    // Adapter info.
    bool m_useWarpDevice;

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;

    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

public:

    PMRenderer(UINT width, UINT height, std::wstring name);

    ~PMRenderer();

    void OnInit();

    void OnUpdate();

    void OnRender();

    void OnDestroy();

    // Accessors.
    UINT GetWidth() const           { return m_width; }

    UINT GetHeight() const          { return m_height; }

    const WCHAR* GetTitle() const   { return m_title.c_str(); }

private:

    void LoadPipeline();

    void LoadAssets();

    void PopulateCommandList();

    void WaitForPreviousFrame();

    std::wstring GetAssetFullPath(LPCWSTR assetName);

    void GetHardwareAdapter(_In_ IDXGIFactory2* pFactory, _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter);

    void SetCustomWindowText(LPCWSTR text);
};

