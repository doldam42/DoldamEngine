#include <d3d12.h>
#include <d3dx12.h>

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

#include "GUIManager.h"

static D3D12_CPU_DESCRIPTOR_HANDLE g_cpuHandle = {};
static D3D12_GPU_DESCRIPTOR_HANDLE g_gpuHandle = {};

void GUIManager::Cleanup()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

BOOL GUIManager::Initialize(HWND hWnd, ID3D12Device *pD3DDevice, ID3D12CommandQueue *pCmdQueue,
                            ID3D12DescriptorHeap *pHeap, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
                            D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat,
                            UINT frameCount)
{
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

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

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

    g_cpuHandle = cpuHandle;
    g_gpuHandle = gpuHandle;
    init_info.SrvDescriptorHeap = pHeap;
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE *out_cpu_handle,
                                        D3D12_GPU_DESCRIPTOR_HANDLE *out_gpu_handle) {
        *out_cpu_handle = g_cpuHandle;
        *out_gpu_handle = g_gpuHandle;
    };
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo *, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle,
                                       D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {};
    ImGui_ImplDX12_Init(&init_info);

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
}

void GUIManager::EndRender(ID3D12GraphicsCommandList *pCommandList)
{
    // Rendering
    ImGui::Render();

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);
}

GUIManager::~GUIManager() { Cleanup(); }

BOOL GUIManager::Begin(const char *name, bool showAnotherWindow, GUI_WINDOW_FLAGS flags)
{
    return ImGui::Begin(name, &showAnotherWindow, flags);
}

void GUIManager::End() { ImGui::End(); }

BOOL GUIManager::TreeNode(const char *name) { return ImGui::TreeNode(name); }

void GUIManager::TreePop() { ImGui::TreePop(); }

void GUIManager::Text(const char *txt) { ImGui::Text(txt); }

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

void GUIManager::SetNextWindowPosA(UINT posX, UINT posY) { ImGui::SetNextWindowPos(ImVec2(posX, posY)); }

void GUIManager::SetNextWindowSizeA(UINT width, UINT height) { ImGui::SetNextWindowSize(ImVec2(width, height)); }

void GUIManager::SetNextWindowPosR(float fPosX, float fPosY)
{
    UINT posX = m_width * fPosX;
    UINT posY = m_height * fPosY;

    SetNextWindowPosA(posX, posY);
}

void GUIManager::SetNextWindowSizeR(float fWidth, float fHeight)
{
    UINT width = m_width * fWidth;
    UINT height = m_height * fHeight;

    SetNextWindowSizeA(width, height);
}
