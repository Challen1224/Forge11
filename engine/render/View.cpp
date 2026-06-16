#include "forge11/render/View.h"
#include "forge11/platform/Win32ChildWindow.h"

#define NOMINMAX
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace forge11::render {

namespace {
constexpr uint32_t kFrameCount = 2;
}

struct View::Impl {
    platform::Win32ChildWindow window;

    ComPtr<ID3D12Device>             device;
    ComPtr<ID3D12CommandQueue>       commandQueue;
    ComPtr<IDXGISwapChain3>          swapChain;
    ComPtr<ID3D12DescriptorHeap>     rtvHeap;
    ComPtr<ID3D12Resource>           renderTargets[kFrameCount];
    ComPtr<ID3D12CommandAllocator>   commandAllocators[kFrameCount];
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ComPtr<ID3D12Fence>              fence;
    HANDLE                           fenceEvent = nullptr;
    UINT64                           fenceValues[kFrameCount] = {};
    UINT                             rtvDescriptorSize = 0;
    UINT                             frameIndex = 0;
    uint32_t                         width = 0;
    uint32_t                         height = 0;

    bool createDevice();
    bool createSwapChain();
    bool createRenderTargets();
    void waitForFrame(UINT index);
};

View::View()  : m_impl(std::make_unique<Impl>()) {}
View::~View() {
    if (m_impl->device) waitForGpu();
    if (m_impl->fenceEvent) CloseHandle(m_impl->fenceEvent);
}

bool View::Impl::createDevice() {
    UINT flags = 0;
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> dbg;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dbg)))) {
        dbg->EnableDebugLayer();
        flags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif
    ComPtr<IDXGIFactory6> factory;
    if (FAILED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory)))) return false;

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0;
         factory->EnumAdapterByGpuPreference(
             i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
             IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(),
                                         D3D_FEATURE_LEVEL_11_0,
                                         IID_PPV_ARGS(&device)))) break;
    }
    if (!device) return false;

    D3D12_COMMAND_QUEUE_DESC qd{};
    qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    return SUCCEEDED(device->CreateCommandQueue(&qd, IID_PPV_ARGS(&commandQueue)));
}

bool View::Impl::createSwapChain() {
    UINT flags = 0;
#if defined(_DEBUG)
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    ComPtr<IDXGIFactory6> factory;
    if (FAILED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory)))) return false;

    DXGI_SWAP_CHAIN_DESC1 sd{};
    sd.BufferCount  = kFrameCount;
    sd.Width        = width;
    sd.Height       = height;
    sd.Format       = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> sc1;
    HWND hwnd = static_cast<HWND>(window.nativeHandle());
    if (FAILED(factory->CreateSwapChainForHwnd(
            commandQueue.Get(), hwnd, &sd, nullptr, nullptr, &sc1))) return false;

    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    if (FAILED(sc1.As(&swapChain))) return false;

    frameIndex = swapChain->GetCurrentBackBufferIndex();
    return true;
}

bool View::Impl::createRenderTargets() {
    D3D12_DESCRIPTOR_HEAP_DESC hd{};
    hd.NumDescriptors = kFrameCount;
    hd.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    hd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(device->CreateDescriptorHeap(&hd, IID_PPV_ARGS(&rtvHeap)))) return false;

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    auto rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < kFrameCount; ++i) {
        if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])))) return false;
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
        if (FAILED(device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                IID_PPV_ARGS(&commandAllocators[i])))) return false;
    }
    return true;
}

bool View::initialize(void* parentHwnd, int width, int height) {
    m_impl->width  = static_cast<uint32_t>(width);
    m_impl->height = static_cast<uint32_t>(height);

    if (!m_impl->window.create(parentHwnd, width, height)) return false;
    if (!m_impl->createDevice())        return false;
    if (!m_impl->createSwapChain())     return false;
    if (!m_impl->createRenderTargets()) return false;

    if (FAILED(m_impl->device->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_impl->commandAllocators[m_impl->frameIndex].Get(),
            nullptr, IID_PPV_ARGS(&m_impl->commandList)))) return false;
    m_impl->commandList->Close();

    if (FAILED(m_impl->device->CreateFence(
            0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_impl->fence)))) return false;
    m_impl->fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    return true;
}

void* View::hwnd() const {
    return m_impl->window.nativeHandle();
}

void View::Impl::waitForFrame(UINT index) {
    const UINT64 target = fenceValues[index];
    if (target != 0 && fence->GetCompletedValue() < target) {
        fence->SetEventOnCompletion(target, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
}

void View::waitForGpu() {
    const UINT64 val = ++m_impl->fenceValues[m_impl->frameIndex];
    m_impl->commandQueue->Signal(m_impl->fence.Get(), val);
    m_impl->fence->SetEventOnCompletion(val, m_impl->fenceEvent);
    WaitForSingleObject(m_impl->fenceEvent, INFINITE);
}

void View::resize(int width, int height) {
    if (!m_impl->device || width <= 0 || height <= 0) return;
    waitForGpu();

    for (UINT i = 0; i < kFrameCount; ++i)
        m_impl->renderTargets[i].Reset();

    m_impl->swapChain->ResizeBuffers(
        kFrameCount, static_cast<UINT>(width), static_cast<UINT>(height),
        DXGI_FORMAT_R8G8B8A8_UNORM, 0);

    m_impl->frameIndex = m_impl->swapChain->GetCurrentBackBufferIndex();
    m_impl->width  = static_cast<uint32_t>(width);
    m_impl->height = static_cast<uint32_t>(height);

    auto rtvHandle = m_impl->rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < kFrameCount; ++i) {
        m_impl->swapChain->GetBuffer(i, IID_PPV_ARGS(&m_impl->renderTargets[i]));
        m_impl->device->CreateRenderTargetView(
            m_impl->renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_impl->rtvDescriptorSize;
    }

    // Resize the child window too.
    HWND hwnd = static_cast<HWND>(m_impl->window.nativeHandle());
    if (hwnd) SetWindowPos(hwnd, nullptr, 0, 0, width, height,
                           SWP_NOZORDER | SWP_NOACTIVATE);
}

void View::tick(float r, float g, float b) {
    auto& impl = *m_impl;
    UINT idx = impl.frameIndex;

    impl.waitForFrame(idx);
    impl.commandAllocators[idx]->Reset();
    impl.commandList->Reset(impl.commandAllocators[idx].Get(), nullptr);

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource   = impl.renderTargets[idx].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    impl.commandList->ResourceBarrier(1, &barrier);

    auto rtvHandle = impl.rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += static_cast<SIZE_T>(idx) * impl.rtvDescriptorSize;

    const float clearColor[4] = {r, g, b, 1.0f};
    impl.commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    impl.commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    impl.commandList->ResourceBarrier(1, &barrier);
    impl.commandList->Close();

    ID3D12CommandList* lists[] = {impl.commandList.Get()};
    impl.commandQueue->ExecuteCommandLists(1, lists);
    impl.swapChain->Present(0, 0);

    const UINT64 val = ++impl.fenceValues[idx];
    impl.commandQueue->Signal(impl.fence.Get(), val);
    impl.frameIndex = impl.swapChain->GetCurrentBackBufferIndex();
}

} // namespace forge11::render