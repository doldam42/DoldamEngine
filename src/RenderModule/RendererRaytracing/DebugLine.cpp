#include "pch.h"

#include <d3dcompiler.h>
#include "GraphicsCommon.h"
#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "DescriptorPool.h"

#include "DebugLine.h"

void DebugLine::Cleanup()
{
    if (m_pPipelineState)
    {
        m_pPipelineState->Release();
        m_pPipelineState = nullptr;
    }
    if (m_pRootSignature)
    {
        m_pRootSignature->Release();
        m_pRootSignature = nullptr;
    }

    if (m_pVertexBuffer)
    {
        if (m_pLineVertexList)
        {
            m_pVertexBuffer->Unmap(0, nullptr);
            m_pLineVertexList = nullptr;
        }

        m_pVertexBuffer->Release();
        m_pVertexBuffer = nullptr;
    }
}

BOOL DebugLine::InitPiplineState() 
{
    ID3D12Device5 *pD3DDeivce = m_pRenderer->GetD3DDevice();

    ID3DBlob *pVertexShader = nullptr;
    ID3DBlob *pPixelShader = nullptr;
    
#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif
    ID3DBlob *pErrorBlob = nullptr;
    if (FAILED(D3DCompileFromFile(L"./Shaders/DrawLine.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0,
                                  &pVertexShader, &pErrorBlob)))
    {
        if (pErrorBlob != nullptr)
        {
            OutputDebugStringA((char *)pErrorBlob->GetBufferPointer());
            pErrorBlob->Release();
        }
        __debugbreak();
    }
    if (FAILED(D3DCompileFromFile(L"./Shaders/DrawLine.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0,
                                  &pPixelShader, &pErrorBlob)))
    {
        if (pErrorBlob != nullptr)
        {
            OutputDebugStringA((char *)pErrorBlob->GetBufferPointer());
            pErrorBlob->Release();
        }
        __debugbreak();
    }

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};
    psoDesc.pRootSignature = m_pRootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(pVertexShader->GetBufferPointer(), pVertexShader->GetBufferSize());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pPixelShader->GetBufferPointer(), pPixelShader->GetBufferSize());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleDesc.Count = 1;
    if (FAILED(pD3DDeivce->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState))))
    {
        __debugbreak();
    }

    if (pVertexShader)
    {
        pVertexShader->Release();
        pVertexShader = nullptr;
    }
    if (pPixelShader)
    {
        pPixelShader->Release();
        pPixelShader = nullptr;
    }
    return TRUE;
}

BOOL DebugLine::InitRootSignature()
{
    ID3D12Device5 *pD3DDeivce = m_pRenderer->GetD3DDevice();
    ID3DBlob      *pSignature = nullptr;
    ID3DBlob      *pError = nullptr;

    CD3DX12_DESCRIPTOR_RANGE rangesPerGlobal[1] = {};
    rangesPerGlobal[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0 : Global Constant Buffer View

    CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[0].InitAsDescriptorTable(_countof(rangesPerGlobal), rangesPerGlobal, D3D12_SHADER_VISIBILITY_ALL);

    D3D12_STATIC_SAMPLER_DESC sampler;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = D3D12_MAX_MAXANISOTROPY;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    sampler.MinLOD = 0;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    sampler.RegisterSpace = 0;

    // LinearWrap
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.ShaderRegister = SAMPLER_TYPE_LINEAR_WRAP;

    // Allow input layout and deny uneccessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // Create an empty root signature.
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

    if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
    {
        OutputDebugStringA((char *)pError->GetBufferPointer());
        pError->Release();
        __debugbreak();
    }

    if (FAILED(pD3DDeivce->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(),
                                               IID_PPV_ARGS(&m_pRootSignature))))
    {
        __debugbreak();
    }
    if (pSignature)
    {
        pSignature->Release();
        pSignature = nullptr;
    }
    if (pError)
    {
        pError->Release();
        pError = nullptr;
    }
    return TRUE;
}

BOOL DebugLine::Initialize(D3D12Renderer *pRenderer, UINT maxLineCount)
{
    m_pDevice = pRenderer->GetD3DDevice();
    m_pRenderer = pRenderer;

    InitRootSignature();
    InitPiplineState();

    //m_pLineVertexList = new LineVertex[maxLineCount * 2];
    m_pLineVertexList = nullptr;
    m_maxLineCount = maxLineCount;

    // Create Vertex Buffer
    {
        UINT VertexBufferSize = sizeof(LineVertex) * maxLineCount;
        if (FAILED(m_pDevice->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize), D3D12_RESOURCE_STATE_COMMON, nullptr,
                IID_PPV_ARGS(&m_pVertexBuffer))))
        {
            __debugbreak();
            return FALSE;
        }

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(LineVertex);
        m_vertexBufferView.SizeInBytes = VertexBufferSize;

        m_pVertexBuffer->SetName(L"Line Vertex Buffer");
    }

    // System Memory Mapping
    {
        CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
        if (FAILED(m_pVertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&m_pLineVertexList))))
        {
            __debugbreak();
            return FALSE;
        }
    }

    return TRUE;
}

void DebugLine::DrawLine(const Vector3 &start, const Vector3 &end, const RGBA &color)
{
    DASSERT(m_drawLineCount < m_maxLineCount);

    LineVertex *pStart = m_pLineVertexList + (2 * m_drawLineCount);
    LineVertex *pEnd = m_pLineVertexList + (2 * m_drawLineCount + 1);

    pStart->position = start;
    pStart->color = color;
    pEnd->position = end;
    pEnd->color = color;

    m_drawLineCount++;
}

void DebugLine::DrawLineAll(UINT threadIndex, ID3D12GraphicsCommandList4 *pCommandList)
{
    if (m_drawLineCount == 0)
        return;

    DescriptorPool       *pDescriptorPool = m_pRenderer->GetDescriptorPool(threadIndex);
    ID3D12DescriptorHeap *pDescriptorHeap = pDescriptorPool->GetDescriptorHeap();

    pCommandList->SetGraphicsRootSignature(m_pRootSignature);
    ID3D12DescriptorHeap *ppHeaps[] = {pDescriptorHeap};
    pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    pCommandList->SetPipelineState(m_pPipelineState);
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    pCommandList->SetGraphicsRootDescriptorTable(0, m_pRenderer->GetGlobalDescriptorHandle(threadIndex));

    pCommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    
    pCommandList->DrawInstanced(m_drawLineCount * 2, 1, 0, 0);

    m_drawLineCount = 0;
}

DebugLine::~DebugLine() { Cleanup(); }
