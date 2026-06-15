#include "forge11/render/Renderer.h"

#define NOMINMAX
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

#include <vector>
#include <cstring>

using Microsoft::WRL::ComPtr;

namespace forge11::render {

namespace {
constexpr uint32_t kFrameCount = 2;
constexpr uint32_t kMaxQuads = 4096;

struct Vertex {
    float position[2]; // NDC-space x, y
    float color[4];
};

constexpr const char* kShaderSource = R"(
struct VSInput {
    float2 position : POSITION;
    float4 color    : COLOR;
};

struct PSInput {
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

PSInput VSMain(VSInput input) {
    PSInput output;
    output.position = float4(input.position, 0.0f, 1.0f);
    output.color = input.color;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET {
    return input.color;
}
)";

} // namespace

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

    // Quad pipeline
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;
    ComPtr<ID3D12Resource> vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
    uint8_t* vertexBufferMapped = nullptr;

    uint32_t viewportWidth = 0;
    uint32_t viewportHeight = 0;

    std::vector<Vertex> pendingVertices;

    bool createDevice();
    bool createSwapChain(uint32_t width, uint32_t height);
    bool createRenderTargets();
    bool createQuadPipeline();
    void waitForFrame(UINT index);
};

Renderer::Renderer() : m_impl(std::make_unique<Impl>()) {}

Renderer::~Renderer() {
    if (m_impl->device) {
        waitForGpu();
    }
    if (m_impl->vertexBuffer && m_impl->vertexBufferMapped) {
        m_impl->vertexBuffer->Unmap(0, nullptr);
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
    viewportWidth = width;
    viewportHeight = height;
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

bool Renderer::Impl::createQuadPipeline() {
    // Root signature: no resources needed, vertex color drives output.
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    if (FAILED(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                            &signature, &error))) {
        return false;
    }
    if (FAILED(device->CreateRootSignature(0, signature->GetBufferPointer(),
                                            signature->GetBufferSize(),
                                            IID_PPV_ARGS(&rootSignature)))) {
        return false;
    }

    // Compile shaders from source.
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    if (FAILED(D3DCompile(kShaderSource, strlen(kShaderSource), nullptr, nullptr, nullptr,
                           "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &error))) {
        return false;
    }
    if (FAILED(D3DCompile(kShaderSource, strlen(kShaderSource), nullptr, nullptr, nullptr,
                           "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &error))) {
        return false;
    }

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.InputLayout = {inputLayout, _countof(inputLayout)};
    psoDesc.pRootSignature = rootSignature.Get();
    psoDesc.VS = {vertexShader->GetBufferPointer(), vertexShader->GetBufferSize()};
    psoDesc.PS = {pixelShader->GetBufferPointer(), pixelShader->GetBufferSize()};

    D3D12_RASTERIZER_DESC rasterDesc{};
    rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterDesc.DepthClipEnable = TRUE;
    psoDesc.RasterizerState = rasterDesc;

    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoDesc.BlendState = blendDesc;

    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;

    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    if (FAILED(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)))) {
        return false;
    }

    // Upload-heap vertex buffer, persistently mapped, sized for kMaxQuads * 6 verts (2 triangles).
    const UINT vertexBufferSize = kMaxQuads * 6 * sizeof(Vertex);

    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufferDesc{};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = vertexBufferSize;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    if (FAILED(device->CreateCommittedResource(
            &heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer)))) {
        return false;
    }

    if (FAILED(vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertexBufferMapped)))) {
        return false;
    }

    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = vertexBufferSize;

    return true;
}

bool Renderer::initialize(void* hwndPtr, uint32_t width, uint32_t height) {
    m_impl->hwnd = static_cast<HWND>(hwndPtr);

    if (!m_impl->createDevice()) return false;
    if (!m_impl->createSwapChain(width, height)) return false;
    if (!m_impl->createRenderTargets()) return false;
    if (!m_impl->createQuadPipeline()) return false;

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
    m_impl->viewportWidth = width;
    m_impl->viewportHeight = height;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_impl->rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < kFrameCount; ++i) {
        m_impl->swapChain->GetBuffer(i, IID_PPV_ARGS(&m_impl->renderTargets[i]));
        m_impl->device->CreateRenderTargetView(m_impl->renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += m_impl->rtvDescriptorSize;
    }
}

void Renderer::drawRect(const ui::layout::LayoutRect& rect, const Color& color) {
    if (m_impl->viewportWidth == 0 || m_impl->viewportHeight == 0) {
        return;
    }

    // Convert pixel rect (origin top-left) to NDC (-1..1, origin center, Y up).
    float vw = static_cast<float>(m_impl->viewportWidth);
    float vh = static_cast<float>(m_impl->viewportHeight);

    float x0 = (static_cast<float>(rect.x) / vw) * 2.0f - 1.0f;
    float x1 = (static_cast<float>(rect.x + rect.width) / vw) * 2.0f - 1.0f;
    float y0 = 1.0f - (static_cast<float>(rect.y) / vh) * 2.0f;
    float y1 = 1.0f - (static_cast<float>(rect.y + rect.height) / vh) * 2.0f;

    Vertex topLeft{{x0, y0}, {color.r, color.g, color.b, color.a}};
    Vertex topRight{{x1, y0}, {color.r, color.g, color.b, color.a}};
    Vertex bottomLeft{{x0, y1}, {color.r, color.g, color.b, color.a}};
    Vertex bottomRight{{x1, y1}, {color.r, color.g, color.b, color.a}};

    // Two triangles forming the quad.
    m_impl->pendingVertices.push_back(topLeft);
    m_impl->pendingVertices.push_back(topRight);
    m_impl->pendingVertices.push_back(bottomLeft);

    m_impl->pendingVertices.push_back(topRight);
    m_impl->pendingVertices.push_back(bottomRight);
    m_impl->pendingVertices.push_back(bottomLeft);
}

void Renderer::renderFrame(float clearR, float clearG, float clearB, float clearA) {
    auto& impl = *m_impl;
    UINT idx = impl.frameIndex;

    impl.waitForFrame(idx);

    impl.commandAllocators[idx]->Reset();
    impl.commandList->Reset(impl.commandAllocators[idx].Get(), impl.pipelineState.Get());

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = impl.renderTargets[idx].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    impl.commandList->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = impl.rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += static_cast<SIZE_T>(idx) * impl.rtvDescriptorSize;

    const float clearColor[4] = {clearR, clearG, clearB, clearA};
    impl.commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    impl.commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Draw queued quads, if any.
    if (!impl.pendingVertices.empty()) {
        size_t vertCount = std::min(impl.pendingVertices.size(),
                                     static_cast<size_t>(kMaxQuads) * 6);
        std::memcpy(impl.vertexBufferMapped, impl.pendingVertices.data(),
                    vertCount * sizeof(Vertex));

        D3D12_VIEWPORT viewport{};
        viewport.Width = static_cast<float>(impl.viewportWidth);
        viewport.Height = static_cast<float>(impl.viewportHeight);
        viewport.MaxDepth = 1.0f;

        D3D12_RECT scissor{};
        scissor.right = static_cast<LONG>(impl.viewportWidth);
        scissor.bottom = static_cast<LONG>(impl.viewportHeight);

        impl.commandList->RSSetViewports(1, &viewport);
        impl.commandList->RSSetScissorRects(1, &scissor);
        impl.commandList->SetGraphicsRootSignature(impl.rootSignature.Get());
        impl.commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        impl.commandList->IASetVertexBuffers(0, 1, &impl.vertexBufferView);
        impl.commandList->DrawInstanced(static_cast<UINT>(vertCount), 1, 0, 0);
    }

    impl.pendingVertices.clear();

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