#include "forge11/render/Renderer.h"

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

struct Renderer::Impl {
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<IDXGISwapChain3> swapChain;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12Resource> renderTargets[kFrameCount];
    ComPtr<ID3D12CommandAllocator> commandAllocators[kFrameCount];
    ComPtr<ID3D12GraphicsCommandList> commandList;
    ComPtr<ID3D12Fence> fence;
    HANDLE fenceEvent = nullptr;
    UINT64 fenceValues[kFrameCount] = {};
    UINT rtvDescriptorSize = 0;
    UINT frameIndex = 0;
    HWND hwnd = nullptr;

    bool createDevice();
    bool createSwapChain(uint32_t width, uint32_t height);
    bool createRenderTargets();
    void waitForFrame(UINT index);
};

Renderer::Renderer() : m_impl(std::make_unique<Impl>()) {}

Renderer::~Renderer() {
    if (m_impl->device) {
        waitForGpu();
    }
    if (m_impl->fenceEvent) {
        CloseHandle(m_impl->fenceEvent);
    }
}

bool Renderer::Impl::createDevice() {
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    ComPtr<IDXGIFactory6> factory;
    if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)))) {
        return false;
    }

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0;
         factory->EnumAdapterByGpuPreference(
             i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
         ++i) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
            break;
        }
    }

    if (!device) {
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)))) {
        return false;
    }

    return true;
}

bool Renderer::Impl::createSwapChain(uint32_t width, uint32_t height) {
    ComPtr<IDXGIFactory6> factory;
    UINT flags = 0;
#if defined(_DEBUG)
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
    if (FAILED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory)))) {
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 scDesc{};
    scDesc.BufferCount = kFrameCount;
    scDesc.Width = width;
    scDesc.Height = height;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain1;
    if (FAILED(factory->CreateSwapChainForHwnd(
            commandQueue.Get(), hwnd, &scDesc, nullptr, nullptr, &swapChain1))) {
        return false;
    }

    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    if (FAILED(swapChain1.As(&swapChain))) {
        return false;
    }

    frameIndex = swapChain->GetCurrentBackBufferIndex();
    return true;
}

bool Renderer::Impl::createRenderTargets() {
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
    heapDesc.NumDescriptors = kFrameCount;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeap)))) {
        return false;
    }

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < kFrameCount; ++i) {
        if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])))) {
            return false;
        }
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;

        if (FAILED(device->CreateCommandAllocator(
                D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i])))) {
            return false;
        }
    }

    return true;
}

bool Renderer::initialize(void* hwndPtr, uint32_t width, uint32_t height) {
    m_impl->hwnd = static_cast<HWND>(hwndPtr);

    if (!m_impl->createDevice()) return false;
    if (!m_impl->createSwapChain(width, height)) return false;
    if (!m_impl->createRenderTargets()) return false;

    if (FAILED(m_impl->device->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_impl->commandAllocators[m_impl->frameIndex].Get(), nullptr,
            IID_PPV_ARGS(&m_impl->commandList)))) {
        return false;
    }
    m_impl->commandList->Close();

    if (FAILED(m_impl->device->CreateFence(
            0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_impl->fence)))) {
        return false;
    }
    m_impl->fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    return true;
}

void Renderer::Impl::waitForFrame(UINT index) {
    const UINT64 target = fenceValues[index];
    if (target != 0 && fence->GetCompletedValue() < target) {
        fence->SetEventOnCompletion(target, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
}

void Renderer::waitForGpu() {
    const UINT64 fenceValue = ++m_impl->fenceValues[m_impl->frameIndex];
    m_impl->commandQueue->Signal(m_impl->fence.Get(), fenceValue);
    m_impl->fence->SetEventOnCompletion(fenceValue, m_impl->fenceEvent);
    WaitForSingleObject(m_impl->fenceEvent, INFINITE);
}

void Renderer::resize(uint32_t width, uint32_t height) {
    if (!m_impl->device) return;

    waitForGpu();

    for (UINT i = 0; i < kFrameCount; ++i) {
        m_impl->renderTargets[i].Reset();
    }

    m_impl->swapChain->ResizeBuffers(kFrameCount, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    m_impl->frameIndex = m_impl->swapChain->GetCurrentBackBufferIndex();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_impl->rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < kFrameCount; ++i) {
        m_impl->swapChain->GetBuffer(i, IID_PPV_ARGS(&m_impl->renderTargets[i]));
        m_impl->device->CreateRenderTargetView(m_impl->renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_impl->rtvDescriptorSize;
    }
}

void Renderer::renderFrame(float r, float g, float b, float a) {
    auto& impl = *m_impl;
    UINT idx = impl.frameIndex;

    impl.waitForFrame(idx);

    impl.commandAllocators[idx]->Reset();
    impl.commandList->Reset(impl.commandAllocators[idx].Get(), nullptr);

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = impl.renderTargets[idx].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    impl.commandList->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = impl.rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += static_cast<SIZE_T>(idx) * impl.rtvDescriptorSize;

    const float clearColor[4] = {r, g, b, a};
    impl.commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    impl.commandList->ResourceBarrier(1, &barrier);

    impl.commandList->Close();

    ID3D12CommandList* lists[] = {impl.commandList.Get()};
    impl.commandQueue->ExecuteCommandLists(1, lists);

    impl.swapChain->Present(1, 0);

    const UINT64 fenceValue = ++impl.fenceValues[idx];
    impl.commandQueue->Signal(impl.fence.Get(), fenceValue);

    impl.frameIndex = impl.swapChain->GetCurrentBackBufferIndex();
}

} // namespace forge11::render