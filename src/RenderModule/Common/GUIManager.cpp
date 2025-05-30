#include <d3d12.h>
#include <d3dx12.h>

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"

#include "RendererTypedef.h"
#include "SingleDescriptorAllocator.h"

#include "GUIManager.h"
#include "OITManager.h"

static GUIManager *g_pGUIManager = nullptr;

BOOL GUIManager::Alloc(D3D12_CPU_DESCRIPTOR_HANDLE *pOutCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE *pOutGPUHandle)
{
    if (m_allocatedDescriptorCount + 1 > MAX_GUI_RESOURCE_COUNT)
    {
#ifdef _DEBUG
        __debugbreak();
#endif //  _DEBUG
        return FALSE;
    }
    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                                            m_allocatedDescriptorCount, m_srvDescriptorSize);
    CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart(),
                                            m_allocatedDescriptorCount, m_srvDescriptorSize);

    *pOutCPUHandle = cpuHandle;
    *pOutGPUHandle = gpuHandle;
    m_allocatedDescriptorCount++;
    return TRUE;
}

BOOL GUIManager::InitDescriptorHeap()
{
    BOOL result = FALSE;
    m_pD3DDevice = m_pD3DDevice;

    m_srvDescriptorSize = m_pD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // create descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC commonHeapDesc = {};
    commonHeapDesc.NumDescriptors = MAX_GUI_RESOURCE_COUNT;
    commonHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    commonHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(m_pD3DDevice->CreateDescriptorHeap(&commonHeapDesc, IID_PPV_ARGS(&m_pDescriptorHeap))))
    {
#ifdef _DEBUG
        __debugbreak();
#endif // _DEBUG
        return FALSE;
    }
    return TRUE;
}

void GUIManager::Cleanup()
{
    if (m_pDescriptorHeap)
    {
        m_pDescriptorHeap->Release();
        m_pDescriptorHeap = nullptr;
    }

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

BOOL GUIManager::Initialize(HWND hWnd, ID3D12Device5 *pD3DDevice, ID3D12CommandQueue *pCmdQueue, DXGI_FORMAT rtvFormat,
                            DXGI_FORMAT dsvFormat, UINT frameCount)
{
    g_pGUIManager = this;

    m_pD3DDevice = pD3DDevice;
    InitDescriptorHeap();

    RECT rect;
    ::GetClientRect(hWnd, &rect);
    m_width = rect.right - rect.left;
    m_height = rect.bottom - rect.top;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 키보드 지원
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // 게임패드 지원
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // 도킹 지원

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hWnd);

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = pD3DDevice;
    init_info.CommandQueue = pCmdQueue;
    init_info.NumFramesInFlight = frameCount;
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
    // Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
    // (current version of the backend will only allocate one descriptor, future versions will need to allocate more)

    init_info.SrvDescriptorHeap = m_pDescriptorHeap;
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE *out_cpu_handle,
                                        D3D12_GPU_DESCRIPTOR_HANDLE *out_gpu_handle) {
        g_pGUIManager->Alloc(out_cpu_handle, out_gpu_handle);
    };
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
                                       D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {};
    ImGui_ImplDX12_Init(&init_info);

    m_reservedDescriptorCount = m_allocatedDescriptorCount;
    m_pD3DDevice = pD3DDevice;

    return TRUE;
}

HRESULT __stdcall GUIManager::QueryInterface(REFIID riid, void **ppvObject) { return E_NOTIMPL; }

ULONG __stdcall GUIManager::AddRef(void)
{
    m_refCount++;
    return m_refCount;
}

ULONG __stdcall GUIManager::Release(void)
{
    ULONG ref_count = --m_refCount;
    if (!m_refCount)
        delete this;

    return ref_count;
}

void GUIManager::BeginRender()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    //ImGui::ShowDemoWindow();
}

void GUIManager::EndRender(ID3D12GraphicsCommandList *pCommandList)
{
    // Rendering
    ImGui::Render();

    pCommandList->SetDescriptorHeaps(1, &m_pDescriptorHeap);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);

    // Reset Descriptor Count
    m_allocatedDescriptorCount = m_reservedDescriptorCount;
}

GUIManager::~GUIManager() { Cleanup(); }

BOOL GUIManager::Begin(const char *name, bool *pOpen, GUI_WINDOW_FLAGS flags)
{
    return ImGui::Begin(name, pOpen, flags);
}

void GUIManager::End() { ImGui::End(); }

BOOL GUIManager::TreeNode(const char *name) { return ImGui::TreeNode(name); }

void GUIManager::TreePop() { ImGui::TreePop(); }

void GUIManager::SameLine() { ImGui::SameLine(); }

void GUIManager::Text(const char *fmt) { ImGui::Text(fmt); }

void GUIManager::SliderFloat(const char *label, float *v, float vMin, float vMax, const char *fmt)
{
    if (!label || !strlen(label))
    {
        ImGui::SliderFloat(EMPTY_LABEL, v, vMin, vMax, fmt);
    }
    else
    {
        ImGui::SliderFloat(label, v, vMin, vMax, fmt);
    }
}

BOOL GUIManager::DragFloat3(const char *label, Vector3 *v, float delta)
{
    if (!label || !strlen(label))
    {
        return ImGui::DragFloat3(EMPTY_LABEL, (float *)v, delta);
    }
    return ImGui::DragFloat3(label, (float *)v, delta);
}

void GUIManager::CheckBox(const char *label, bool *v)
{
    if (!label || !strlen(label))
    {
        ImGui::Checkbox(EMPTY_LABEL, v);
    }
    else
    {
        ImGui::Checkbox(label, v);
    }
}

BOOL GUIManager::Button(const char *label)
{
    if (!label || !strlen(label))
    {
        return ImGui::Button(EMPTY_LABEL);
    }
    else
    {
        return ImGui::Button(label);
    }
}

BOOL GUIManager::ColoredButton(const char *label, RGBA color)
{
    RGBA hoverColor = BrightenColor(color, 0.1f);
    RGBA activeColor = DarkenColor(color, 0.1f);

    ImVec4 idleCol(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f);
    ImVec4 hoverCol(hoverColor.r / 255.f, hoverColor.g / 255.f, hoverColor.b / 255.f, hoverColor.a / 255.f);
    ImVec4 activeCol(activeColor.r / 255.f, activeColor.g / 255.f, activeColor.b / 255.f, activeColor.a / 255.f);

    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, idleCol);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverCol);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeCol);
    bool isPressed = ImGui::Button(label);
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    return isPressed;
}

BOOL GUIManager::InvisibleButton()
{
    return ImGui::InvisibleButton("InvisibleButton", ImGui::GetContentRegionAvail(), ImGuiButtonFlags_FlattenChildren);
}

void GUIManager::Image(ITextureHandle *pTexHanlde, UINT width, UINT height)
{
    TEXTURE_HANDLE *pTex = (TEXTURE_HANDLE *)pTexHanlde;

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    Alloc(&cpuHandle, &gpuHandle);

    D3D12_RESOURCE_DESC desc = pTex->pTexture->GetDesc();

    m_pD3DDevice->CopyDescriptorsSimple(1, cpuHandle, pTex->srv.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    ImGui::Image((ImTextureID)gpuHandle.ptr, ImVec2(width, height));
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT                       GUIManager::WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

void GUIManager::OnUpdateWindowSize(UINT width, UINT height)
{
    m_width = width;
    m_height = height;
}

void GUIManager::SetNextWindowPosA(UINT posX, UINT posY, bool onlyAtFirst)
{
    ImGui::SetNextWindowPos(ImVec2(posX, posY), (onlyAtFirst ? ImGuiCond_Once : 0));
}

void GUIManager::SetNextWindowSizeA(UINT width, UINT height, bool onlyAtFirst)
{
    ImGui::SetNextWindowSize(ImVec2(width, height), (onlyAtFirst ? ImGuiCond_Once : 0));
}

void GUIManager::SetNextWindowPosR(float fPosX, float fPosY, bool onlyAtFirst)
{
    UINT posX = m_width * fPosX;
    UINT posY = m_height * fPosY;

    SetNextWindowPosA(posX, posY, onlyAtFirst);
}

void GUIManager::SetNextWindowSizeR(float fWidth, float fHeight, bool onlyAtFirst)
{
    UINT width = m_width * fWidth;
    UINT height = m_height * fHeight;

    SetNextWindowSizeA(width, height, onlyAtFirst);
}

BOOL GUIManager::BeginMenuBar() { return ImGui::BeginMenuBar(); }

void GUIManager::EndMenuBar() { ImGui::EndMenuBar(); }

BOOL GUIManager::BeginMenu(const char *label) { return ImGui::BeginMenu(label); }

void GUIManager::EndMenu() { ImGui::EndMenu(); }

BOOL GUIManager::MenuItem(const char *label, const char *shortcut) { return ImGui::MenuItem(label, shortcut); }

BOOL GUIManager::BeginChild(const char *label, float width, float height)
{
    ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();
    return ImGui::BeginChild(label, ImVec2(contentRegionAvail.x * width, contentRegionAvail.y * height),
                             ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeX);
}

void GUIManager::EndChild() { ImGui::EndChild(); }

void GUIManager::DockSpace(const char *label)
{
    /*ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGui::DockSpace(label, );
    }*/
}

void GUIManager::BeginGroup() { ImGui::BeginGroup(); }

void GUIManager::EndGroup() { ImGui::EndGroup(); }

BOOL GUIManager::BeginTabBar(const char *label) { return ImGui::BeginTabBar(label, ImGuiTabBarFlags_None); }

void GUIManager::EndTabBar() { ImGui::EndTabBar(); }

BOOL GUIManager::BeginTabItem(const char *label) { return ImGui::BeginTabItem(label); }

void GUIManager::EndTabItem() { ImGui::EndTabItem(); }

BOOL GUIManager::BeginDragDropSource() { return ImGui::BeginDragDropSource(); }

void GUIManager::EndDragDropSource() { ImGui::EndDragDropSource(); }

BOOL GUIManager::BeginDragDropTarget() { return ImGui::BeginDragDropTarget(); }

void GUIManager::EndDragDropTarget() { ImGui::EndDragDropTarget(); }

void GUIManager::SetDragDropPayload(const char *type, const void *data, size_t dataSize)
{
    ImGui::SetDragDropPayload(type, data, dataSize);
}

BOOL GUIManager::AcceptDragDropPayload(const char *type, RENDER_GUI_PAYLOAD *pOutPayload)
{
    const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(type);

    if (!payload)
        return FALSE;

    pOutPayload->data = payload->Data;
    pOutPayload->dataSize = payload->DataSize;

    return TRUE;
}
